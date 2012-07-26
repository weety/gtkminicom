TARGET = gtkminicom
SOURCE = gtkminicom.c window.c window.h serialconfigdialog.c serialconfigdialog.h serial.c serial.h icon.h logfile.c logfile.h trayicon.c trayicon.h list.h
OBJS   = gtkminicom.o window.o serialconfigdialog.o serial.o logfile.o trayicon.o

CFLAGS=-Wall -g `pkg-config --cflags gtk+-2.0 vte gthread-2.0`
LDFLAGS=`pkg-config --libs gtk+-2.0 vte gthread-2.0`
CC=gcc

all:$(TARGET)

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o a.out $(TARGET) core *~
