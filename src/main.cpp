#include "../include/logkit/logkit.hpp"
#include "../include/ssd1315_display/ui_manager.hpp"
#include "../include/system_monitor/system_monitor.hpp"

#include <iostream>
#include <unistd.h>

int main(int argc, char const *argv[]) {
  SystemMonitor system_monitor;
  SSD1315Display ssd1315_display("/dev/i2c-3");
  UiManager ui_manager(ssd1315_display);
  ui_manager.CreateInitUi();
  sleep(1);

  while (true) {

    for (uint32_t i = 0; i < 3; i++) {
      auto dev_temp_info = system_monitor.GetDevTempInfo();
      ui_manager.DrawDevTempPage(dev_temp_info);
      sleep(1);
    }

    for (uint32_t i = 0; i < 3; i++) {
      double cpu_usage = system_monitor.GetCpuUsage();
      double mem_usage = system_monitor.GetMemUsage();
      auto dev_disk_info = system_monitor.GetDiskInfo("/");
      ui_manager.DrawDevMemAndDiskAndCpuUsagePage(cpu_usage, mem_usage,
                                                  dev_disk_info);
      sleep(1);
    }
  }

  return 0;
}
