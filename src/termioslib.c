#include "termioslib/termioslib.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
