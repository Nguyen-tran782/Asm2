/*
 * File: circular_buffer.c
 * Purpose: Triển khai các hàm cho Circular Buffer đa luồng an toàn (Thread-safe)
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#include "circular_buffer.h"
#include <stdlib.h>
#include <stdio.h>

/* Khởi tạo bộ đệm và các cơ chế đồng bộ */
void cb_init(CircularBuffer *cb, int size) {
    cb->buffer = (SensorData *)malloc(sizeof(SensorData) * size);
    cb->head = 0;
    cb->tail = 0;
    cb->max_size = size;
    cb->current_size = 0;
    cb->overflow_count = 0;
    cb->total_produced = 0;
    cb->total_consumed = 0;
    
    /* Khởi tạo Khóa Mutex và Biến điều kiện (Condition Variables) */
    pthread_mutex_init(&cb->mutex, NULL);
    pthread_cond_init(&cb->cond_non_empty, NULL);
    pthread_cond_init(&cb->cond_non_full, NULL);
}

/* Hàm Ghi dữ liệu vào bộ đệm (dành cho Producer/Reader Thread) */
void cb_push(CircularBuffer *cb, SensorData data) {
    pthread_mutex_lock(&cb->mutex); // Khóa lại, không cho ai can thiệp
    
    /* Nếu bộ đệm đầy, Thread ghi sẽ phải tạm ngủ và chờ đến khi có chỗ trống */
    while (cb->current_size == cb->max_size) {
        cb->overflow_count++;
        pthread_cond_wait(&cb->cond_non_full, &cb->mutex);
    }
    
    /* Ghi dữ liệu vào đuôi (tail) của bộ đệm vòng */
    cb->buffer[cb->tail] = data;
    cb->tail = (cb->tail + 1) % cb->max_size;
    cb->current_size++;
    cb->total_produced++;
    
    /* Ghi xong thì đánh thức Thread đang chờ đọc (Consumer) dậy */
    pthread_cond_signal(&cb->cond_non_empty);
    
    pthread_mutex_unlock(&cb->mutex); // Mở khóa cho người khác dùng
}

/* Hàm Lấy dữ liệu từ bộ đệm ra (dành cho Consumer/Processor Thread) */
SensorData cb_pop(CircularBuffer *cb) {
    SensorData data;
    
    pthread_mutex_lock(&cb->mutex); // Khóa lại
    
    /* Nếu bộ đệm trống, Thread đọc sẽ phải ngủ để chờ có dữ liệu mới */
    while (cb->current_size == 0) {
        pthread_cond_wait(&cb->cond_non_empty, &cb->mutex);
    }
    
    /* Lấy dữ liệu từ đầu (head) của bộ đệm vòng */
    data = cb->buffer[cb->head];
    cb->head = (cb->head + 1) % cb->max_size;
    cb->current_size--;
    cb->total_consumed++;
    
    /* Rút dữ liệu ra xong, đánh thức Thread ghi dậy (nếu trước đó bộ đệm bị đầy) */
    pthread_cond_signal(&cb->cond_non_full);
    
    pthread_mutex_unlock(&cb->mutex); // Mở khóa
    
    return data;
}

/* Dọn dẹp bộ nhớ trước khi tắt chương trình */
void cb_cleanup(CircularBuffer *cb) {
    free(cb->buffer);
    pthread_mutex_destroy(&cb->mutex);
    pthread_cond_destroy(&cb->cond_non_empty);
    pthread_cond_destroy(&cb->cond_non_full);
}
