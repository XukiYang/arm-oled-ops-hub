#!/bin/bash

# arm-oled-ops-hub 统一管理脚本
# 支持的命令:
#   ./arm-oled.sh build    - 仅编译
#   ./arm-oled.sh run      - 编译并运行
#   ./arm-oled.sh package  - 编译并打包
#   ./arm-oled.sh update   - 编译打包并更新服务
#   ./arm-oled.sh help     - 显示帮助

SCRIPT_NAME="$(basename "$0")"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVICE_NAME="arm-oled-ops-hub"
SUDO_PASS="orangepi"

# 显示帮助
show_help() {
    echo "arm-oled-ops-hub 统一管理脚本"
    echo ""
    echo "用法: $SCRIPT_NAME <命令>"
    echo ""
    echo "命令:"
    echo "  build    - 仅编译项目"
    echo "  run      - 编译并运行程序"
    echo "  package  - 编译并打包为 tar.xz"
    echo "  update   - 编译打包并更新服务"
    echo "  help     - 显示此帮助"
    echo ""
    echo "示例:"
    echo "  $SCRIPT_NAME build"
    echo "  $SCRIPT_NAME run"
    echo "  $SCRIPT_NAME package"
    echo "  $SCRIPT_NAME update"
}

# 编译函数
do_build() {
    echo "[编译] 开始编译..."
    cd "$PROJECT_DIR/build" || {
        echo "[错误] 无法进入 build 目录"
        exit 1
    }
    
    cmake .. || {
        echo "[错误] cmake 失败"
        exit 1
    }
    
    make -j$(nproc) || {
        echo "[错误] make 失败"
        exit 1
    }
    
    echo "[编译] 编译完成"
}

# 运行函数
do_run() {
    echo "[运行] 编译并运行..."
    do_build
    
    cd "$PROJECT_DIR/build/output" || {
        echo "[错误] 无法进入 output 目录"
        exit 1
    }
    
    sudo ./$SERVICE_NAME || {
        echo "[错误] 运行程序失败"
        exit 1
    }
}

# 打包函数
do_package() {
    echo "[打包] 编译并打包..."
    do_build
    
    cd "$PROJECT_DIR/build" || {
        echo "[错误] 无法进入 build 目录"
        exit 1
    }
    
    tag=$(git describe --tags --exact-match 2>/dev/null || echo "unknown")
    
    if [ ! -d "version" ]; then
        mkdir version
    fi
    
    tar -cJf "arm-oled-ops-hub_$tag.tar.xz" ./output || {
        echo "[错误] 打包失败"
        exit 1
    }
    
    mv "arm-oled-ops-hub_$tag.tar.xz" ./version || {
        echo "[错误] 移动文件失败"
        exit 1
    }
    
    echo "[打包] 打包完成"
    echo "  Tag:  $tag"
    echo "  Path: ./version/arm-oled-ops-hub_$tag.tar.xz"
}

# 更新函数
do_update() {
    echo "[更新] 编译打包并更新服务..."
    do_package
    
    tag=$(git describe --tags --exact-match 2>/dev/null || echo "unknown")
    
    cd "$PROJECT_DIR/build/version" || {
        echo "[错误] 无法进入 version 目录"
        exit 1
    }
    
    tar -xf "arm-oled-ops-hub_$tag.tar.xz" -C /home/orangepi/services/arm-oled-ops-hub || {
        echo "[错误] 解压失败"
        exit 1
    }
    
    echo "[更新] 重启服务..."
    echo "$SUDO_PASS" | sudo -S systemctl daemon-reload || {
        echo "[错误] 重载服务失败"
        exit 1
    }
    
    echo "$SUDO_PASS" | sudo -S systemctl restart $SERVICE_NAME || {
        echo "[错误] 重启服务失败"
        exit 1
    }
    
    echo "$SUDO_PASS" | sudo -S systemctl status $SERVICE_NAME --no-pager || {
        echo "[错误] 查看服务状态失败"
        exit 1
    }
    
    echo "[更新] 更新完成"
    echo "  Tag:  $tag"
    echo "  Path: /home/orangepi/services/arm-oled-ops-hub"
}

# 主函数
main() {
    if [ $# -eq 0 ]; then
        show_help
        exit 1
    fi
    
    case "$1" in
        build)
            do_build
            ;;
        run)
            do_run
            ;;
        package)
            do_package
            ;;
        update)
            do_update
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            echo "[错误] 未知命令: $1"
            show_help
            exit 1
            ;;
    esac
}

# 执行主函数
main "$@"