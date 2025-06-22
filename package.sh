#!/bin/bash
./make.sh
tag=$(git describe --tags --exact-match 2>/dev/null || echo "unknown")
cd build
if [ ! -d "version" ]; then
    mkdir version
fi
tar -cJf "arm-oled-ops-hub_$tag.tar.xz" ./output
mv "arm-oled-ops-hub_$tag.tar.xz" ./version
path=$(ls ./version/arm-oled-ops-hub_$tag.tar.xz )
printf "\n打包完成\n"
printf "Tag:  %s\n" "$tag"
printf "Path: %s\n" "$path"