TARGET = sntp
OBJS = sntp.o

all:$(TARGET)
	echo "done"

$(TARGET) : $(OBJS)
	$(AR) rcs $(APP_LIB) $(OBJS)

clean:
	rm -f $(TARGET) *.o
