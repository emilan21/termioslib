#ifndef TERMIOSLIB_TERMIOSLIB_H
#define TERMIOSLIB_TERMIOSLIB_H

#include <arena/arena.h>
#include <base/base.h>

#include <termios.h>
#include <unistd.h>

// Cursor Controls
#define TERMIOSLIB_CURSOR_HOME "\x1b[H"
#define TERMIOSLIB_CURSOR_MOVE_UP_NUM_LINES "\x1b[#A"
#define TERMIOSLIB_CURSOR_MOVE_DOWN_NUM_LINES "\x1b[#B"
#define TERMIOSLIB_CURSOR_MOVE_RIGHT_NUM_COL "\x1b[#C"
#define TERMIOSLIB_CURSOR_MOVE_LEFT_NUM_COL "\x1b[#D"
#define TERMIOSLIB_CURSOR_NEXT_LINE "\x1b[1E"
#define TERMIOSLIB_CURSOR_PREV_LINE "\x1b[#F"
#define TERMIOSLIB_CURSOR_MOVE_COL "\x1b[#G"

// Erase Controls
#define TERMIOSLIB_ERASE_SCREEN "\x1b[2J"

// Colors / Graphics Mode
#define TERMIOSLIB_RESET "\x1b[0m"

// Common Private Modes
#define TERMIOSLIB_ALT_BUFF_ENABLE "\x1b[?1049h"
#define TERMIOSLIB_ALT_BUFF_DISABLE "\x1b[?1049l"
#define TERMIOSLIB_HIDE_CURSOR "\x1b[?25l"
#define TERMIOSLIB_SHOW_CURSOR "\x1b[?25h"

// RGB Colors
#define TERMIOSLIB_SET_FG_RGB "\x1b[38;2;%d;%d;%dm"
#define TERMIOSLIB_SET_BG_RGB "\x1b[48;2;%d;%d;%dm"

typedef struct {
  struct termios orig_termios;

  u32 buf_capacity;
  u32 buf_size;
  u8 *buf;
} term_context;

typedef struct {
  u8 r, g, b;
} win_col;

typedef struct {
  u8 c;
  win_col fg_col;
  win_col bg_col;
} win_tile;

typedef struct {
  u32 width, height;
  win_tile *data;
} win_buf;

term_context *term_create(u32 capacity, mem_arena *arena);
void term_quit(term_context *term);
u32 term_read(term_context *term, u8 *keys, u32 capacity);
void term_write(term_context *term, string8 str);
void term_write_c(term_context *term, u8 c);
void term_flush(term_context *term);

b32 col_eq(win_col a, win_col b);
void set_col(term_context *term, win_col c, b32 fg);

#endif
