/*
 * File: sensor_processor.c
 * Purpose: Sensor Thread đọc thiết bị, Processor Thread xử lý dữ liệu và phát cảnh báo
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#include "sensor_processor.h"
#include "common.h"
#include "logger_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <mqueue.h>

#define DEVICE_PATH "/dev/sms_sensor"

/* ====== SENSOR THREAD ====== */
void* sensor_thread_func(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    char buf[128];
    SensorData data;
    struct timespec next_wake_time;

    clock_gettime(CLOCK_MONOTONIC, &next_wake_time);

    while (is_running) {
        next_wake_time.tv_nsec += args->interval_ms * 1000000L;
        while (next_wake_time.tv_nsec >= 1000000000L) {
            next_wake_time.tv_sec++;
            next_wake_time.tv_nsec -= 1000000000L;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wake_time, NULL);

        int fd = open(DEVICE_PATH, O_RDONLY);
        if (fd >= 0) {
            if (read(fd, buf, sizeof(buf)) > 0) {
                if (sscanf(buf, "%lld,%f,%f", &data.timestamp_ms, &data.temperature, &data.humidity) == 3) {
                    cb_push(args->cb, data);
                    if (args->verbose) {
                        printf("[SENSOR] Read: %lld | T: %.1fC | H: %.1f%%\n", 
                               data.timestamp_ms, data.temperature, data.humidity);
                    }
                }
            }
            close(fd);
        }
    }
    return NULL;
}

/* ====== PROCESSOR THREAD ====== */
void* processor_thread_func(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    SensorData current_data;
    float prev_temp = -100.0;
    mqd_t alarm_mq = mq_open(ALARM_QUEUE_NAME, O_WRONLY);

    while (is_running) {
        current_data = cb_pop(args->cb); /* Lấy từ Circular Buffer */
        
        /* Ghi log dữ liệu đã xử lý */
        send_log("[INFO][%lld] Processed: T=%.1f H=%.1f\n", 
                 current_data.timestamp_ms, current_data.temperature, current_data.humidity);

        if (alarm_mq != (mqd_t)-1) {
            AlertMsg msg;
            msg.timestamp_ms = current_data.timestamp_ms;
            int alert_triggered = 0;

            /* Check Nhiệt độ cao */
            if (current_data.temperature > args->temp_threshold) {
                msg.alert_type = 1; msg.value = current_data.temperature; msg.threshold = args->temp_threshold;
                mq_send(alarm_mq, (const char*)&msg, sizeof(AlertMsg), 0);
                alert_triggered = 1;
            }
            /* Check Độ ẩm cao */
            if (current_data.humidity > args->humid_threshold) {
                msg.alert_type = 2; msg.value = current_data.humidity; msg.threshold = args->humid_threshold;
                mq_send(alarm_mq, (const char*)&msg, sizeof(AlertMsg), 0);
                alert_triggered = 1;
            }
            /* Check Nhiệt độ thay đổi đột ngột (> 5 độ) */
            if (prev_temp != -100.0 && (current_data.temperature - prev_temp > 5.0 || prev_temp - current_data.temperature > 5.0)) {

                msg.alert_type = 3; msg.value = current_data.temperature - prev_temp; msg.threshold = 5.0;
                mq_send(alarm_mq, (const char*)&msg, sizeof(AlertMsg), 0);
                alert_triggered = 1;
            }
            
            if (alert_triggered) usleep(50000); /* Chống dội (debounce) việc gửi alarm liên tục */
        }
        prev_temp = current_data.temperature;
prev_temp = current_data.temperature;
        
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        long latency_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        if (args->verbose) {
            printf("[PERF] Do tre xu ly (Latency): %ld us\n", latency_us);
        }
        
    } 

    if (alarm_mq != (mqd_t)-1) mq_close(alarm_mq);
    return NULL;
}
    }

    if (alarm_mq != (mqd_t)-1) mq_close(alarm_mq);
    return NULL;
}
