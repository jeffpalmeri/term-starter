#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

/*
4 rows x 4 columns
[x x x x
 x x x x
 x x x x
 x x x x]

I can make position 0 index based now to make it easy.
Then, only when moving the terminal cursor to my position do
I need to take the 1-offset into consideration. Everything
else is normal.
*/

typedef struct Color {
  int r;
  int g;
  int b;
} Color;

typedef struct Player {
  int pos_x;
  int pos_y;
  int rows;
  int cols;
} Player;

void PutPixel(int x, int y, Color color) {
}

struct termios pt;
void restore_term() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &pt);
}

void set_player_pos(Player *player, int y, int x) {
  player->pos_x = x;
  player->pos_y = y;
}
void jkhl(char c) {
  printf("\033[1A");
}
void clear() {
  printf("\033[H\033[J");
}
void moveHome(Player *player) {
  printf("\033[H");
  set_player_pos(player,0,0);
}
void hideCursor() {
  printf("\033[?25l");
}
void showCursor() {
  printf("\033[?25h");
}
void moveSomewhere() {
  printf("\033[33;136H"); // row column
}

void move_row_column(Player *player, int row, int column) {
  printf("\033[%d;%dH", player->pos_y, player->pos_x); // row column
  set_player_pos(player, player->pos_y, player->pos_x);
}

void match_player_pos(Player *player) {
  printf("\033[%d;%dH", player->pos_y+1, player->pos_x+1); // row column
}

void put_char_then_move_back(Player *player, int row, int column, char c) {
  printf("\033[%d;%dH%c", row, column, c); // row column
  printf("\033[%d;%dH", player->pos_y, player->pos_x); // row column
}

void put_elsewhere_on_screen(int row, int column, char c) {
  printf("\033[%d;%dH%c", row+1, column+1, c); // row column
}

void drawScreen(
  int row, int column,
  void (*move_row_column)(Player *player, int row, int column),
  int data[row][column],
  Player *player
) {
  for(int y = 0; y < row; y++) {
    for(int x = 0; x < column; x++) {
      if(data[y][x] == 1) {
        put_elsewhere_on_screen(y, x ,'x');
      } else if (data[y][x] == 0) {
        put_elsewhere_on_screen(y, x ,' ');
      }
    }
  }
  // move_row_column(player, player->pos_y, player->pos_x);
}

void move(char c, int amount, Player *player) {
  switch(c) {
    case 'j':
      if(player->pos_y + 1 < player->rows) {
        // printf("\033[%dB", amount); // down
        player->pos_y += 1;
      }
      break;
    case 'k':
      if(player->pos_y - 1 >= 0) {
        // printf("\033[%dA", amount); // up
        player->pos_y -= 1;
      }
      break;
    case 'h':
      if(player->pos_x - 1 >= 0) {
        // printf("\033[%dD", amount); // left
        player->pos_x -= 1;
      }
      break;
    case 'l':
      if(player->pos_x + 1 < player->cols) {
        // printf("\033[%dC", amount); // right
        player->pos_x += 1;
      }
      break;
  }
}

void put_char(Player *player, char c) {
  printf("%c", c);
  if(player->pos_x + 1 < player->cols) player->pos_x++;



  // if(player->pos_x + 1 == player->cols + 1) {
  //   // Moving the player would go out of bounds of data, so just print the character
  //   // and don't update the player position
  //   printf("%c", c);
  // } else  if(player->pos_x + 1 <= player->cols) {
  //   // Inbounds so do both, print the char and bump the player position
  //   printf("%c", c);
  //   player->pos_x += 1;
  // }
}

void raw_mode(struct termios t) {
  pt = t;

  t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
                      /* Noncanonical mode, disable signals, extended
                         input processing, and echoing */

  t.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
                    INPCK | ISTRIP | IXON | PARMRK);
                      /* Disable special handling of CR, NL, and BREAK.
                         No 8th-bit stripping or parity error handling.
                         Disable START/STOP output flow control. */

  t.c_oflag &= ~OPOST;                /* Disable all output processing */

  t.c_cc[VMIN] = 1;                   /* Character-at-a-time input */
  t.c_cc[VTIME] = 0;                  /* with blocking */

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t) == -1) {
    printf("tcsetattr error\n");
      return;
  }
}

void updateData(Player *player, int data[player->rows][player->cols]) {
  data[player->pos_y][player->pos_x] = 0;
}

int main() {
  atexit(restore_term);
  Player player = {0,0,0,0};
  struct termios t;
  if(tcgetattr(STDIN_FILENO, &t) == -1) {
    printf("tcgetattr error\n");
  }
  raw_mode(t);



  printf("\033[31mx\033[0m\r\n");
  struct winsize ws;
  if(ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) {
    printf("ioctl errorrrrr\n");
  };
  printf("ws_row: %i\r\n", ws.ws_row);
  printf("ws_col: %i\r\n", ws.ws_col);
  player.rows = ws.ws_row;
  player.cols = ws.ws_col;

  int data[player.rows][player.cols];

  clear();
  for(int y = 0; y < player.rows; y++) {
    for(int x = 0; x < player.cols; x++) {
      if((y+x) % 15 == 0) {
        data[y][x] = 1;
      } else {
        data[y][x] = 0;
      }
    }
  }
  moveHome(&player);
  fflush(stdout);
  char c;

  // fd_set rfd;
  // char buf[256];
  drawScreen(player.rows, player.cols, move_row_column, data, &player);
  match_player_pos(&player);
  fflush(stdout);
  for(;;) {
    // FD_ZERO(&rfd);
    // FD_SET(STDIN_FILENO, &rfd);
    //
    // if (pselect(STDIN_FILENO+1, &rfd, NULL, NULL, NULL, NULL) < 0) {
    //   printf("something bad happened\n");
    // }
    //
    ssize_t n = read(STDIN_FILENO, &c, 1);
    // if(FD_ISSET(STDIN_FILENO, &rfd)) {
    //   
    //   ssize_t numRead = read(STDIN_FILENO, buf, 256);
    //   printf("got something %zd\n", numRead);
    // }
    if(n == 0) break;


    if(data[player.pos_y][player.pos_x] == 1) {
      data[player.pos_y][player.pos_x] = 0;
    }

    if(c == 'q')                                     break;
    hideCursor();
    if(c == 'f')                                     clear();
    if(c == 'j' || c == 'k' || c == 'h' || c == 'l') {
      move(c, 1, &player);
      updateData(&player, data);
    }
    // if(c == 'y') hideCursor();
    // if(c == 'z') showCursor();
    // if(c == 'u') drawScreen(player.rows, player.cols, move_row_column, data, &player);

    if(c == 'a') {
      data[player.pos_y][player.pos_x] = 2;
      put_char(&player, 'x');
    }

    drawScreen(player.rows, player.cols, move_row_column, data, &player);
    // Final thing every loop before flushing: move the cursor to whatever my internal player state is
    match_player_pos(&player);
    showCursor();
    fflush(stdout);
  };

  clear();
  return 0;
}

