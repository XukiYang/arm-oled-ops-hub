#include "../include/logkit/logkit.hpp"
#include "../include/ssd1315_display/ui_manager.hpp"
#include "../include/system_monitor/system_monitor.hpp"

#include <iostream>
#include <unistd.h>

int main(int argc, char const *argv[]) {
  LOG_MSG("device monitor runing!");
  SystemMonitor system_monitor;
  SSD1315Display ssd1315_display("/dev/i2c-3");
  UiManager ui_manager(ssd1315_display);
  ui_manager.CreateInitUi();
  sleep(1);

  while (true) {
    for (uint32_t i = 0; i < 30; i++) {
      auto dev_temp_info = system_monitor.GetDevTempInfo();
      ui_manager.DrawDevTempPage(dev_temp_info);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (uint32_t i = 0; i < 30; i++) {
      double cpu_usage = system_monitor.GetCpuUsage();
      auto dev_mem_info = system_monitor.GetMemInfo();
      auto dev_disk_info = system_monitor.GetDiskInfo("/");
      ui_manager.DrawDevMemAndDiskAndCpuUsagePage(cpu_usage, dev_mem_info,
                                                  dev_disk_info);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    auto net_infos = system_monitor.GetNetInfo();
    ui_manager.DrawNetInfosPage(net_infos);
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  return 0;
}
