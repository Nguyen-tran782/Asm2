/*
 * File: sms_sensor_driver.c
 * Purpose: Kernel Module for Sensor Monitoring System
 * Author: Tran Dinh Khoi Nguyen - SE200558
 */
#include <linux/module.h>
#include <linux/math64.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/random.h>
#include <linux/ktime.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include "sms_sensor.h"

#define PROC_NAME "sms_stats"

static unsigned int read_count = 0;
static unsigned int last_temp_int = 0, last_temp_frac = 0;
static unsigned int last_hum_int = 0, last_hum_frac = 0;
static unsigned int min_interval_ms = 100;
static ktime_t start_time;
static ktime_t last_read_time;
static DEFINE_MUTEX(sensor_mutex);
static struct proc_dir_entry *proc_entry;

/* Hàm tạo số ngẫu nhiên trong khoảng [min, max] */
static unsigned int get_random_range(unsigned int min, unsigned int max) {
    unsigned int rand_val = get_random_u32();
    return min + (rand_val % (max - min + 1));
}

static ssize_t sensor_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    char buf[SMS_BUFFER_SIZE];
    int len;
    ktime_t current_time;
    s64 delta_ms;
    unsigned int temp_raw, hum_raw;

    /* Ngăn tình trạng vòng lặp vô hạn khi đọc (đã học ở Task 4 - Lab 2) */
    if (*ppos > 0) return 0;

    mutex_lock(&sensor_mutex);
    
    current_time = ktime_get();
    /* Kiem tra thoi gian giua 2 lan doc (IOCTL Interval) */
    if (read_count > 0) {
        delta_ms = ktime_ms_delta(current_time, last_read_time);
        if (delta_ms < min_interval_ms) {
            mutex_unlock(&sensor_mutex);
            return -EAGAIN; /* Tra ve loi neu driver chua san sang */
        }
    }

    /* Tao nhiet do: 15.0-45.0 (nhan 10 -> 150-450) | Do am: 30.0-90.0 (300-900) */
    temp_raw = get_random_range(150, 450);
    hum_raw = get_random_range(300, 900);

    last_temp_int = temp_raw / 10;
    last_temp_frac = temp_raw % 10;
    last_hum_int = hum_raw / 10;
    last_hum_frac = hum_raw % 10;

    last_read_time = current_time;
    read_count++;

    /* Dinh dang: <timestamp_ms>,<temperature>,<humidity>\n */
    len = snprintf(buf, SMS_BUFFER_SIZE, "%lld,%u.%u,%u.%u\n",
                   ktime_to_ms(current_time),
                   last_temp_int, last_temp_frac,
                   last_hum_int, last_hum_frac);

    mutex_unlock(&sensor_mutex);

    if (len > count) len = count;
    if (copy_to_user(user_buf, buf, len)) {
        return -EFAULT;
    }

    *ppos += len;
    return len;
}

static long sensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    unsigned int new_interval;
    
    switch (cmd) {
        case SMS_SET_INTERVAL:
            if (copy_from_user(&new_interval, (unsigned int __user *)arg, sizeof(new_interval)))
                return -EFAULT;
            mutex_lock(&sensor_mutex);
            min_interval_ms = new_interval;
            mutex_unlock(&sensor_mutex);
            pr_info("sms_sensor: min_interval_ms duoc dat thanh %u ms\n", min_interval_ms);
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

static const struct file_operations sensor_fops = {
    .owner = THIS_MODULE,
    .read = sensor_read,
    .unlocked_ioctl = sensor_ioctl,
};

static struct miscdevice sensor_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "sms_sensor",
    .fops = &sensor_fops,
};

static int stats_show(struct seq_file *m, void *v) {
    s64 uptime_s;
    
    mutex_lock(&sensor_mutex);
    uptime_s = div_s64(ktime_ms_delta(ktime_get(), start_time), 1000);
    
    seq_printf(m, "--- SMS Sensor Stats ---\n");
    seq_printf(m, "Read Count      : %u\n", read_count);
    seq_printf(m, "Last Temperature: %u.%u C\n", last_temp_int, last_temp_frac);
    seq_printf(m, "Last Humidity   : %u.%u %%\n", last_hum_int, last_hum_frac);
    seq_printf(m, "Driver Uptime   : %lld seconds\n", uptime_s);
    mutex_unlock(&sensor_mutex);
    
    return 0;
}

static int stats_open(struct inode *inode, struct file *file) {
    return single_open(file, stats_show, NULL);
}

static const struct proc_ops stats_proc_ops = {
    .proc_open = stats_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init sms_sensor_init(void) {
    int ret;

    ret = misc_register(&sensor_misc);
    if (ret < 0) {
        pr_err("sms_sensor: Loi dang ky misc device\n");
        return ret;
    }

    proc_entry = proc_create(PROC_NAME, 0444, NULL, &stats_proc_ops);
    if (!proc_entry) {
        misc_deregister(&sensor_misc);
        pr_err("sms_sensor: Loi tao proc entry\n");
        return -ENOMEM;
    }

    start_time = ktime_get();
    last_read_time = start_time;
    pr_info("sms_sensor: Module nap thanh cong. Major=10, Minor=%d\n", sensor_misc.minor);
    return 0;
}

static void __exit sms_sensor_exit(void) {
    proc_remove(proc_entry);
    misc_deregister(&sensor_misc);
    pr_info("sms_sensor: Module da duoc go bo\n");
}

module_init(sms_sensor_init);
module_exit(sms_sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tran Dinh Khoi Nguyen - SE200558");
MODULE_DESCRIPTION("Sensor Monitoring System Driver");
