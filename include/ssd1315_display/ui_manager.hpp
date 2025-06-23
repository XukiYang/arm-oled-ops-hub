#pragma once
#include "../system_monitor/system_monitor.hpp"
#include "ssd1315_display.hpp"

class UiManager {
private:
  SSD1315Display ssd1315_display_;

  // 温度格式化 (保留2位小数 + 摄氏度符号)
  std::string FormatTemperatureC(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value << "C";
    return oss.str();
  }

  // 百分比格式化 (保留2位小数 + 百分号)
  std::string FormatPercentage(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value << "%";
    return oss.str();
  }

  // 存储容量格式化 (整数 + GB单位)
  std::string FormatStorageGB(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << value << "GB";
    return oss.str();
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
    ssd1315_display_.DrawRect(0, 0, ssd1315_display_.Width(),
                              ssd1315_display_.Height(), 1);
    // 绘制欢迎标题
    ssd1315_display_.DrawString(30, 10, "WELCOME", 1, 2);

    // 绘制欢迎信息
    ssd1315_display_.DrawString(15, 35, "CREATED BY XUKI", 1, 1);

    // 刷新显示
    ssd1315_display_.RefreshDisplay();
  };

  /// @brief 绘制设备温度UI
  /// @param dev_temp
  void DrawDevTempPage(DevTempInfo &dev_temp) {
    // 清空显示缓冲区
    ssd1315_display_.ClearDisplay();

    // 绘制边框
    ssd1315_display_.DrawRect(0, 0, ssd1315_display_.Width(),
                              ssd1315_display_.Height(), 1);
    // 居中显示标题
    ssd1315_display_.DrawString((128 / 2) - (5 * 4 / 2), 5, "TEMP", 1, 1);

    // 绘制温度标题
    ssd1315_display_.DrawString(5, 25, "CPU", 1, 1);
    ssd1315_display_.DrawString(5, 45, "DDR", 1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5, 25, "GPU", 1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5, 45, "VE", 1, 1);

    // 绘制温度值
    ssd1315_display_.DrawString(5 + 20, 25, FormatTemperatureC(dev_temp.cpu_t),
                                1, 1);
    ssd1315_display_.DrawString(5 + 20, 45, FormatTemperatureC(dev_temp.ddr_t),
                                1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5 + 20, 25,
                                FormatTemperatureC(dev_temp.gpu_t), 1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5 + 20, 45,
                                FormatTemperatureC(dev_temp.ve_t), 1, 1);

    ssd1315_display_.RefreshDisplay();
  };

  /// @brief 绘制CPU使用率&内存&磁盘页面
  void DrawDevMemAndDiskAndCpuUsagePage(double cpu_usage, MemInfo mem_info,
                                        DiskInfo &disk_info) {
    // 清空显示缓冲区
    ssd1315_display_.ClearDisplay();
    // 绘制边框
    ssd1315_display_.DrawRect(0, 0, ssd1315_display_.Width(),
                              ssd1315_display_.Height(), 1);
    // 居中显示标题
    ssd1315_display_.DrawString((128 / 2) - (5 * 5 / 2), 5, "USAGE", 1, 1);

    ssd1315_display_.DrawString(5, 25, "CPU", 1, 1);
    ssd1315_display_.DrawString(5, 45, "MEM", 1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5, 25, "DS", 1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5, 45, "ALL", 1, 1);

    ssd1315_display_.DrawString(5 + 20, 25, FormatPercentage(cpu_usage), 1, 1);
    ssd1315_display_.DrawString(5 + 20, 45,
                                FormatPercentage(mem_info.usage_percent), 1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5 + 20, 25,
                                FormatPercentage(disk_info.usage_percent), 1,
                                1);
    ssd1315_display_.DrawString(
        (128 / 2) + 5 + 20, 45,
        FormatStorageGB(disk_info.total_bytes / 1024 / 1024 / 1024), 1, 1);
    ssd1315_display_.RefreshDisplay();
  };

  void DrawNetInfosPage(std::vector<NetInfo> &net_infos) {
    // 清空显示缓冲区
    ssd1315_display_.ClearDisplay();
    // 绘制边框
    ssd1315_display_.DrawRect(0, 0, ssd1315_display_.Width(),
                              ssd1315_display_.Height(), 1);
    // 居中显示标题
    ssd1315_display_.DrawString((128 / 2) - (5 * 3 / 2), 5, "NET", 1, 1);
    for (uint8_t i = 0; i < net_infos.size(); i++) {
      ssd1315_display_.DrawString(5, 25 + i * 15, net_infos[i].interface_name,
                                  1, 1);
      ssd1315_display_.DrawString(20, 25 + i * 15, net_infos[i].ip, 1, 1);
      ssd1315_display_.DrawString(60, 25 + i * 15, net_infos[i].family, 1, 1);
    }
    ssd1315_display_.RefreshDisplay();
  };

  ~UiManager(){

  };
};
