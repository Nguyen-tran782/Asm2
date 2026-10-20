/*
 * File: circular_buffer.h
 * Purpose: Thread-safe circular buffer cho Sensor Monitoring System
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <pthread.h>

/* Cấu trúc lưu 1 dòng dữ liệu cảm biến */
typedef struct {
    long long timestamp_ms;
    float temperature;
    float humidity;
} SensorData;

/* Cấu trúc Bộ đệm vòng đa luồng */
typedef struct {
    SensorData *buffer;
    int head;
    int tail;
    int max_size;
    int current_size;
    int overflow_count;
    unsigned int total_produced;
    unsigned int total_consumed;
    
    /* Khóa bảo vệ đa luồng */
    pthread_mutex_t mutex;
    pthread_cond_t cond_non_empty;
    pthread_cond_t cond_non_full;
} CircularBuffer;

/* Các hàm thao tác với bộ đệm */
void cb_init(CircularBuffer *cb, int size);
void cb_cleanup(CircularBuffer *cb);
void cb_push(CircularBuffer *cb, SensorData data);
SensorData cb_pop(CircularBuffer *cb);

#endif /* CIRCULAR_BUFFER_H */
