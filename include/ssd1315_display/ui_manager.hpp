#pragma once
#include "../system_monitor/system_monitor.hpp"
#include "ssd1315_display.hpp"

class UiManager {
private:
  SSD1315Display ssd1315_display_;

  std::string DoubleToTwoDecimals(double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value << "%";
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

  /// @brief 创建系统信息UI
  void CreateSystemInfoUi(CpuInfo &cpu_info, DiskInfo &disk_info) {
    // 清空显示缓冲区
    ssd1315_display_.ClearDisplay();
    // 绘制边框
    ssd1315_display_.DrawRect(0, 0, ssd1315_display_.Width(),
                              ssd1315_display_.Height(), 1);

    // 绘制温度页
    // display.DrawString(40, 5, "", 1, 1);
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
    ssd1315_display_.DrawString(5 + 20, 25, DoubleToTwoDecimals(dev_temp.cpu_t),
                                1, 1);
    ssd1315_display_.DrawString(5 + 20, 45, DoubleToTwoDecimals(dev_temp.ddr_t),
                                1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5 + 20, 25,
                                DoubleToTwoDecimals(dev_temp.gpu_t), 1, 1);
    ssd1315_display_.DrawString((128 / 2) + 5 + 20, 45,
                                DoubleToTwoDecimals(dev_temp.ve_t), 1, 1);

    ssd1315_display_.RefreshDisplay();
  };

  ~UiManager(){

  };
};
