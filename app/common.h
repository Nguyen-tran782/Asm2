/*
 * File: common.h
 * Purpose: Các cấu trúc dữ liệu và hằng số dùng chung cho toàn bộ ứng dụng
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#ifndef COMMON_H
#define COMMON_H

#include <mqueue.h>

/* Tên của các POSIX Message Queue dùng để IPC (Giao tiếp giữa các tiến trình) */
#define ALARM_QUEUE_NAME "/sms_alarm_queue"
#define LOG_QUEUE_NAME "/sms_log_queue"

/* Cấu trúc tin nhắn cảnh báo (Alarm) */
typedef struct {
    long long timestamp_ms;
    int alert_type; /* 1: TEMP_HIGH, 2: HUMID_HIGH, 3: TEMP_SPIKE */
    float value;
    float threshold;
} AlertMsg;

/* Cấu trúc tin nhắn Log */
typedef struct {
    char text[256];
} LogMsg;

/* Biến toàn cục kiểm soát trạng thái chạy (dùng extern để các file khác đọc được) */
extern int is_running;

#endif /* COMMON_H */
