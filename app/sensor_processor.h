/*
 * File: sensor_processor.h
 * Purpose: Header cho luồng Sensor và Processor
 */
#ifndef SENSOR_PROCESSOR_H
#define SENSOR_PROCESSOR_H

#include "circular_buffer.h"

/* Cấu trúc tham số truyền vào các luồng */
typedef struct {
    CircularBuffer* cb;
    int interval_ms;
    float temp_threshold;
    float humid_threshold;
    int verbose;
} ThreadArgs;

void* sensor_thread_func(void* arg);
void* processor_thread_func(void* arg);

#endif
