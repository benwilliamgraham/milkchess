#include "chess.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc > 1 && !strcmp("test", argv[1])) {
    test();
    return 0;
  }

  init_game();

  /* game loop */
  for (;;) {
    /* draw board */
    printf("\033[2J\033[H  \033[4ma b c d e f g h\033[0m\n");
    uint8_t x, y;
    for (y = 0; y < BOARD_SIZE; y++) {
      printf("%d|", BOARD_SIZE - y);
      for (x = 0; x < BOARD_SIZE; x++) {
        Piece *piece = board[y][x];
        if (piece) {
          const char *symbols[] = {
              "♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚",
          };
          printf("%s ", symbols[piece->type + (piece->color == BLACK ? 0 : 6)]);
        } else {
          printf("%s ", (x + y) % 2 ? "·" : "•");
        }
      }
      printf("\n");
    }
    printf("\n");

    /* get recommendations */
    uint8_t x1, y1, x2, y2;
    PieceType promotion_type;
    int rating = best_move(&x1, &y1, &x2, &y2, &promotion_type, 5);
    x1 += 'a';
    y1 = BOARD_SIZE - y1;
    x2 += 'a';
    y2 = BOARD_SIZE - y2;
    printf("AI suggests: %c%d to %c%d", x1, y1, x2, y2);
    if (promotion_type) {
      const char *promotion_types[] = {"knight", "bishop", "rook", "queen"};
      printf("(%s)", promotion_types[promotion_type + 1]);
    }
    printf("; rating: %d\n", rating);

    /* get move */
  get_move:;
    printf("\033[1m%s's move: \033[0m",
           move_color == BLACK ? "Black" : "White");
    if (scanf(" %c%c to %c%c", &x1, &y1, &x2, &y2) != 4) {
      printf("\rInvalid move; expected form `xy to xy`\n");
      goto get_move;
    }
    x1 -= 'a';
    y1 = BOARD_SIZE - (y1 - '1') - 1;
    x2 -= 'a';
    y2 = BOARD_SIZE - (y2 - '1') - 1;
    if (x1 > BOARD_SIZE || y1 > BOARD_SIZE || x2 > BOARD_SIZE ||
        y2 > BOARD_SIZE) {
      printf("Invalid square, must be between `a1` and `h8`\n");
      goto get_move;
    }

    /* check if the move is a pawn promotion */
    promotion_type = PAWN;
    if (board[y1][x1] && board[y1][x1]->type == PAWN && (y2 == 0 || y2 == 7)) {
    get_promotion:;
      char promotion_input;
      printf("\033[1mEnter pawn promotion type: \033[0m");
      if (scanf(" %c", &promotion_input) != 1) {
        promotion_input = 0;
      }
      switch (promotion_input) {
      case 'k':
        promotion_type = KNIGHT;
        break;
      case 'b':
        promotion_type = BISHOP;
        break;
      case 'r':
        promotion_type = ROOK;
        break;
      case 'q':
        promotion_type = QUEEN;
        break;
      default:
        printf("Invalid promotion type; must be `k`, `b`, `r`, or `q`\n");
        goto get_promotion;
      }
    }

    /* attempt to make move */
    switch (make_move(x1, y1, x2, y2, promotion_type)) {
    case INVALID_PIECE:
      printf("Invalid piece selected\n");
      goto get_move;
    case INVALID_TARGET:
      printf("Invalid target selected\n");
      goto get_move;
    case IN_CHECK:
      printf("King is in check, preventing move\n");
      goto get_move;
    default:
      break;
    }

    /* update status if necessary */
    switch (get_state()) {
    case BLACK_WIN:
      printf("Black wins!\n");
      return 0;
    case WHITE_WIN:
      printf("White wins!\n");
      return 0;
    case DRAW:
      printf("Draw!\n");
      return 0;
    default:
      break;
    }
  }
}
