#pragma once
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <thread>
#include <unordered_map>

/// @brief 温度信息
struct DevTempInfo {
  double cpu_t;
  double ddr_t;
  double gpu_t;
  double ve_t;
};

/// @brief cpu信息
struct CpuInfo {
  unsigned long user; // 用户态运行时间（不包括低优先级进程）
  unsigned long nice; // 低优先级（nice）用户态进程运行时间
  unsigned long system;     // 内核态运行时间
  unsigned long idle;       // 空闲时间（不包括 IO 等待时间）
  unsigned long iowait;     // 等待 I/O 操作完成的时间
  unsigned long irq;        // 处理硬件中断的时间
  unsigned long softirq;    // 处理软件中断的时间
  unsigned long steal;      // 虚拟环境下被 hypervisor 偷走的时间
  unsigned long guest;      // 运行普通 guest 虚拟机的时间
  unsigned long guest_nice; // 运行低优先级 guest 虚拟机的时间
};

/// @brief 磁盘
struct DiskInfo {
  std::string mount_point;   // 挂载点路径
  uint64_t block_size;       // 文件系统块大小(字节)
  uint64_t total_blocks;     // 总块数
  uint64_t free_blocks;      // 空闲块数
  uint64_t available_blocks; // 可用块数(考虑保留空间)
  uint64_t total_bytes;      // 总空间(字节)
  uint64_t free_bytes;       // 空闲空间(字节)
  uint64_t available_bytes;  // 可用空间(字节)
  double usage;              // 使用率百分比(0-100)
};

/// @brief 系统监控
class SystemMonitor {

private:
  std::ifstream file_reader_;

private:
  void ReadCpuInfo(CpuInfo &cpu_info) {
    file_reader_.open("/proc/stat");
    std::string line;
    std::getline(file_reader_, line);
    file_reader_.close();

    std::istringstream iss(line);
    std::string cpu_label;
    iss >> cpu_label >> cpu_info.user >> cpu_info.nice >> cpu_info.system >>
        cpu_info.idle >> cpu_info.iowait >> cpu_info.irq >> cpu_info.softirq >>
        cpu_info.steal >> cpu_info.guest >> cpu_info.guest_nice;
  }

public:
  double GetCpuUsage(unsigned int interval_ms = 100) {
    CpuInfo cpu_info_first, cpu_info_second;
    ReadCpuInfo(cpu_info_first);
    // 等待采样间隔
    std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    ReadCpuInfo(cpu_info_second);
    // 计算差值
    const auto calculate_diff = [](const CpuInfo &info) {
      return std::make_pair(info.idle + info.iowait, // idle
                            info.user + info.nice + info.system + info.irq +
                                info.softirq + info.steal // non-idle
      );
    };

    auto [first_idle, first_non_idle] = calculate_diff(cpu_info_first);
    auto [second_idle, second_non_idle] = calculate_diff(cpu_info_second);

    unsigned long total_diff =
        (second_idle + second_non_idle) - (first_idle + first_non_idle);
    unsigned long idle_diff = second_idle - first_idle;

    if (total_diff == 0) {
      return 0.0;
    }
    return static_cast<double>(total_diff - idle_diff) / total_diff * 100.0;
  }

  /// @brief 获取内存使用率
  /// @return
  double GetMemUsage() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
      return -1;
    }

    // 转换为double后再进行除法运算
    double total = static_cast<double>(info.totalram) * info.mem_unit;
    double free = static_cast<double>(info.freeram) * info.mem_unit;
    double used = total - free;

    if (total <= 0)
      return 0.0; // 避免除以零

    return (used / total) * 100.0;
  };

  /// @brief 获取设备相关温度信息
  /// @return
  DevTempInfo GetDevTempInfo() {
    std::string cpu_t_path = "/sys/class/thermal/thermal_zone0/temp",
                ddr_t_path = "/sys/class/thermal/thermal_zone1/temp",
                gpu_t_path = "/sys/class/thermal/thermal_zone2/temp",
                ve_t_path = "/sys/class/thermal/thermal_zone3/temp";

    DevTempInfo dev_temp_info;

    file_reader_.open(cpu_t_path);
    file_reader_ >> dev_temp_info.cpu_t;
    file_reader_.close();

    file_reader_.open(ddr_t_path);
    file_reader_ >> dev_temp_info.ddr_t;
    file_reader_.close();

    file_reader_.open(gpu_t_path);
    file_reader_ >> dev_temp_info.gpu_t;
    file_reader_.close();

    file_reader_.open(ve_t_path);
    file_reader_ >> dev_temp_info.ve_t;
    file_reader_.close();

    dev_temp_info.cpu_t /= 1e3;
    dev_temp_info.ddr_t /= 1e3;
    dev_temp_info.gpu_t /= 1e3;
    dev_temp_info.ve_t /= 1e3;

    return dev_temp_info;
  };

  /// @brief 获取磁盘信息
  /// @param mount_point
  /// @return
  DiskInfo GetDiskInfo(const std::string &mount_point) {
    struct statvfs vfs;
    if (statvfs(mount_point.c_str(), &vfs) != 0) {
      throw std::runtime_error("无法获取磁盘信息: " + mount_point);
    }

    DiskInfo info;
    info.mount_point = mount_point;
    info.block_size = vfs.f_frsize;       // 文件系统块大小
    info.total_blocks = vfs.f_blocks;     // 总块数
    info.free_blocks = vfs.f_bfree;       // 空闲块数
    info.available_blocks = vfs.f_bavail; // 可用块数

    // 计算字节大小
    info.total_bytes = info.total_blocks * info.block_size;
    info.free_bytes = info.free_blocks * info.block_size;
    info.available_bytes = info.available_blocks * info.block_size;

    // 计算使用率百分比
    if (info.total_bytes > 0) {
      info.usage = 100.0 * (1.0 - static_cast<double>(info.available_bytes) /
                                      info.total_bytes);
    } else {
      info.usage = 0.0;
    }

    return info;
  };

public:
  SystemMonitor(){};
  ~SystemMonitor(){};
};
