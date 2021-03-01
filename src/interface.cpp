#include "chess.hpp"
#include <stdio.h>
#include <string.h>

using namespace milkchess;

int main(int argc, char **argv) {
  Game game;

  if (argc > 1 && !strcmp("test", argv[1])) {
    game.test();
    return 0;
  }

  // game loop
  for (;;) {
    // draw board
    printf("\033[2J\033[H  \033[4ma b c d e f g h\033[0m\n");
    for (uint8_t y = 0; y < BOARD_SIZE; y++) {
      printf("%d|", BOARD_SIZE - y);
      for (uint8_t x = 0; x < BOARD_SIZE; x++) {
        Piece *piece = game.board[y][x];
        if (piece) {
          const char *symbols[] = {
              "♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚",
          };
          printf("%s ",
                 symbols[(piece->type - 1) + (piece->color == BLACK ? 0 : 6)]);
        } else {
          printf("%s ", (x + y) % 2 ? "·" : "•");
        }
      }
      printf("\n");
    }
    printf("\n");

    // update status if necessary
    switch (game.get_state()) {
    case Game::BLACK_WIN:
      printf("Black wins!\n");
      return 0;
    case Game::WHITE_WIN:
      printf("White wins!\n");
      return 0;
    case Game::DRAW:
      printf("Draw!\n");
      return 0;
    default:
      break;
    }

    // get recommendations
    std::tuple<Move, int> best_move_tpl = game.suggest_move();
    Move best_move = std::get<0>(best_move_tpl);
    int rating = std::get<1>(best_move_tpl);
    printf("AI suggests: %c%d to %c%d", best_move.x1 + 'a',
           BOARD_SIZE - best_move.y1, best_move.x2 + 'a',
           BOARD_SIZE - best_move.y2);
    if (best_move.promotion_type) {
      const char *promotion_types[] = {"knight", "bishop", "rook", "queen"};
      printf("(%s)", promotion_types[best_move.promotion_type + 1]);
    }
    printf("; rating: %d, current: %d\n", rating, game.rate_state());

    // TEMP AUTO MODE:
    printf("Press any button to continue...");
    getchar();
    game.make_move(best_move);
    std::swap(game.active, game.opponent);
    continue;

    // get move
  get_move:;
    uint8_t x1, y1, x2, y2;
    printf("\033[1m%s's move: \033[0m",
           game.active->color == BLACK ? "Black" : "White");
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

    // check if the move is a pawn promotion
    Piece::Type promotion_type = Piece::NONE;
    if (game.board[y1][x1] && game.board[y1][x1]->type == Piece::PAWN &&
        (y2 == 0 || y2 == 7)) {
    get_promotion:;
      char promotion_input;
      printf("\033[1mEnter pawn promotion type: \033[0m");
      if (scanf(" %c", &promotion_input) != 1) {
        promotion_input = 0;
      }
      switch (promotion_input) {
      case 'k':
        promotion_type = Piece::KNIGHT;
        break;
      case 'b':
        promotion_type = Piece::BISHOP;
        break;
      case 'r':
        promotion_type = Piece::ROOK;
        break;
      case 'q':
        promotion_type = Piece::QUEEN;
        break;
      default:
        printf("Invalid promotion type; must be `k`, `b`, `r`, or `q`\n");
        goto get_promotion;
      }
    }

    // ensure that move is reachable
    Piece *piece = game.board[y1][x1];
    if (!piece || piece->color != game.active->color) {
      printf("Invalid piece selected\n");
      goto get_move;
    }
    Move move(piece, x2, y2, game.board[y2][x2], promotion_type);
    for (Move &pos_move : game.get_moves()) {
      if (move.piece == pos_move.piece && move.x2 == pos_move.x2 &&
          move.y2 == pos_move.y2) {
        goto found;
      }
    }
    printf("Invalid move: invalid target square\n");
    goto get_move;
  found:
    if (!game.make_move(move)) {
      printf("Invalid move: resulted in check\n");
      goto get_move;
    }

    std::swap(game.active, game.opponent);
  }
}