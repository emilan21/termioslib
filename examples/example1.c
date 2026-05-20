#include "termioslib/termioslib.h"

#define WIDTH 80
#define HEIGHT 32

#define FG_BORDER (win_col){255, 0, 0}
#define FG_CONTENT (win_col){0, 255, 0}

#define TITLE STR8_LIT("Example 1")

int main(void) {
  mem_arena *perm_arena = arena_create(GiB(1), MiB(1));

  term_context *term = term_create(1 << 20, perm_arena);

  term_win main_win = win_create(term, 1, 1, WIDTH, HEIGHT);

  while (1) {
    u8 input = 0;
    read(STDIN_FILENO, &input, 1);

    if (input == 'q') {
      break;
    }

    term_write(term, STR8_LIT(TERMIOSLIB_ERASE_SCREEN TERMIOSLIB_CURSOR_HOME));

    set_col(term, FG_BORDER, 1);

    win_clear(&main_win);
    win_border(&main_win);

    term_win content = win_create(term, 3, 3, WIDTH - 4, HEIGHT - 4);

    set_col(term, FG_CONTENT, 1);

    win_write_line(&content, 0, TITLE, WIN_ALIGN_CENTER);
    win_write_line(&content, 2, STR8_LIT("hello world"), WIN_ALIGN_LEFT);
    win_write_line(&content, 3, STR8_LIT("how are you!"), WIN_ALIGN_LEFT);
    win_write_line(&content, 4, STR8_LIT("I'm good"), WIN_ALIGN_RIGHT);

    term_flush(term);

    usleep(200 * 1000);
  }

  term_quit(term);

  arena_destroy(perm_arena);

  return 0;
}
