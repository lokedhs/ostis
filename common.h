#ifndef COMMON_H
#define COMMON_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "screen.h"

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t LONG;

typedef int8_t SBYTE;
typedef int16_t SWORD;
typedef int32_t SLONG;

#define SIZE_B 0
#define SIZE_W 1
#define SIZE_L 2
#define MASK_B 0xff
#define MASK_W 0xffff
#define MASK_L 0xffffffff

#include "shifter.h"

#define ENTER do { if(cpu->debug) printf("----\nDEBUG: Entering: %s()\n----\n\n", __func__); } while(0);
#define EXIT do { if(cpu->debug) printf("DEBUG: Exiting: %s()\n", __func__); } while(0);
#define ENDLOOP do { shifter_build_image(1); screen_swap(); } while(1);

#define MAX(x, y) (((x)>(y))?(x):(y))
#define MIN(x, y) (((x)<(y))?(x):(y))

#endif
