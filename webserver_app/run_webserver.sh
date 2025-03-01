#!/bin/bash
sleep 10

# Chuyển đến thư mục chứa webserver (cập nhật đường dẫn nếu cần)
cd /home/Pizero2W/webserver_app

# Đường dẫn đến script Python của webserver
SCRIPT_PATH="/home/Pizero2W/webserver_app/webserver.py"

# File log để lưu output của webserver
LOG_FILE="webserver.log"

# Kiểm tra nếu script Python tồn tại
if [[ ! -f "$SCRIPT_PATH" ]]; then
    echo "Can not find path $SCRIPT_PATH"
    exit 1
fi

# Chạy webserver với nohup
echo "Starting  webserver..."
nohup python3 "$SCRIPT_PATH" > "$LOG_FILE" 2>&1 &

# Lấy PID của tiến trình webserver
SERVER_PID=$!
echo -e "\033[92mWebserver is running with PID:\033[0m $SERVER_PID. \033[92mLog will write to\033[0m $LOG_FILE"
echo "Run command: 'tail -f webserver.log' to watch log" 
