#!/bin/bash

# 服务配置
# sudo nano /etc/systemd/system/arm-oled-ops-hub.service
# 重启并加载服务
# sudo systemctl daemon-reload
# sudo systemctl restart arm-oled-ops-hub
# sudo systemctl status arm-oled-ops-hub
# 停止服务
# sudo systemctl stop arm-oled-ops-hub.service

tag=$(git describe --tags --exact-match 2>/dev/null || echo "unknown")
cd ./build/version
tar -xf "arm-oled-ops-hub_$tag.tar.xz" -C /home/orangepi/services/arm-oled-ops-hub
path=$(ls /home/orangepi/services/arm-oled-ops-hub)

printf "重启服务\n"
echo "orangepi" | sudo -S systemctl daemon-reload
echo "orangepi" | sudo -S systemctl restart arm-oled-ops-hub
echo "orangepi" | sudo -S systemctl status arm-oled-ops-hub

printf "更新完成\n"
printf "Tag:  %s\n" "$tag"
printf "Path: %s\n" "$path"