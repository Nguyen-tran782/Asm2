/*
 * File: sms_app.c
 * Purpose: Main application entry cho Sensor Monitoring System
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#include "common.h"
#include "circular_buffer.h"
#include "logger_thread.h"
#include "alert_thread.h"
#include "sensor_processor.h"
#include "../driver/sms_sensor.h" /* Tận dụng lại file header của Driver */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int is_running = 1;

/* Xử lý tín hiệu ngắt (Ctrl+C) để Graceful Shutdown */
void handle_sigint(int sig) {
    (void)sig; /* Bỏ qua cảnh báo unused */
    printf("\n[MAIN] Nhan tin hieu tat (SIGINT). Dang don dep he thong...\n");
    is_running = 0;
}

int main(int argc, char* argv[]) {
    int opt;
    int interval_ms = 500;
    float temp_thresh = 40.0;
    float humid_thresh = 80.0;
    int duration_sec = 60;
    int verbose = 0;

    /* Đọc các tham số cấu hình từ dòng lệnh (-i, -t, -h, -d, -v) */
    while ((opt = getopt(argc, argv, "i:t:h:d:v")) != -1) {
        switch (opt) {
            case 'i': interval_ms = atoi(optarg); break;
            case 't': temp_thresh = atof(optarg); break;
            case 'h': humid_thresh = atof(optarg); break;
            case 'd': duration_sec = atoi(optarg); break;
            case 'v': verbose = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-i interval_ms] [-t temp_thresh] [-h humid_thresh] [-d duration_sec] [-v]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    signal(SIGINT, handle_sigint);

    printf("=== SENSOR MONITORING SYSTEM (SMS) ===\n");
    printf("Author: Tran Dinh Khoi Nguyen - SE200558\n");
    printf("Interval: %d ms | Temp Threshold: %.1f C | Humid Threshold: %.1f %%\n", 
            interval_ms, temp_thresh, humid_thresh);
    
    /* Cấu hình IOCTL cho driver (Yêu cầu ở Task 2) */
    int fd = open("/dev/sms_sensor", O_RDONLY);
    if (fd >= 0) {
        ioctl(fd, SMS_SET_INTERVAL, &interval_ms);
        close(fd);
    } else {
        perror("[MAIN] Canh bao: Khong mo duoc /dev/sms_sensor de set interval");
    }

    CircularBuffer cb;
    cb_init(&cb, 64); /* Cấp phát buffer 64 phần tử */

    ThreadArgs args = { &cb, interval_ms, temp_thresh, humid_thresh, verbose };
    pthread_t logger_tid, alert_tid, sensor_tid, processor_tid;

    /* Khởi chạy 4 luồng song song */
    pthread_create(&logger_tid, NULL, logger_thread_func, NULL);
    pthread_create(&alert_tid, NULL, alert_thread_func, NULL);
    pthread_create(&sensor_tid, NULL, sensor_thread_func, &args);
    pthread_create(&processor_tid, NULL, processor_thread_func, &args);

    /* Chờ hết thời gian cài đặt hoặc khi người dùng bấm Ctrl+C */
    if (duration_sec > 0) {
        int waited = 0;
        while (waited < duration_sec && is_running) {
            sleep(1);
            waited++;
        }
        is_running = 0; /* Gửi tín hiệu dừng cho tất cả các luồng */
	SensorData dummy = {0, 0.0, 0.0};
        cb_push(&cb, dummy); /* Bơm dữ liệu rỗng để đánh thức Processor Thread */
    }

    /* Đợi các luồng dọn dẹp và kết thúc */
    pthread_join(sensor_tid, NULL);
    pthread_join(processor_tid, NULL);
    pthread_join(alert_tid, NULL);
    pthread_join(logger_tid, NULL);

    /* In thống kê tổng kết */
    printf("\n=== THONG KE KET THUC ===\n");
    printf("Tong so mau da doc    : %u\n", cb.total_produced);
    printf("Tong so mau da xu ly  : %u\n", cb.total_consumed);
    printf("So lan tran bo dem    : %d\n", cb.overflow_count);

    cb_cleanup(&cb);
    printf("He thong da tat an toan (Graceful Shutdown).\n");

    return EXIT_SUCCESS;
}
