#include "termioslib/termioslib.h"
#include "base/base.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Terminal Functions
term_context *term_create(u32 capacity, mem_arena *arena) {
  term_context *term = PUSH_STRUCT(arena, term_context);

  tcgetattr(STDIN_FILENO, &term->orig_termios);

  struct termios raw = term->orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  term->buf_capacity = capacity;
  term->buf_size = 0;
  term->buf = PUSH_ARRAY(arena, u8, capacity);
  memset(term->buf, 0, capacity);

  string8 to_write =
      STR8_LIT(TERMIOSLIB_ALT_BUFF_ENABLE TERMIOSLIB_ERASE_SCREEN);
  write(STDOUT_FILENO, to_write.str, to_write.size);

  return term;
}

void term_quit(term_context *term) {
  string8 to_write = STR8_LIT(TERMIOSLIB_ALT_BUFF_DISABLE);
  write(STDOUT_FILENO, to_write.str, to_write.size);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &term->orig_termios);
}

u32 term_read(term_context *term, u8 *keys, u32 capacity) {
  return read(STDIN_FILENO, keys, capacity);
}

void term_write(term_context *term, string8 str) {
  while (str.size > 0) {
    u32 to_write = MIN(term->buf_capacity - term->buf_size, str.size);

    memcpy(term->buf + term->buf_size, str.str, to_write);
    term->buf_size += to_write;

    str.str += to_write;
    str.size -= to_write;

    if (term->buf_size >= term->buf_capacity) {
      term_flush(term);
    }
  }
}

void term_write_c(term_context *term, u8 c) {
  term->buf[term->buf_size++] = c;
  if (term->buf_size >= term->buf_capacity) {
    term_flush(term);
  }
}

void term_flush(term_context *term) {
  write(STDOUT_FILENO, term->buf, term->buf_size);
  term->buf_size = 0;
}

// Color Functions
b32 col_eq(win_col a, win_col b) {
  return a.r == b.r && a.g == b.g && a.b == b.b;
}

void set_col(term_context *term, win_col c, b32 fg) {
  u8 chars[21] = {0};
  u32 size = 0;

  if (fg != 0) {
    size = snprintf((char *)chars, sizeof(chars), TERMIOSLIB_SET_FG_RGB, c.r,
                    c.g, c.b);
  } else {
    size = snprintf((char *)chars, sizeof(chars), TERMIOSLIB_SET_BG_RGB, c.r,
                    c.g, c.b);
  }

  term_write(term, (string8){chars, size});
}

// Window Functions
void term_move_to(term_context *term, u32 x, u32 y) {
  u8 chars[64];

  int written =
      snprintf((char *)chars, sizeof(chars), TERMIOSLIB_CURSOR_POS, y, x);

  if (written > 0 && (u32)written < sizeof(chars)) {
    term_write(term, (string8){
                         .str = chars,
                         .size = (u64)written,
                     });
  }
}

term_win win_create(term_context *term, u32 x, u32 y, u32 w, u32 h) {
  return (term_win){
      .term = term,
      .rect = {x, y, w, h},
      .cursor_x = 0,
      .cursor_y = 0,
  };
}

void win_move_to(term_win *win, u32 x, u32 y) {
  if (x >= win->rect.w) {
    x = win->rect.w - 1;
  }

  if (y >= win->rect.h) {
    y = win->rect.h - 1;
  }

  win->cursor_x = x;
  win->cursor_y = y;

  term_move_to(win->term, win->rect.x + x, win->rect.y + y);
}

void win_write(term_win *win, string8 str) {
  if (win->cursor_y >= win->rect.h) {
    return;
  }

  u32 remaining = win->rect.w - win->cursor_x;

  if (remaining == 0) {
    return;
  }

  u64 to_write = str.size;

  if (to_write > remaining) {
    to_write = remaining;
  }

  term_write(win->term, (string8){
                            .str = str.str,
                            .size = to_write,
                        });

  win->cursor_x += to_write;
}

void win_write_line(term_win *win, u32 y, string8 str, win_align align) {
  if (y >= win->rect.h) {
    return;
  }

  u64 size = str.size;

  if (size > win->rect.w) {
    size = win->rect.w;
  }

  u32 x = 0;

  switch (align) {
  case WIN_ALIGN_LEFT:
    x = 0;
    break;
  case WIN_ALIGN_CENTER:
    x = (win->rect.w - size) / 2;
    break;
  case WIN_ALIGN_RIGHT:
    x = win->rect.w - size;
    break;
  }

  win_move_to(win, x, y);

  term_write(win->term, (string8){
                            .str = str.str,
                            .size = size,
                        });
}

void win_clear(term_win *win) {
  for (u32 y = 0; y < win->rect.h; y++) {
    win_move_to(win, 0, y);
    for (u32 x = 0; x < win->rect.w; x++) {
      term_write_c(win->term, ' ');
    }
  }

  win_move_to(win, 0, 0);
}

void win_border(term_win *win) {
  if (win->rect.w < 2 || win->rect.h < 2) {
    return;
  }

  u32 max_x = win->rect.w - 1;
  u32 max_y = win->rect.h - 1;

  win_move_to(win, 0, 0);
  term_write_c(win->term, '+');

  for (u32 x = 1; x < max_x; x++) {
    term_write_c(win->term, '-');
  }

  term_write_c(win->term, '+');

  for (u32 y = 1; y < max_y; y++) {
    win_move_to(win, 0, y);
    term_write_c(win->term, '|');

    win_move_to(win, max_x, y);
    term_write_c(win->term, '|');
  }

  win_move_to(win, 0, max_y);
  term_write_c(win->term, '+');

  for (u32 x = 1; x < max_x; x++) {
    term_write_c(win->term, '-');
  }

  term_write_c(win->term, '+');
}
