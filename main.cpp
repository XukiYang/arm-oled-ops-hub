#include "include/logkit/logkit.hpp"
#include "include/system_monitor/system_monitor.hpp"
#include <iostream>

int main(int argc, char const *argv[]) {
  SystemMonitor system_monitor;
  double cpu_usage = system_monitor.GetCpuUsage();
  double mem_usage = system_monitor.GetMemUsage();
  auto dev_temp_info = system_monitor.GetDevTempInfo();
  auto dev_disk_info = system_monitor.GetDiskInfo("/");

  LOGP_MSG("cpu_usage:%f%,mem_usage:%f%", cpu_usage, mem_usage);
  LOGP_MSG("cpu_t:%f%,ddr_t:%f%,gpu_t:%f%,ve_t:%f%", dev_temp_info.cpu_t,
           dev_temp_info.ddr_t, dev_temp_info.gpu_t, dev_temp_info.ve_t);
  LOGP_MSG("disk_usage:%f%", dev_disk_info.usage);
  return 0;
}
