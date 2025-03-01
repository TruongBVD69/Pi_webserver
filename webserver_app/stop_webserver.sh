#!/bin/bash
#
# Đường dẫn đến script webserver.py
WEB_SERVER_PATH="/home/Pizero2W/webserver_app/webserver.py"

# Kiểm tra xem tiến trình Python đang chạy webserver có tồn tại không
PID=$(pgrep -f "$WEB_SERVER_PATH")

if [ -z "$PID" ]; then
    echo -e "\033[93mWarning: Webserver is not running.\033[0m"
else
    # Dừng tiến trình Flask (Python)
    echo "Stopping the webserver..."
    pkill -f "$WEB_SERVER_PATH"
    echo -e "\033[92mWebserver stopped successfully.\033[0m"
fi
