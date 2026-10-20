/*
 * File: alert_thread.c
 * Purpose: Luồng nhận tin nhắn báo động qua Message Queue, in ra màn hình và ghi log
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#include "alert_thread.h"
#include "common.h"
#include "logger_thread.h"
#include <stdio.h>
#include <mqueue.h>
#include <unistd.h>

void* alert_thread_func(void* arg) {
	(void)arg;
    mqd_t alarm_mq;
    struct mq_attr attr;
    AlertMsg msg;

    /* Cấu hình POSIX Message Queue cho Alert */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(AlertMsg);
    attr.mq_curmsgs = 0;

    mq_unlink(ALARM_QUEUE_NAME);
    alarm_mq = mq_open(ALARM_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);

    while (is_running) {
        if (mq_receive(alarm_mq, (char*)&msg, sizeof(AlertMsg), NULL) > 0) {
            const char* type_str = "";
            switch (msg.alert_type) {
                case 1: type_str = "TEMP_HIGH"; break;
                case 2: type_str = "HUMID_HIGH"; break;
                case 3: type_str = "TEMP_SPIKE"; break;
            }
            
            /* In ra stderr như yêu cầu đề bài */
            fprintf(stderr, "[ALERT][%lld] TYPE: %s VALUE: %.1f THRESHOLD: %.1f\n",
                    msg.timestamp_ms, type_str, msg.value, msg.threshold);
            
            /* Đồng thời gửi vào Log queue */
            send_log("[ALERT][%lld] TYPE: %s VALUE: %.1f THRESHOLD: %.1f\n",
                     msg.timestamp_ms, type_str, msg.value, msg.threshold);
        } else {
            usleep(100000); /* Ngủ 100ms chờ báo động mới */
        }
    }

    mq_close(alarm_mq);
    mq_unlink(ALARM_QUEUE_NAME);
    return NULL;
}
