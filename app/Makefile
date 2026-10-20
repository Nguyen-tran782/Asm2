CC_ARM = arm-linux-gnueabihf-gcc
CC_X86 = gcc
CFLAGS = -Wall -Wextra -pthread -I../driver
LDFLAGS = -lrt -static

SRCS = circular_buffer.c logger_thread.c alert_thread.c sensor_processor.c sms_app.c
TARGET_ARM = sms_app
TARGET_X86 = sms_app_x86

all: arm x86

arm:
	$(CC_ARM) $(CFLAGS) $(SRCS) -o $(TARGET_ARM) $(LDFLAGS)

x86:
	$(CC_X86) $(CFLAGS) $(SRCS) -o $(TARGET_X86) $(LDFLAGS)

clean:
	rm -f $(TARGET_ARM) $(TARGET_X86)
