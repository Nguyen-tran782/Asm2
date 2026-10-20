obj-m += sms_sensor_driver.o
ccflags-y += -march=armv7-a

# Đường dẫn tới source code Kernel mô phỏng ARM
KDIR = /home/nguyen-tran/embedded_lab1/kernel/linux-5.15
CROSS = arm-linux-gnueabihf-

all:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KDIR) M=$(PWD) modules

clean:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KDIR) M=$(PWD) clean
