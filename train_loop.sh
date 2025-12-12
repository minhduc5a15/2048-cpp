#!/bin/bash
echo "Bắt đầu chuỗi Training vô tận..."

while true; do
    echo ">>> Bắt đầu phiên mới: $(date)"

    # Chạy train 5 triệu ván
    ./build/bin/2048-train 2000000

    echo ">>> Đã xong phiên này. Nghỉ 15s trước khi chạy lại..."
    sleep 15
done
