#include "termioslib/termioslib.h"

#define WIDTH 80
#define HEIGHT 32

// RGB
/*
#define FG_BORDER (win_col){255, 0, 0}
#define FG_CONTENT (win_col){0, 255, 0}
#define FG_TITLE (win_col){0, 255, 0}
*/

// Hex Code

#define FG_BORDER WIN_COL_HEX(0xff0000)
#define FG_CONTENT WIN_COL_HEX(0x00ff00)
#define FG_TITLE WIN_COL_HEX(0x74c7ec)

// Hex Color String
/*
#define FG_BORDER #ff0000
#define FG_CONTENT #00ff00
#define FG_TITLE #74c7ec
*/

#define TITLE STR8_LIT("Example 1")

int main(void) {
  mem_arena *perm_arena = arena_create(GiB(1), MiB(1));

  term_context *term = term_create(1 << 20, perm_arena);

  term_win main_win = win_create(term, 1, 1, WIDTH, HEIGHT);
  term_win content_win = win_inset(&main_win, 2);

  // Using hex color string example
  /*
win_col color = {0};
*/

  while (1) {
    u8 input = 0;
    term_read(term, &input, 1);

    if (input == 'q') {
      break;
    }

    term_write(term, STR8_LIT(TERMIOSLIB_ERASE_SCREEN TERMIOSLIB_CURSOR_HOME));

    // Using hex color string example
    /*
    if (win_col_from_hex_str(FG_BORDER, &color)) {
      set_col(term, color, 1);
    }
    */

    set_col(term, FG_BORDER, 1);

    win_clear(&main_win);
    win_border(&main_win);

    set_col(term, FG_CONTENT, 1);

    win_write_line(&content_win, 0, TITLE, WIN_ALIGN_CENTER);
    win_write_line(&content_win, 2, STR8_LIT("hello world"), WIN_ALIGN_LEFT);
    win_write_line(&content_win, 3, STR8_LIT("how are you!"), WIN_ALIGN_LEFT);
    win_write_line(&content_win, 4, STR8_LIT("I'm good"), WIN_ALIGN_LEFT);

    term_flush(term);

    usleep(200 * 1000);
  }

  term_flush(term);
  term_quit(term);

  arena_destroy(perm_arena);

  return 0;
}
