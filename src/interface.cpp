#include "ai.hpp"
#include "chess.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace chess;
using namespace ai;

void draw_board(Game &game, Player *player) {
  // draw board
  printf("\033[2J\033[H  \033[4m%s\033[0m\n",
         player->color == BLACK ? "h g f e d c b a" : "a b c d e f g h");
  for (uint8_t y = 0; y < BOARD_SIZE; y++) {
    printf("%d|", player->color == BLACK ? y + 1 : BOARD_SIZE - y);
    for (uint8_t x = 0; x < BOARD_SIZE; x++) {
      Piece *piece =
          game.board[player->color == BLACK ? BOARD_SIZE - y - 1 : y]
                    [player->color == BLACK ? BOARD_SIZE - x - 1 : x];
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
}

int main(int argc, char **argv) {
  Game game;
  Player *player = &game.white;

  if (argc > 1 && !strcmp("test", argv[1])) {
    game.test();
    return 0;
  }

  // get player's color
get_color:
  printf("Enter team (`b` or `w`): ");
  char color_input;
  if (scanf(" %c", &color_input) != 1 ||
      (color_input != 'b' && color_input != 'w')) {
    printf("Invalid color\n");
    goto get_color;
  }
  Player *human = color_input == 'b' ? &game.black : &game.white;
  Player *ai = human == &game.black ? &game.white : &game.black;

  // game loop
  draw_board(game, human);
  for (;;) {

    // update status if necessary
    switch (game.get_state(player)) {
    case Game::LOSS:
      printf("%s wins!\n", player == &game.black ? "White" : "Black");
      return 0;
    case Game::DRAW:
      printf("Draw!\n");
      return 0;
    default:
      break;
    }

    // get recommendations
    if (player != human) {
      MoveChoice move_choice = best_move(game, ai);
      Move move = move_choice.move;
      game.make_move(move);
      draw_board(game, human);
      printf("AI's move: %c%d to %c%d (%d.%d -> %d.%d)\n", 'a' + move.x1,
             BOARD_SIZE - move.y1, 'a' + move.x2, BOARD_SIZE - move.y2,
             move_choice.current_rating / 100, std::abs(move_choice.current_rating % 100),
             move_choice.target_rating / 100, std::abs(move_choice.target_rating % 100));
      player = human;
      continue;
    }

    // get move
  get_move:;
    uint8_t x1, y1, x2, y2;
    printf("\033[1mYour move: \033[0m");
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
    if (!piece || piece->color != human->color) {
      printf("Invalid piece selected\n");
      goto get_move;
    }
    for (Move &move : game.get_moves(human)) {
      if (piece == move.piece && x2 == move.x2 && y2 == move.y2 &&
          promotion_type == move.promotion_type) {
        if (!game.make_move(move)) {
          printf("Invalid move: resulted in check\n");
          goto get_move;
        }
        goto found;
      }
    }
    printf("Invalid move: invalid target square\n");
    goto get_move;
  found:
    // switch turns and start over
    draw_board(game, human);
    player = ai;
  }
}
