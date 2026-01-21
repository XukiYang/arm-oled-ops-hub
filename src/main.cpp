#include "../include/logkit/logkit.hpp"
#include "../include/ssd1315_display/ui_manager.hpp"
#include "../include/system_monitor/system_monitor.hpp"
#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>

// 配置开关结构体
struct RuntimeConfig {
  bool enable_logging = true;
  bool enable_ui = true;
  uint32_t log_interval_sec = 1;
  uint32_t ui_refresh_ms = 100;
  uint32_t ui_cycles = 30;
  uint32_t main_while_sec = 1;
};

int main(int argc, char const *argv[]) {
  // 初始化运行时配置（可从配置文件读取）
  RuntimeConfig config;
  config.enable_logging = true; // 启用日志输出
  config.enable_ui = true;      // 启用UI更新
  config.log_interval_sec = 2;  // 日志输出间隔(秒)
  config.ui_refresh_ms = 100;   // UI刷新间隔(毫秒)
  config.ui_cycles = 15;        // 每个页面的刷新次数（每个页面显示约1.5秒）
  config.main_while_sec = 0;    // 主循环刷新间隔时间(毫秒)

  LOGP_INFO("设备监控启动 (日志:%s 界面:%s)",
            config.enable_logging ? "开启" : "关闭",
            config.enable_ui ? "开启" : "关闭");

  try {
    SystemMonitor system_monitor;

    // 尝试初始化OLED显示
    std::unique_ptr<SSD1315Display> ssd1315_display;
    std::unique_ptr<UiManager> ui_manager;
    bool oled_available = false;

    if (config.enable_ui) {
      try {
        ssd1315_display = std::make_unique<SSD1315Display>("/dev/i2c-3");
        ui_manager = std::make_unique<UiManager>(*ssd1315_display);
        oled_available = true;

        // 显示启动动画
        for (int i = 0; i < 10; i++) {
          ui_manager->CreateInitUi();
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        LOGP_INFO("OLED显示初始化成功");
      } catch (const std::exception &e) {
        LOGP_WARN("OLED显示初始化失败: %s, 将仅使用日志输出", e.what());
        config.enable_ui = false;
      }
    }

    std::atomic<uint32_t> cycle_count{0};
    auto last_log_time = std::chrono::steady_clock::now();
    uint8_t current_page = 0;
    const uint8_t total_pages = 6; // 总页数

    while (true) {
      cycle_count++;
      auto current_time = std::chrono::steady_clock::now();

      // 获取系统信息
      auto dev_temp_info = system_monitor.GetDevTempInfo();
      double cpu_usage = system_monitor.GetCpuUsage();
      auto dev_mem_info = system_monitor.GetMemInfo();
      auto dev_disk_info = system_monitor.GetDiskInfo("/");
      auto net_infos = system_monitor.GetNetInfo();
      auto net_traffic = system_monitor.GetNetTraffic();
      auto sys_time = system_monitor.GetSystemTime();
      auto cpu_freq = system_monitor.GetCpuFreq();
      auto sys_load = system_monitor.GetSystemLoad();
      auto uptime = system_monitor.GetUptime();

      // 条件日志输出
      if (config.enable_logging &&
          std::chrono::duration_cast<std::chrono::seconds>(current_time -
                                                           last_log_time)
                  .count() >= config.log_interval_sec) {

        last_log_time = current_time;

        std::stringstream status_log;
        status_log << std::fixed << std::setprecision(1);

        // 温度信息
        status_log << "┌─[系统状态 #" << cycle_count << "]\n";
        status_log << "├─[温度] CPU:" << std::setw(5) << dev_temp_info.cpu_t
                   << "°C" << " DDR:" << std::setw(5) << dev_temp_info.ddr_t
                   << "°C GPU:" << std::setw(5) << dev_temp_info.gpu_t
                   << "°C\n";

        // 资源使用率
        status_log << "├─[使用率] CPU:" << std::setw(5) << cpu_usage << "%"
                   << " 内存:" << std::setw(5) << dev_mem_info.usage_percent
                   << "%" << " 磁盘:" << std::setw(5)
                   << dev_disk_info.usage_percent << "%\n";

        // CPU频率
        status_log << "├─[CPU频率] " << std::setw(5) << cpu_freq.current_mhz
                   << "MHz (" << cpu_freq.min_mhz << "-" << cpu_freq.max_mhz
                   << ")\n";

        // 系统负载
        status_log << "├─[系统负载] 1m:" << std::setw(5) << sys_load.load1
                   << " 5m:" << std::setw(5) << sys_load.load5
                   << " 15m:" << std::setw(5) << sys_load.load15 << "\n";

        // 运行时间
        status_log << "├─[运行时间] " << uptime << "\n";

        // 网络信息
        status_log << "├─[网络接口]\n";
        for (const auto &net : net_infos) {
          status_log << "│  ├─" << net.interface_name << ": " << net.ip << " ("
                     << net.family << ")\n";
        }

        // 网络流量
        if (!net_traffic.empty()) {
          status_log << "├─[网络流量] " << net_traffic[0].interface_name
                     << "\n";
          status_log << "│  ├─接收: " << std::fixed << std::setprecision(2)
                     << net_traffic[0].rx_mbps << " Mbps\n";
          status_log << "│  └─发送: " << std::fixed << std::setprecision(2)
                     << net_traffic[0].tx_mbps << " Mbps\n";
        }

        // 系统时间
        status_log << "└─[系统时间] " << std::setfill('0') << std::setw(2)
                   << sys_time.hour << ":" << std::setw(2) << sys_time.minute
                   << ":" << std::setw(2) << sys_time.second << "  "
                   << sys_time.year << "/" << std::setw(2) << sys_time.month
                   << "/" << std::setw(2) << sys_time.day << "\n";

        LOGP_INFO("%s", status_log.str().c_str());
      }

      // UI更新 - 循环显示多个页面
      if (config.enable_ui && ui_manager) {
        switch (current_page) {
        case 0:
          // 温度页面
          for (uint32_t i = 0; i < config.ui_cycles; i++) {
            ui_manager->DrawDevTempPage(dev_temp_info);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config.ui_refresh_ms));
          }
          break;
        case 1:
          // 资源使用率页面
          for (uint32_t i = 0; i < config.ui_cycles; i++) {
            ui_manager->DrawDevMemAndDiskAndCpuUsagePage(
                cpu_usage, dev_mem_info, dev_disk_info);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config.ui_refresh_ms));
          }
          break;
        case 2:
          // 网络信息页面
          for (uint32_t i = 0; i < config.ui_cycles; i++) {
            ui_manager->DrawNetInfosPage(net_infos);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config.ui_refresh_ms));
          }
          break;
        case 3:
          // 网络流量页面
          for (uint32_t i = 0; i < config.ui_cycles; i++) {
            ui_manager->DrawNetTrafficPage(net_traffic);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config.ui_refresh_ms));
          }
          break;
        case 4:
          // 系统时间页面
          for (uint32_t i = 0; i < config.ui_cycles; i++) {
            sys_time = system_monitor.GetSystemTime();
            ui_manager->DrawSystemTimePage(sys_time);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config.ui_refresh_ms));
          }
          break;
        case 5:
          // 系统信息页面
          for (uint32_t i = 0; i < config.ui_cycles; i++) {
            ui_manager->DrawSystemInfoPage(cpu_freq, sys_load, uptime);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config.ui_refresh_ms));
          }
          break;
        }

        // 切换到下一个页面
        current_page = (current_page + 1) % total_pages;
      }

      // 控制主循环速度
      std::this_thread::sleep_for(
          std::chrono::milliseconds(config.main_while_sec));
    }
  } catch (const std::exception &e) {
    LOGP_ERROR("系统错误: %s", e.what());
    return 1;
  }
  return 0;
}