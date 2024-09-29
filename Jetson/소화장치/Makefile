CC = gcc  # 또는 gcc
CFLAGS = -g

OBJS = main.o modules/itoa.o modules/ai.o modules/signalSender.o modules/pwm.o modules/gpio.o modules/gpioModule.o modules/motorCtl.o
TARGET = main.out

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

modules/%.o: modules/%.c
	$(CC) $(CFLAGS) -c $< -o $@

modules:
	$(MAKE) -C modules

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f img/*.txt
	rm -f *.out

