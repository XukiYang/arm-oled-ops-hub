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
  double cpu_t{0}; // CPU温度(摄氏度)
  double ddr_t{0}; // DDR温度(摄氏度)
  double gpu_t{0}; // GPU温度(摄氏度)
  double ve_t{0};  // VE温度(摄氏度)
};

/// @brief cpu信息
struct CpuTimeStamp {
  uint64_t idle{0};  // idle + iowait
  uint64_t total{0}; // 所有状态的总和
  std::chrono::steady_clock::time_point time;
};

/// @brief 磁盘信息
struct DiskInfo {
  std::string mount_point;   // 挂载点路径
  uint64_t block_size;       // 文件系统块大小(字节)
  uint64_t total_blocks;     // 总块数
  uint64_t free_blocks;      // 空闲块数
  uint64_t available_blocks; // 可用块数(考虑保留空间)
  uint64_t total_bytes;      // 总空间(字节)
  uint64_t free_bytes;       // 空闲空间(字节)
  uint64_t available_bytes;  // 可用空间(字节)
  double usage_percent;      // 使用率百分比(0-100)
};

/// @brief CPU 核心统计信息
struct CpuCoreStats {
  unsigned long idle{0};  // 空闲时间（包括 IO 等待）
  unsigned long total{0}; // 总时间
};

/// @brief 内存使用信息
struct MemInfo {
  double total_mb{0};      // 总内存 (MB)
  double used_mb{0};       // 已用内存 (MB)
  double usage_percent{0}; // 使用率百分比(0-100)
};

/// @brief 系统监控
class SystemMonitor {

private:
  std::ifstream file_reader_;

private:
  std::vector<CpuTimeStamp> ReadCpuStats() {
    std::ifstream file("/proc/stat");
    std::vector<CpuTimeStamp> stamps;
    std::string line;

    while (std::getline(file, line) && line.compare(0, 3, "cpu") == 0) {
      std::istringstream iss(line);
      std::string label;
      unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
      iss >> label >> user >> nice >> system >> idle >> iowait >> irq >>
          softirq >> steal;

      stamps.push_back({.idle = idle + iowait,
                        .total = user + nice + system + idle + iowait + irq +
                                 softirq + steal,
                        .time = std::chrono::steady_clock::now()});
    }
    return stamps;
  }

public:
  double GetCpuUsage() {
    static std::vector<CpuTimeStamp> prev_stamps = ReadCpuStats();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 采样间隔
    auto curr_stamps = ReadCpuStats();

    if (prev_stamps.empty() || curr_stamps.size() != prev_stamps.size()) {
      return 0.0;
    }

    double total_usage = 0.0;
    for (size_t i = 0; i < curr_stamps.size(); ++i) {
      unsigned long total_diff = curr_stamps[i].total - prev_stamps[i].total;
      unsigned long idle_diff = curr_stamps[i].idle - prev_stamps[i].idle;

      if (total_diff > 0) {
        total_usage +=
            (1.0 - static_cast<double>(idle_diff) / total_diff) * 100.0;
      }
    }
    prev_stamps = curr_stamps;               // 更新状态
    return total_usage / curr_stamps.size(); // 返回多核平均值
  }

  /// @brief 获取内存使用率
  /// @return
  /// @return
  MemInfo GetMemInfo() {
    std::ifstream file("/proc/meminfo");
    std::string line;
    unsigned long total_kb = 0, free_kb = 0, buffers_kb = 0, cached_kb = 0;

    while (file >> line) {
      if (line == "MemTotal:")
        file >> total_kb;
      else if (line == "MemFree:")
        file >> free_kb;
      else if (line == "Buffers:")
        file >> buffers_kb;
      else if (line == "Cached:")
        file >> cached_kb;
    }

    double total_mb = total_kb / 1024.0;
    double used_mb = (total_kb - free_kb - buffers_kb - cached_kb) / 1024.0;
    double usage_percent =
        (total_kb > 0) ? (used_mb / (total_mb + 1e-9)) * 100.0 : 0.0;

    return {total_mb, used_mb, usage_percent};
  }
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
      info.usage_percent =
          100.0 *
          (1.0 - static_cast<double>(info.available_bytes) / info.total_bytes);
    } else {
      info.usage_percent = 0.0;
    }

    return info;
  };

public:
  SystemMonitor(){};
  ~SystemMonitor(){};
};
