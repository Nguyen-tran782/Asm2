/*
 * File: logger_thread.c
 * Purpose: Luồng nhận tin nhắn và ghi định kỳ ra sms.log (Rotate khi > 1MB)
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#include "logger_thread.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/stat.h>
#define LOG_FILE "sms.log"
#define LOG_FILE_BAK "sms.log.1"
#define MAX_LOG_SIZE (1024 * 1024) /* 1MB */

/* Hàm tiện ích: Đóng gói chuỗi (string) thành LogMsg và ném vào Message Queue */
void send_log(const char* format, ...) {
    LogMsg msg;
    va_list args;
    va_start(args, format);
    vsnprintf(msg.text, sizeof(msg.text), format, args);
    va_end(args);

    mqd_t mq = mq_open(LOG_QUEUE_NAME, O_WRONLY);
    if (mq != (mqd_t)-1) {
        mq_send(mq, (const char*)&msg, sizeof(LogMsg), 0);
        mq_close(mq);
    }
}

/* Hàm chính của Logger Thread */
void* logger_thread_func(void* arg) {
	(void)arg;
    mqd_t log_mq;
    struct mq_attr attr;
    FILE *fp;
    LogMsg msg;
    int flush_counter = 0;

    /* Cấu hình POSIX Message Queue cho Log */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(LogMsg);
    attr.mq_curmsgs = 0;

    mq_unlink(LOG_QUEUE_NAME); /* Xóa queue cũ nếu chương trình trước đó bị crash */
    log_mq = mq_open(LOG_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);
    
    fp = fopen(LOG_FILE, "a");

    while (is_running) {
        /* Đọc non-block để có thể thoát vòng lặp ngay khi is_running = 0 (Lúc bấm Ctrl+C) */
        if (mq_receive(log_mq, (char*)&msg, sizeof(LogMsg), NULL) > 0) {
            if (fp) {
                fprintf(fp, "%s", msg.text);
                flush_counter++;
                
                /* Flush định kỳ (Giả lập mỗi 5 dòng để giảm I/O theo yêu cầu đề bài) */
                if (flush_counter >= 5) {
                    fflush(fp);
                    flush_counter = 0;
                    
                    /* Kiểm tra dung lượng file, nếu lớn hơn 1MB thì Rotate */
                    struct stat st;
                    if (stat(LOG_FILE, &st) == 0 && st.st_size >= MAX_LOG_SIZE) {
                        fclose(fp);
                        rename(LOG_FILE, LOG_FILE_BAK);
                        fp = fopen(LOG_FILE, "a");
                    }
                }
            }
        } else {
            usleep(100000); /* Ngủ 100ms nếu không có tin nhắn log nào để tránh tốn CPU */
        }
    }

    /* Dọn dẹp trước khi tắt luồng */
    if (fp) {
        fflush(fp);
        fclose(fp);
    }
    mq_close(log_mq);
    mq_unlink(LOG_QUEUE_NAME);
    return NULL;
}
