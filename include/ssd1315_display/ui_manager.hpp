#pragma once
#include "../system_monitor/system_monitor.hpp"
#include "ssd1315_display.hpp"
#include <iomanip>

class UiManager {
private:
  SSD1315Display ssd1315_display_;
  uint8_t animation_frame_ = 0;

  // 温度格式化 (保留1位小数 + 摄氏度符号)
  std::string FormatTemperatureC(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << value << "C";
    std::string result = oss.str();
    if (result.length() > 6) result = result.substr(0, 6);
    return result;
  }

  // 百分比格式化 (保留1位小数 + 百分号)
  std::string FormatPercentage(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << value << "%";
    std::string result = oss.str();
    if (result.length() > 6) result = result.substr(0, 6);
    return result;
  }

  // 存储容量格式化 (整数 + GB单位)
  std::string FormatStorageGB(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << value << "GB";
    std::string result = oss.str();
    if (result.length() > 5) result = result.substr(0, 5);
    return result;
  }

  // 速率格式化 (保留2位小数 + 单位)
  std::string FormatMbps(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value << "Mbps";
    std::string result = oss.str();
    if (result.length() > 10) result = result.substr(0, 10);
    return result;
  }

  // 时间格式化
  std::string FormatTime(int hour, int minute, int second) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hour << ":"
        << std::setw(2) << minute << ":" << std::setw(2) << second;
    return oss.str();
  }

  // 日期格式化
  std::string FormatDate(int year, int month, int day) {
    std::ostringstream oss;
    oss << year << "/" << std::setfill('0') << std::setw(2) << month << "/"
        << std::setw(2) << day;
    return oss.str();
  }

  // 绘制小图标 - CPU
  void DrawCpuIcon(int x, int y, uint8_t color = 1) {
    // 简化的CPU图标
    ssd1315_display_.FillRect(x, y, 8, 8, color);
    ssd1315_display_.FillRect(x + 1, y + 1, 6, 6, 0);
    ssd1315_display_.DrawLine(x + 2, y + 3, x + 6, y + 3, color);
    ssd1315_display_.DrawLine(x + 2, y + 5, x + 6, y + 5, color);
  }

  // 绘制小图标 - 内存
  void DrawMemIcon(int x, int y, uint8_t color = 1) {
    // 简化的内存芯片图标
    ssd1315_display_.FillRect(x, y, 8, 8, color);
    ssd1315_display_.FillRect(x + 1, y + 1, 6, 6, 0);
    ssd1315_display_.FillRect(x + 2, y + 2, 2, 4, color);
    ssd1315_display_.FillRect(x + 4, y + 2, 2, 4, color);
  }

  // 绘制小图标 - 磁盘
  void DrawDiskIcon(int x, int y, uint8_t color = 1) {
    // 简化的磁盘图标
    ssd1315_display_.DrawRoundRect(x, y, 8, 8, 1, color);
    ssd1315_display_.FillCircle(x + 4, y + 4, 1, color);
    ssd1315_display_.DrawLine(x + 2, y + 4, x + 6, y + 4, color);
  }

  // 绘制小图标 - 网络
  void DrawNetIcon(int x, int y, uint8_t color = 1) {
    // 简化的网络图标
    ssd1315_display_.DrawCircle(x + 4, y + 4, 3, color);
    ssd1315_display_.DrawPixel(x + 4, y, color);
    ssd1315_display_.DrawPixel(x, y + 4, color);
    ssd1315_display_.DrawPixel(x + 8, y + 4, color);
    ssd1315_display_.DrawPixel(x + 4, y + 8, color);
  }

  // 绘制小图标 - 温度
  void DrawTempIcon(int x, int y, uint8_t color = 1) {
    // 简化的温度计图标
    ssd1315_display_.FillRect(x + 2, y + 2, 4, 5, color);
    ssd1315_display_.FillRect(x + 1, y + 7, 6, 1, color);
  }

public:
  /// @brief 依赖构造
  /// @param ssd1315_display
  UiManager(SSD1315Display &ssd1315_display)
      : ssd1315_display_(ssd1315_display){

        };

  /// @brief 绘制初始UI
  void CreateInitUi() {
    // 清空显示缓冲区
    ssd1315_display_.ClearDisplay();
    // 绘制边框
    ssd1315_display_.DrawRoundRect(0, 0, ssd1315_display_.Width(),
                                   ssd1315_display_.Height(), 3, 1);
    // 绘制欢迎标题
    ssd1315_display_.DrawString(28, 10, "WELCOME", 1, 2);

    // 绘制动画圆点
    uint8_t offset = (animation_frame_ % 4) * 2;
    for (int i = 0; i < 3; i++) {
      uint8_t size = (i == offset / 2) ? 2 : 1;
      ssd1315_display_.FillCircle(45 + i * 10 + offset, 55, size, 1);
    }
    animation_frame_++;

    // 绘制欢迎信息
    ssd1315_display_.DrawString(15, 35, "ORANGE PI", 1, 1);

    // 刷新显示
    ssd1315_display_.RefreshDisplay();
  };

  /// @brief 绘制设备温度UI
  /// @param dev_temp
  void DrawDevTempPage(DevTempInfo &dev_temp) {
    // 清空显示缓冲区
    ssd1315_display_.ClearDisplay();

    // 绘制圆角边框
    ssd1315_display_.DrawRoundRect(0, 0, ssd1315_display_.Width(),
                                   ssd1315_display_.Height(), 3, 1);
    
    // 居中显示标题
    ssd1315_display_.DrawString((128 / 2) - (5 * 4 / 2), 5, "TEMP", 1, 1);
    
    // 绘制分隔线
    ssd1315_display_.DrawLine(0, 15, 128, 15, 1);

    // 绘制温度图标和值 - 左上区域
    DrawTempIcon(8, 22, 1);
    ssd1315_display_.DrawString(20, 22, "CPU:", 1, 1);
    ssd1315_display_.DrawString(45, 22, FormatTemperatureC(dev_temp.cpu_t), 1, 1);

    // 右上区域
    DrawTempIcon(70, 22, 1);
    ssd1315_display_.DrawString(82, 22, "GPU:", 1, 1);
    ssd1315_display_.DrawString(105, 22, FormatTemperatureC(dev_temp.gpu_t), 1, 1);

    // 左下区域
    DrawTempIcon(8, 42, 1);
    ssd1315_display_.DrawString(20, 42, "DDR:", 1, 1);
    ssd1315_display_.DrawString(45, 42, FormatTemperatureC(dev_temp.ddr_t), 1, 1);

    // 右下区域
    DrawTempIcon(70, 42, 1);
    ssd1315_display_.DrawString(82, 42, "VE:", 1, 1);
    ssd1315_display_.DrawString(105, 42, FormatTemperatureC(dev_temp.ve_t), 1, 1);

    // 绘制进度条风格的温度指示
    int cpu_temp_bar = static_cast<int>(dev_temp.cpu_t / 100.0 * 120);
    int gpu_temp_bar = static_cast<int>(dev_temp.gpu_t / 100.0 * 120);
    
    ssd1315_display_.DrawRect(4, 56, 58, 4, 1);
    if (cpu_temp_bar > 0) ssd1315_display_.FillRect(5, 57, cpu_temp_bar/2, 2, 1);
    
    ssd1315_display_.DrawRect(66, 56, 58, 4, 1);
    if (gpu_temp_bar > 0) ssd1315_display_.FillRect(67, 57, gpu_temp_bar/2, 2, 1);

    ssd1315_display_.RefreshDisplay();
  };

  /// @brief 绘制CPU使用率&内存&磁盘页面
  void DrawDevMemAndDiskAndCpuUsagePage(double cpu_usage, MemInfo mem_info,
                                        DiskInfo &disk_info) {
    // 清空显示缓冲区
    ssd1315_display_.ClearDisplay();
    // 绘制圆角边框
    ssd1315_display_.DrawRoundRect(0, 0, ssd1315_display_.Width(),
                                   ssd1315_display_.Height(), 3, 1);
    // 居中显示标题
    ssd1315_display_.DrawString((128 / 2) - (5 * 5 / 2), 5, "USAGE", 1, 1);
    
    // 绘制分隔线
    ssd1315_display_.DrawLine(0, 15, 128, 15, 1);

    // CPU使用率 - 带图标和进度条
    DrawCpuIcon(8, 20, 1);
    ssd1315_display_.DrawString(20, 20, "CPU:", 1, 1);
    ssd1315_display_.DrawString(45, 20, FormatPercentage(cpu_usage), 1, 1);
    ssd1315_display_.DrawProgressBar(8, 30, 112, 6, static_cast<uint8_t>(cpu_usage), 1);

    // 内存使用率 - 带图标和进度条
    DrawMemIcon(8, 40, 1);
    ssd1315_display_.DrawString(20, 40, "MEM:", 1, 1);
    std::string mem_str = FormatPercentage(mem_info.usage_percent) + " (" + 
                          FormatStorageGB(mem_info.used_mb / 1024) + "/" +
                          FormatStorageGB(mem_info.total_mb / 1024) + ")";
    ssd1315_display_.DrawString(45, 40, mem_str.substr(0, 15), 1, 1);
    ssd1315_display_.DrawProgressBar(8, 50, 112, 6, 
                                      static_cast<uint8_t>(mem_info.usage_percent), 1);

    ssd1315_display_.RefreshDisplay();
  };

  /// @brief 绘制网络信息页面
  /// @param net_infos 
  void DrawNetInfosPage(std::vector<NetInfo> &net_infos) {
    // 清空显示缓冲区
    ssd1315_display_.ClearDisplay();
    // 绘制圆角边框
    ssd1315_display_.DrawRoundRect(0, 0, ssd1315_display_.Width(),
                                   ssd1315_display_.Height(), 3, 1);
    // 居中显示标题
    ssd1315_display_.DrawString((128 / 2) - (5 * 3 / 2), 5, "NET", 1, 1);
    
    // 绘制分隔线
    ssd1315_display_.DrawLine(0, 15, 128, 15, 1);
    
    // 绘制网络图标动画效果
    uint8_t icon_y = (animation_frame_ % 2) == 0 ? 18 : 20;
    DrawNetIcon(5, icon_y, 1);
    animation_frame_++;

    // 过滤掉回环接口
    std::vector<NetInfo> filtered_infos;
    for (auto &info : net_infos) {
      if (info.interface_name != "lo") {
        filtered_infos.push_back(info);
      }
    }

    // 显示第一个网络接口的信息
    if (!filtered_infos.empty()) {
      std::string iface_name = filtered_infos[0].interface_name;
      if (iface_name.length() > 8) iface_name = iface_name.substr(0, 8);
      
      std::string ip = filtered_infos[0].ip;
      if (ip.length() > 15) ip = ip.substr(0, 15);
      
      ssd1315_display_.DrawString(20, 20, iface_name, 1, 1);
      ssd1315_display_.DrawString(20, 32, ip, 1, 1);
      ssd1315_display_.DrawString(100, 32, filtered_infos[0].family, 1, 1);
    }

    // 显示第二个网络接口的信息（如果有）
    if (filtered_infos.size() > 1) {
      std::string iface_name2 = filtered_infos[1].interface_name;
      if (iface_name2.length() > 8) iface_name2 = iface_name2.substr(0, 8);
      
      std::string ip2 = filtered_infos[1].ip;
      if (ip2.length() > 15) ip2 = ip2.substr(0, 15);
      
      ssd1315_display_.DrawString(20, 44, iface_name2, 1, 1);
      ssd1315_display_.DrawString(20, 56, ip2, 1, 1);
    }

    ssd1315_display_.RefreshDisplay();
  };

  /// @brief 绘制系统时间页面
  void DrawSystemTimePage(SystemTime &sys_time) {
    ssd1315_display_.ClearDisplay();
    ssd1315_display_.DrawRoundRect(0, 0, ssd1315_display_.Width(),
                                   ssd1315_display_.Height(), 3, 1);
    
    // 显示时间（使用大小1的字体，避免超出屏幕）
    std::string time_str = FormatTime(sys_time.hour, sys_time.minute, sys_time.second);
    ssd1315_display_.DrawString(15, 15, time_str, 1, 1);
    
    // 显示日期
    std::string date_str = FormatDate(sys_time.year, sys_time.month, sys_time.day);
    ssd1315_display_.DrawString(15, 30, date_str, 1, 1);
    
    // 绘制动画时钟指针
    uint8_t sec_hand = (sys_time.second * 60) / 60; // 限制指针长度为60像素
    ssd1315_display_.DrawLine(64, 54, 64 + sec_hand/2, 54, 1);
    
    ssd1315_display_.RefreshDisplay();
  }

  /// @brief 绘制网络流量页面
  void DrawNetTrafficPage(std::vector<NetTraffic> &traffic) {
    ssd1315_display_.ClearDisplay();
    ssd1315_display_.DrawRoundRect(0, 0, ssd1315_display_.Width(),
                                   ssd1315_display_.Height(), 3, 1);
    
    ssd1315_display_.DrawString((128 / 2) - (5 * 4 / 2), 5, "TRAFFIC", 1, 1);
    ssd1315_display_.DrawLine(0, 15, 128, 15, 1);
    
    // 过滤掉回环接口
    NetTraffic *selected_traffic = nullptr;
    for (auto &t : traffic) {
      if (t.interface_name != "lo") {
        selected_traffic = &t;
        break;
      }
    }
    
    // 显示第一个网络接口的流量
    if (selected_traffic) {
      DrawNetIcon(5, 20, 1);
      
      std::string iface_name = selected_traffic->interface_name;
      if (iface_name.length() > 8) iface_name = iface_name.substr(0, 8);
      ssd1315_display_.DrawString(20, 20, iface_name, 1, 1);
      
      std::string rx_str = "RX: " + FormatMbps(selected_traffic->rx_mbps);
      std::string tx_str = "TX: " + FormatMbps(selected_traffic->tx_mbps);
      
      ssd1315_display_.DrawString(8, 35, rx_str, 1, 1);
      ssd1315_display_.DrawString(8, 48, tx_str, 1, 1);
      
      // 绘制流量指示条
      uint8_t rx_bar = static_cast<uint8_t>(selected_traffic->rx_mbps * 10);
      uint8_t tx_bar = static_cast<uint8_t>(selected_traffic->tx_mbps * 10);
      if (rx_bar > 50) rx_bar = 50;
      if (tx_bar > 50) tx_bar = 50;
      
      ssd1315_display_.DrawRect(75, 36, 50, 5, 1);
      if (rx_bar > 0) ssd1315_display_.FillRect(76, 37, rx_bar, 3, 1);
      
      ssd1315_display_.DrawRect(75, 49, 50, 5, 1);
      if (tx_bar > 0) ssd1315_display_.FillRect(76, 50, tx_bar, 3, 1);
    }
    
    ssd1315_display_.RefreshDisplay();
  }

  /// @brief 绘制系统信息页面
  void DrawSystemInfoPage(CpuFreqInfo &cpu_freq, SystemLoad &sys_load, 
                          const std::string &uptime) {
    ssd1315_display_.ClearDisplay();
    ssd1315_display_.DrawRoundRect(0, 0, ssd1315_display_.Width(),
                                   ssd1315_display_.Height(), 3, 1);
    
    ssd1315_display_.DrawString((128 / 2) - (5 * 4 / 2), 5, "SYSTEM", 1, 1);
    ssd1315_display_.DrawLine(0, 15, 128, 15, 1);
    
    // CPU频率
    DrawCpuIcon(5, 20, 1);
    std::string freq_str = std::to_string(static_cast<int>(cpu_freq.current_mhz)) + "MHz";
    if (freq_str.length() > 12) freq_str = freq_str.substr(0, 12);
    ssd1315_display_.DrawString(20, 20, freq_str, 1, 1);
    
    // 系统负载
    std::string load_str = "L: " + std::to_string(sys_load.load1).substr(0, 4);
    ssd1315_display_.DrawString(8, 35, load_str, 1, 1);
    
    // 运行时间
    std::string up_str = "UP: " + uptime;
    if (up_str.length() > 20) up_str = up_str.substr(0, 20);
    ssd1315_display_.DrawString(8, 48, up_str, 1, 1);
    
    ssd1315_display_.RefreshDisplay();
  }

  ~UiManager(){

  };
};
