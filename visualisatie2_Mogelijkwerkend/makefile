CC := g++

OBJDIR := obj
SRCDIR := src

TARGET := SGPiApp

SRCS := main.cc \
        usb_device.cc \
        usb.cc \
        utils.cc \
				scan_data_frame.cc \
				save_data.cc \
				scan_data_pack.cc \
				data_streamer.cc \
				rs232.cc \
				sg_pi_app.cc

HDRS := timer.h \
        libs.h \
				usb_device.h \
			  usb.h \
			  utils.h \
			  global_var.h \
				scan_data_frame.h \
				scan_data_pack.h \
				save_data.h \
				data_streamer.h \
				rs232.h \
				sg_pi_app.h
OBJS := $(patsubst %.cc, %.o, $(SRCS))

SRCS := $(patsubst %, $(SRCDIR)/%, $(SRCS))
HDRS := $(patsubst %, $(SRCDIR)/%, $(HDRS))
OBJS := $(patsubst %, $(OBJDIR)/%, $(OBJS))

CFLAGS := -Wall -g -std=gnu++11
LFLAGS := -L'/usr/lib/x86_64-linux-gnu' \
					-lusb-1.0 \
					-lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cc $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: clean
clean:
	rm $(TARGET) $(OBJS)
