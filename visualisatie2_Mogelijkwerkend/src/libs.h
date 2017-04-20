#ifndef _LIBS_H_
#define _LIBS_H_

#define DEBUG

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <chrono>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <libusb-1.0/libusb.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) \
          do { fprintf(stderr, ##__VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#else
#define DEBUG_PRINT(...) ;
#endif

#define ERROR_PRINT(...) \
        do { fprintf(stderr, ##__VA_ARGS__); fprintf(stderr, "\n"); } while (0)

#define PRINT(...) \
        do { fprintf(stderr, ##__VA_ARGS__); fprintf(stderr, "\n"); } while (0)

#define PI 3.14159265359

#endif // _LIBS_H_
