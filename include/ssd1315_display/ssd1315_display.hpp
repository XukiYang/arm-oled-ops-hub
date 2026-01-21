#pragma once
#include "font.hpp"
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/i2c-dev.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

/* OLED硬件参数定义 */
constexpr uint8_t SSD1315_I2C_ADDRESS = 0x3C; // OLED的I2C地址
constexpr uint16_t SSD1315_WIDTH = 128;       // OLED宽度（像素）
constexpr uint16_t SSD1315_HEIGHT = 64;       // OLED高度（像素）
constexpr uint8_t SSD1315_PAGES = 8;          // OLED页数（高度/8）
constexpr size_t SSD1315_BUFFER_SIZE =        // 显示缓冲区大小
    (SSD1315_WIDTH * SSD1315_HEIGHT / 8);

class SSD1315Display {
private:
  std::string i2c_dev_path_;  // I2C设备路径
  int i2c_fd_ = -1;           // I2C文件描述符
  uint8_t *buffer_ = nullptr; // 显示缓冲区

private:
  /// @brief 设置页地址
  /// @param page 页地址(0-7)
  void SetPageAddress(uint8_t page) {
    SendCommand(0x22); // 设置页地址命令
    SendCommand(page); // 起始页
    SendCommand(0x07); // 结束页(共8页)
  }

  /// @brief 设置列地址
  /// @param col 起始列地址
  void SetColumnAddress(uint8_t col) {
    SendCommand(0x21);              // 设置列地址命令
    SendCommand(col);               // 起始列
    SendCommand(SSD1315_WIDTH - 1); // 结束列
  }

public:
  /// @brief 打开设备
  /// @param i2c_device
  SSD1315Display(std::string i2c_device) : i2c_dev_path_(i2c_device) {
    buffer_ = new uint8_t[SSD1315_BUFFER_SIZE];
    InitSSD1315();
  };

  /// @brief 初始化
  bool InitSSD1315() {
    // 打开设备
    i2c_fd_ = open(i2c_dev_path_.c_str(), O_RDWR);
    // 设置从设备地址
    ioctl(i2c_fd_, I2C_SLAVE, SSD1315_I2C_ADDRESS);

    // 初始化命令队列
    const uint8_t init_sequence[] = {
        0xAE,       // 关闭显示
        0xD5, 0x80, // 设置显示时钟分频比/振荡器频率
        0xA8, 0x3F, // 设置复用率(1/64)
        0xD3, 0x00, // 设置显示偏移
        0x40,       // 设置显示起始行
        0x8D, 0x14, // 启用电荷泵
        0x20, 0x00, // 设置内存地址模式为水平模式
        0xA1,       // 段重映射(0xA1正常,0xA0镜像)
        0xC8,       // COM输出扫描方向(0xC8正常,0xC0镜像)
        0xDA, 0x12, // 设置COM管脚配置
        0x81, 0xCF, // 设置对比度控制
        0xD9, 0xF1, // 设置预充电周期
        0xDB, 0x40, // 设置VCOMH电平
        0xA4,       // 整个显示开启(无背景)
        0xA6,       // 正常显示(非反色)
        0xAF        // 开启显示
    };

    // 发送初始化命令
    for (uint8_t cmd : init_sequence) {
      if (!SendCommand(cmd))
        return false;
    }
    return true;
  }

  /// @brief 下发命令
  /// @param command
  /// @return
  bool SendCommand(uint8_t command) {
    uint8_t packet[2] = {0x00, command}; // Co=0, D/C=0(命令模式)
    if (write(i2c_fd_, packet, 2) != 2) {
      std::cerr << "发送命令失败: 0x" << std::hex << (int)command << std::endl;
      return false;
    }
    return true;
  }

  /// @brief 清屏
  void ClearDisplay() { std::memset(buffer_, 0, SSD1315_BUFFER_SIZE); }

  /// @brief 刷新显示
  void RefreshDisplay() {
    // 检查I2C文件是否有效
    if (i2c_fd_ < 0)
      return;

    // 逐页发送数据
    for (uint8_t page = 0; page < SSD1315_PAGES; page++) {
      SetPageAddress(page); // 设置当前页
      SetColumnAddress(0);  // 设置列起始地址

      uint8_t packet[SSD1315_WIDTH + 1];
      packet[0] = 0x40; // 数据模式控制字节

      // 从缓冲区复制当前页的数据
      std::memcpy(packet + 1, buffer_ + (page * SSD1315_WIDTH), SSD1315_WIDTH);

      // 写入I2C数据
      if (write(i2c_fd_, packet, SSD1315_WIDTH + 1) != SSD1315_WIDTH + 1) {
        std::cerr << "写入I2C数据失败" << std::endl;
      }
    }
  };

  /// @brief 获取屏幕宽度
  /// @return 屏幕宽度(像素)
  int16_t Width() const { return SSD1315_WIDTH; }

  /// @brief 获取屏幕高度
  /// @return 屏幕高度(像素)
  int16_t Height() const { return SSD1315_HEIGHT; }

  /// @brief 绘制ASCII字符
  /// @brief 填充整个显示缓冲区
  /// @param color 填充颜色: 0-黑色, 1-白色
  void FillDisplay(uint8_t color = 0) {
    std::memset(buffer_, color ? 0xFF : 0x00, SSD1315_BUFFER_SIZE);
  }

  /// @brief 绘制单个像素点
  /// @param x X坐标
  /// @param y Y坐标
  /// @param color 像素颜色: 0-黑色, 1-白色, 2-反转
  void DrawPixel(int16_t x, int16_t y, uint8_t color) {
    // 检查坐标是否在有效范围内
    if (x < 0 || x >= Width() || y < 0 || y >= Height())
      return;

    // 计算像素在显存中的位置
    uint16_t byte_index = x + (y / 8) * SSD1315_WIDTH;
    uint8_t bit_mask = 1 << (y & 7); // 计算位位置

    // 根据颜色设置像素
    switch (color) {
    case 1: // 设置像素（白色）
      buffer_[byte_index] |= bit_mask;
      break;
    case 0: // 清除像素（黑色）
      buffer_[byte_index] &= ~bit_mask;
      break;
    case 2: // 反转像素
      buffer_[byte_index] ^= bit_mask;
      break;
    }
  }

  /// @brief 使用Bresenham算法绘制直线
  /// @param x0 起点X坐标
  /// @param y0 起点Y坐标
  /// @param x1 终点X坐标
  /// @param y1 终点Y坐标
  /// @param color 线条颜色
  void DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color) {
    int16_t dx = std::abs(x1 - x0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t dy = -std::abs(y1 - y0);
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    int16_t e2;

    while (true) {
      DrawPixel(x0, y0, color);
      if (x0 == x1 && y0 == y1)
        break;
      e2 = 2 * err;
      if (e2 >= dy) { // 垂直方向步进
        err += dy;
        x0 += sx;
      }
      if (e2 <= dx) { // 水平方向步进
        err += dx;
        y0 += sy;
      }
    }
  }

  /// @brief 绘制矩形边框
  /// @param x 左上角X坐标
  /// @param y 左上角Y坐标
  /// @param w 宽度
  /// @param h 高度
  /// @param color 边框颜色
  void DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    // 绘制上下边框
    for (int16_t i = x; i < x + w; i++) {
      DrawPixel(i, y, color);         // 上边框
      DrawPixel(i, y + h - 1, color); // 下边框
    }

    // 绘制左右边框
    for (int16_t i = y; i < y + h; i++) {
      DrawPixel(x, i, color);         // 左边框
      DrawPixel(x + w - 1, i, color); // 右边框
    }
  }

  /// @brief 绘制填充矩形
  /// @param x 左上角X坐标
  /// @param y 左上角Y坐标
  /// @param w 宽度
  /// @param h 高度
  /// @param color 填充颜色
  void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    for (int16_t i = y; i < y + h; i++) {
      for (int16_t j = x; j < x + w; j++) {
        DrawPixel(j, i, color);
      }
    }
  }

  /// @brief 绘制ASCII字符
  /// @param x 字符左上角X坐标
  /// @param y 字符左上角Y坐标
  /// @param c 要绘制的字符
  /// @param color 字符颜色
  /// @param size 字符缩放倍数
  void DrawChar(int16_t x, int16_t y, char c, uint8_t color = 1,
                uint8_t size = 1) {
    // 检查字符位置是否有效
    if (x >= Width() || y >= Height() || (x + 6 * size - 1 < 0) ||
        (y + 8 * size - 1 < 0))
      return;

    // 替换无效字符为问号
    if (c < 32 || c > 126)
      c = '?';

    // 绘制字符点阵
    for (int8_t col = 0; col < 6; col++) {
      uint8_t line_data;
      if (col == 5) {
        line_data = 0x00; // 最后一列为空
      } else {
        // 获取字体数据
        line_data = fonts::FONT_5X8[(c - 32) * 5 + col];
      }

      // 绘制当前列的点阵数据
      for (int8_t row = 0; row < 8; row++) {
        if (line_data & 0x1) { // 当前位是否置位
          if (size == 1) {
            DrawPixel(x + col, y + row, color);
          } else {
            // 放大字符
            FillRect(x + col * size, y + row * size, size, size, color);
          }
        }
        line_data >>= 1; // 移至下一个位
      }
    }
  }

  /// @brief 绘制字符串
  /// @param x 字符串左上角X坐标
  /// @param y 字符串左上角Y坐标
  /// @param str 要绘制的字符串
  /// @param color 字符串颜色
  /// @param size 字符缩放倍数
  void DrawString(int16_t x, int16_t y, const std::string &str,
                  uint8_t color = 1, uint8_t size = 1) {
    int16_t origin_x = x; // 保存初始X坐标
    for (char c : str) {
      if (c == '\n') { // 换行符处理
        y += 8 * size; // 下移一行
        x = origin_x;  // 回到起始位置
      } else {
        DrawChar(x, y, c, color, size);
        x += 6 * size; // 移动到下一个字符位置
      }
    }
  }

  /// @brief 设置OLED对比度
  /// @param contrast 对比度值(0-255)
  void SetContrast(uint8_t contrast) {
    SendCommand(0x81);     // 设置对比度命令
    SendCommand(contrast); // 对比度值
  }

  /// @brief 绘制圆形
  /// @param x0 圆心X坐标
  /// @param y0 圆心Y坐标
  /// @param r 半径
  /// @param color 颜色
  void DrawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    int16_t x = -r, y = 0, err = 2 - 2 * r;
    do {
      DrawPixel(x0 - x, y0 + y, color);
      DrawPixel(x0 - y, y0 - x, color);
      DrawPixel(x0 + x, y0 - y, color);
      DrawPixel(x0 + y, y0 + x, color);
      r = err;
      if (r <= y) err += ++y * 2 + 1;
      if (r > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
  }

  /// @brief 绘制填充圆形
  /// @param x0 圆心X坐标
  /// @param y0 圆心Y坐标
  /// @param r 半径
  /// @param color 颜色
  void FillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {
    int16_t x = -r, y = 0, err = 2 - 2 * r;
    do {
      for (int16_t i = x0 + x; i <= x0 - x; i++) {
        DrawPixel(i, y0 + y, color);
        DrawPixel(i, y0 - y, color);
      }
      r = err;
      if (r <= y) err += ++y * 2 + 1;
      if (r > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
  }

  /// @brief 绘制进度条
  /// @param x 左上角X坐标
  /// @param y 左上角Y坐标
  /// @param w 宽度
  /// @param h 高度
  /// @param progress 进度(0-100)
  /// @param color 颜色
  void DrawProgressBar(int16_t x, int16_t y, int16_t w, int16_t h, 
                       uint8_t progress, uint8_t color) {
    // 绘制边框
    DrawRect(x, y, w, h, color);
    
    // 计算进度宽度
    int16_t bar_width = (progress * (w - 2)) / 100;
    if (bar_width > w - 2) bar_width = w - 2;
    if (bar_width < 0) bar_width = 0;
    
    // 绘制填充部分
    if (bar_width > 0) {
      FillRect(x + 1, y + 1, bar_width, h - 2, color);
    }
  }

  /// @brief 绘制圆角矩形
  /// @param x 左上角X坐标
  /// @param y 左上角Y坐标
  /// @param w 宽度
  /// @param h 高度
  /// @param r 圆角半径
  /// @param color 颜色
  void DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                     int16_t r, uint8_t color) {
    DrawLine(x + r, y, x + w - r - 1, y, color);
    DrawLine(x + r, y + h - 1, x + w - r - 1, y + h - 1, color);
    DrawLine(x, y + r, x, y + h - r - 1, color);
    DrawLine(x + w - 1, y + r, x + w - 1, y + h - r - 1, color);
    
    DrawCircleHelper(x + r, y + r, r, 1, color);
    DrawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    DrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    DrawCircleHelper(x + r, y + h - r - 1, r, 8, color);
  }

  /// @brief 绘制填充圆角矩形
  /// @param x 左上角X坐标
  /// @param y 左上角Y坐标
  /// @param w 宽度
  /// @param h 高度
  /// @param r 圆角半径
  /// @param color 颜色
  void FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                     int16_t r, uint8_t color) {
    FillRect(x + r, y, w - 2 * r, h, color);
    FillRect(x, y + r, r, h - 2 * r, color);
    FillRect(x + w - r, y + r, r, h - 2 * r, color);
    
    FillCircleHelper(x + r, y + r, r, 1, h - 2 * r + 1, color);
    FillCircleHelper(x + w - r - 1, y + r, r, 2, h - 2 * r + 1, color);
  }

  /// @brief 圆形辅助函数
  void DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, 
                        uint8_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
      if (f >= 0) {
        y--;
        ddF_y += 2;
        f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;
      if (cornername & 0x4) {
        DrawPixel(x0 + x, y0 + y, color);
        DrawPixel(x0 + y, y0 + x, color);
      }
      if (cornername & 0x2) {
        DrawPixel(x0 + x, y0 - y, color);
        DrawPixel(x0 + y, y0 - x, color);
      }
      if (cornername & 0x8) {
        DrawPixel(x0 - y, y0 + x, color);
        DrawPixel(x0 - x, y0 + y, color);
      }
      if (cornername & 0x1) {
        DrawPixel(x0 - y, y0 - x, color);
        DrawPixel(x0 - x, y0 - y, color);
      }
    }
  }

  /// @brief 填充圆形辅助函数
  void FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, 
                        int16_t delta, uint8_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
      if (f >= 0) {
        y--;
        ddF_y += 2;
        f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      if (cornername & 0x1) {
        DrawLine(x0 - x, y0 + y, x0 - x, y0 + y - delta, color);
        DrawLine(x0 - y, y0 + x, x0 - y, y0 + x - delta, color);
      }
      if (cornername & 0x2) {
        DrawLine(x0 + x, y0 + y, x0 + x, y0 + y - delta, color);
        DrawLine(x0 + y, y0 + x, x0 + y, y0 + x - delta, color);
      }
      if (cornername & 0x4) {
        DrawLine(x0 - x, y0 - y + delta, x0 - x, y0 - y, color);
        DrawLine(x0 - y, y0 - x + delta, x0 - y, y0 - x, color);
      }
      if (cornername & 0x8) {
        DrawLine(x0 + x, y0 - y + delta, x0 + x, y0 - y, color);
        DrawLine(x0 + y, y0 - x + delta, x0 + y, y0 - x, color);
      }
    }
  }

  ~SSD1315Display() {
    // 释放显存缓冲区
    delete[] buffer_;
    // 关闭I2C设备
    if (i2c_fd_ >= 0) {
      close(i2c_fd_);
    }
  };
};
