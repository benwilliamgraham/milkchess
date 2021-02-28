#ifndef CHESS_H
#define CHESS_H

#include <stdbool.h>
#include <stdint.h>

#define NUM_PIECES_PER_SIDE 16
#define BOARD_SIZE 8

/* team options */
typedef enum { BLACK, WHITE } PieceColor;

/* type options */
typedef enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } PieceType;

/* piece description */
typedef struct {
  PieceColor color;
  PieceType type;
  bool has_moved;
  bool is_live;
  uint8_t x, y;
} Piece;

/* game state */
extern Piece *board[BOARD_SIZE][BOARD_SIZE];

extern Piece black_pieces[NUM_PIECES_PER_SIDE],
    white_pieces[NUM_PIECES_PER_SIDE];

extern PieceColor move_color;

void init_game();

typedef enum { IN_PLAY, BLACK_WIN, WHITE_WIN, DRAW } GameState;

GameState get_state();

/* make a move, assuming game is still in play
 * takes (x1, y1) -> (x2, y2) and uses promotion_type for pawn promotion
 * returns status of move
 */
typedef enum {
  SUCCESS,
  INVALID_PIECE,
  INVALID_TARGET,
  IN_CHECK,
} MoveStatus;

MoveStatus make_move(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                     PieceType promotion_type);

/* find the best move given the current game state
 * assigns (x1, y1) -> (x2, y2) and uses piece_opt for pawn promotion
 */
int best_move(uint8_t *x1, uint8_t *y1, uint8_t *x2, uint8_t *y2,
               PieceType *promotion_type, uint8_t search_depth);

/* test the chess engine */
void test();

#endif /* !CHESS_H */
