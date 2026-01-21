#pragma once
#include <chrono>
#include <cstdio>
#include <fstream>
#include <ifaddrs.h>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <string>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <thread>
#include <unordered_map>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

/// @brief 网络信息
struct NetInfo {
  std::string interface_name;
  std::string ip;
  std::string family;
};

/// @brief 网络流量信息
struct NetTraffic {
  std::string interface_name;
  uint64_t rx_bytes{0};  // 接收字节数
  uint64_t tx_bytes{0};  // 发送字节数
  double rx_mbps{0};     // 接收速率(Mbps)
  double tx_mbps{0};     // 发送速率(Mbps)
};

/// @brief 系统时间信息
struct SystemTime {
  int hour;
  int minute;
  int second;
  int day;
  int month;
  int year;
};

/// @brief CPU频率信息
struct CpuFreqInfo {
  double current_mhz{0};
  double min_mhz{0};
  double max_mhz{0};
};

/// @brief 系统负载信息
struct SystemLoad {
  double load1{0};   // 1分钟负载
  double load5{0};   // 5分钟负载
  double load15{0};  // 15分钟负载
};

/// @brief 系统监控
class SystemMonitor {

private:
  std::ifstream file_reader_;
  std::unordered_map<std::string, NetTraffic> prev_net_traffic_;
  std::chrono::steady_clock::time_point prev_net_time_;

private:
  /// @brief 读取CPU状态
  /// @return
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
  /// @brief  获取CPU使用率
  /// @return
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

  std::vector<NetInfo> GetNetInfo() {
    std::vector<NetInfo> net_infos;

    // 获取网络信息
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
      perror("getifaddrs");
      return {};
    }

    // 遍历网络信息
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == nullptr)
        continue;

      int family = ifa->ifa_addr->sa_family;
      if (family == AF_INET || family == AF_INET6) { // IPv4 or IPv6
        char host[NI_MAXHOST];
        int s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in)
                                                : sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
        if (s != 0) {
          std::cerr << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
          continue;
        }
        net_infos.push_back(
            {ifa->ifa_name, host, (family == AF_INET) ? "IPv4" : "IPv6"});
      }
    }
    freeifaddrs(ifaddr);
    return net_infos;
  }

  /// @brief 获取网络流量信息
  std::vector<NetTraffic> GetNetTraffic() {
    std::vector<NetTraffic> traffic_list;
    std::ifstream file("/proc/net/dev");
    std::string line;

    // 跳过前两行
    std::getline(file, line);
    std::getline(file, line);

    auto current_time = std::chrono::steady_clock::now();
    double time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                          current_time - prev_net_time_)
                          .count() / 1000.0;

    while (std::getline(file, line)) {
      std::istringstream iss(line);
      std::string iface_name;
      uint64_t rx_bytes, tx_bytes;
      uint64_t dummy;

      iface_name.resize(32);
      if (sscanf(line.c_str(), "%32[^:]: %lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %*lu %*lu %*lu %*lu %*lu",
                 &iface_name[0], &rx_bytes, &tx_bytes) >= 2) {
        // 去除空格
        iface_name.erase(iface_name.find_last_not_of(" \t") + 1);

        NetTraffic traffic;
        traffic.interface_name = iface_name;
        traffic.rx_bytes = rx_bytes;
        traffic.tx_bytes = tx_bytes;

        // 计算速率
        if (time_diff > 0.1 && prev_net_traffic_.find(iface_name) != prev_net_traffic_.end()) {
          auto& prev = prev_net_traffic_[iface_name];
          double rx_diff = rx_bytes - prev.rx_bytes;
          double tx_diff = tx_bytes - prev.tx_bytes;

          // 转换为 Mbps (1 byte = 8 bits, 1 Mbps = 1e6 bits/s)
          traffic.rx_mbps = (rx_diff * 8.0) / (time_diff * 1e6);
          traffic.tx_mbps = (tx_diff * 8.0) / (time_diff * 1e6);
        }

        traffic_list.push_back(traffic);
        prev_net_traffic_[iface_name] = traffic;
      }
    }

    prev_net_time_ = current_time;
    return traffic_list;
  }

  /// @brief 获取系统时间
  SystemTime GetSystemTime() {
    SystemTime st;
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    st.hour = tm.tm_hour;
    st.minute = tm.tm_min;
    st.second = tm.tm_sec;
    st.day = tm.tm_mday;
    st.month = tm.tm_mon + 1;
    st.year = tm.tm_year + 1900;

    return st;
  }

  /// @brief 获取CPU频率信息
  CpuFreqInfo GetCpuFreq() {
    CpuFreqInfo freq;

    // 读取当前频率
    std::ifstream cur_file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (cur_file.is_open()) {
      uint64_t khz;
      cur_file >> khz;
      freq.current_mhz = khz / 1000.0;
      cur_file.close();
    }

    // 读取最小频率
    std::ifstream min_file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
    if (min_file.is_open()) {
      uint64_t khz;
      min_file >> khz;
      freq.min_mhz = khz / 1000.0;
      min_file.close();
    }

    // 读取最大频率
    std::ifstream max_file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    if (max_file.is_open()) {
      uint64_t khz;
      max_file >> khz;
      freq.max_mhz = khz / 1000.0;
      max_file.close();
    }

    return freq;
  }

  /// @brief 获取系统负载
  SystemLoad GetSystemLoad() {
    SystemLoad load;
    double loadavg[3];

    if (getloadavg(loadavg, 3) == 3) {
      load.load1 = loadavg[0];
      load.load5 = loadavg[1];
      load.load15 = loadavg[2];
    }

    return load;
  }

  /// @brief 获取系统运行时间
  std::string GetUptime() {
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
      return "N/A";
    }

    uint64_t total_seconds = si.uptime;
    uint64_t days = total_seconds / (24 * 3600);
    uint64_t hours = (total_seconds % (24 * 3600)) / 3600;
    uint64_t minutes = (total_seconds % 3600) / 60;

    std::ostringstream oss;
    if (days > 0) {
      oss << days << "d ";
    }
    oss << hours << "h " << minutes << "m";
    return oss.str();
  }

public:
  SystemMonitor() : prev_net_time_(std::chrono::steady_clock::now()) {};
  ~SystemMonitor(){};
};
