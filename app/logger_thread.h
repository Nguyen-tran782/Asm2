/*
 * File: logger_thread.h
 * Purpose: Header cho luồng ghi file log
 */
#ifndef LOGGER_THREAD_H
#define LOGGER_THREAD_H

void* logger_thread_func(void* arg);
void send_log(const char* format, ...); /* Hàm tiện ích để các thread khác gọi khi muốn ghi log */

#endif
