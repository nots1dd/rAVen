#ifndef LIBRAVEN_H
#define LIBRAVEN_H

#include <stdio.h>
#include <assert.h>
#include <complex.h>
#include <gtk/gtk.h>
#include <libavformat/avformat.h>
#include <magic.h>
#include <math.h>
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])
#define N             (1 << 13)
#define pi            3.14159265358979323846f

/**************************************************
 * @COLOR PALETTE
 **************************************************/

#define GRUVBOX_BG \
  (Color) { 40, 40, 40, 255 } // #282828
#define GRUVBOX_FG \
  (Color) { 235, 219, 178, 255 } // #ebdbb2
#define GRUVBOX_YELLOW \
  (Color) { 250, 189, 47, 255 } // #fabd2f
#define GRUVBOX_BLUE \
  (Color) { 131, 165, 152, 255 } // #83a598
#define GRUVBOX_GREEN \
  (Color) { 184, 187, 38, 255 } // #b8bb26
#define GRUVBOX_RED \
  (Color) { 251, 73, 52, 255 } // #fb4934
#define GRUVBOX_ORANGE \
  (Color) { 254, 128, 25, 255 } // #fe8019
#define GRUVBOX_AQUA \
  (Color) { 142, 192, 124, 255 } // #8ec07c
#define GRUVBOX_PURPLE \
  (Color) { 211, 134, 155, 255 } // #d3869b

typedef enum
{
  STANDARD,
  PIXEL,
  WAVEFORM,
  STARBURST,
  RADIAL_BARS,
  NUM_MODES = 5
} VisualizationMode;

typedef struct
{
  char  title[128];
  char  artist[128];
  char  album[128];
  float duration; // returns in seconds
} MusicMetadata;

typedef struct 
{
  Music music;
  RenderTexture2D overlay;
  MusicMetadata metadata;
} easC_State;

/*typedef struct*/
/*{*/
/*  float left;*/
/*  float right;*/
/*} Frame;*/


/********************************************************
 * $GLOBAL VARIABLES DECLARATION
 ********************************************************/

/****************************************
 * libraven.h
 * 
 * This header contains function declarations for 
 * dynamic library functions used by the main program.
 ****************************************/

/* Typedef for dynamic function pointers */
typedef void (easC_print_t)(void);
typedef void (easC_init_t)(const char* selected_song);
typedef void* (easC_pre_reload_t)(void);
typedef void (easC_post_reload_t)(void*);
typedef void (easC_update_t)(void);

/**************************************
 *
 * @X_MACROS
 *
 * Used to generate list like structs of code
 *
 * $Source: Wikipedia
 *
 * They are most useful when at least some of the lists cannot be composed by indexing, such as compile time. 
 * They provide reliable maintenance of parallel lists whose corresponding 
 * items must be declared or executed in the same order.
 *
 **************************************/

#define EASC_FUNC_LIST   EASC(easC_init)   EASC(easC_print)   EASC(easC_update)   EASC(easC_pre_reload)   EASC(easC_post_reload)
#endif
