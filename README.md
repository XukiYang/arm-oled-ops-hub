# **ARM OLED OPS Hub**  
**基于SSD1315 OLED的嵌入式监控系统**  

---

## **📌 项目简介**  
ARM OLED OPS Hub 是一个轻量级嵌入式监控解决方案，基于 **SSD1315 OLED 显示屏**，实时显示设备关键状态（CPU/内存/磁盘/温度），并支持 **动态风扇控制** 与 **阈值告警**。适用于 Orange Pi、Raspberry Pi 等嵌入式设备，提供直观的系统监控体验。  

---

## **✨ 功能特性**  

### **📊 实时监控**  
- **CPU 使用率**（多核支持）  
- **内存占用**（RAM/Swap）  
- **磁盘使用率**（根目录/挂载点）  
- **温度监测**（CPU/GPU/DDR/VE）  
- **网络状态**（IP、上下行速率，支持3个网卡）  

### **⚙️ 智能控制**  
- **动态风扇控制**（基于温度阈值，通过 GPIO 2pin 控制）  
- **阈值告警**（CPU/温度过高时触发警告）  

### **🛠️ 技术栈**  
- **硬件**：SSD1315 OLED（I2C/SPI）、ARM SBC（Orange Pi/RPi）  
- **软件**：Python/C++（嵌入式优化）、Linux 系统调用（`/proc`、`/sys`）  

---

## **🚀 快速开始**  

### **1. 硬件连接**  
- **OLED 接线**（I2C 示例）：  
  ```
  SSD1315      Orange Pi  
  VCC    →    3.3V  
  GND    →    GND  
  SDA    →    I2C0_SDA (GPIO2)  
  SCL    →    I2C0_SCL (GPIO3)  
  ```
- **风扇控制**（可选）：  
  ```
  风扇+    →    GPIO17  
  风扇-    →    GND  
  ```

### **2. 软件依赖**  
```bash
sudo apt update
sudo apt install python3-pip i2c-tools  # 基础工具
```

### **3. 运行项目**  
```bash
git clone https://github.com/XukiYang/arm-oled-ops-hub.git
# or git clone git@github.com:XukiYang/arm-oled-ops-hub.git
cd arm-oled-ops-hub
mkdir build
cd build
cmake .. && make
# run it!
./arm-oled-ops-hub
```

---

## **📅 开发计划**  
### **✅ 已实现**  
- 基础监控（CPU/内存/磁盘/温度）  
- 多网卡信息显示  

### **🔜 待开发**  
- [ ] **温度阈值风扇控制**（自动调速）  
- [ ] **历史数据记录**（CSV/轻量级数据库）  
- [ ] **Web 远程监控**（Flask/WebSocket）  
- [ ] **低功耗模式**（动态刷新率调节）  
- [ ] **配置文件支持**（初始化参数可配）  

---

## **📜 开源协议**  
MIT License | 贡献欢迎 👋  