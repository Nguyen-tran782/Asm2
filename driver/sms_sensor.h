/*
 * File: sms_sensor.h
 * Purpose: Header file chung cho Kernel Driver và Userspace App (Sensor Monitoring System)
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#ifndef SMS_SENSOR_H
#define SMS_SENSOR_H

#include <linux/ioctl.h>

/* Định nghĩa Magic Number cho thiết bị SMS */
#define SMS_MAGIC 'S'

/* 
 * Định nghĩa mã lệnh IOCTL: SMS_SET_INTERVAL
 * Dùng để cài đặt khoảng thời gian tối thiểu giữa 2 lần đọc (tính bằng ms)
 */
#define SMS_SET_INTERVAL _IOW(SMS_MAGIC, 1, unsigned int)

/* Chiều dài tối đa của một dòng dữ liệu CSV trả về */
#define SMS_BUFFER_SIZE 128

#endif /* SMS_SENSOR_H */
