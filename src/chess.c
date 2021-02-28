#include "chess.h"
#include <limits.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

/* buffer size constants */
#define _MAX_MOVES 32

/* game state */
Piece *board[BOARD_SIZE][BOARD_SIZE];

Piece black_pieces[NUM_PIECES_PER_SIDE], white_pieces[NUM_PIECES_PER_SIDE];

PieceColor move_color;

Piece *_en_passant_setup;

void init_game() {
  uint8_t i;
  memset(board, 0, sizeof(Piece *) * BOARD_SIZE * BOARD_SIZE);
  for (i = 0; i < NUM_PIECES_PER_SIDE; i++) {
    const PieceType pieceOrder[] = {
        ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK, /* first row */
        PAWN, PAWN,   PAWN,   PAWN,  PAWN, PAWN,   PAWN,   PAWN /* second row */
    };
    uint8_t x = i % BOARD_SIZE, black_y = i / BOARD_SIZE,
            white_y = BOARD_SIZE - i / BOARD_SIZE - 1;
    black_pieces[i] = (Piece){
        .color = BLACK,
        .type = pieceOrder[i],
        .has_moved = false,
        .is_live = true,
        .x = x,
        .y = black_y,
    };
    white_pieces[i] = (Piece){
        .color = WHITE,
        .type = pieceOrder[i],
        .has_moved = false,
        .is_live = true,
        .x = x,
        .y = white_y,
    };
    board[black_y][x] = &black_pieces[i];
    board[white_y][x] = &white_pieces[i];
  }
  move_color = WHITE;
  _en_passant_setup = NULL;
}

/* piece movement options */
typedef struct {
  int8_t x, y;
} _Delta;

const _Delta _KNIGHT_DELTAS[] =
    {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2},
},
             _KING_DELTAS[] =
                 {
                     {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
                     {0, 1}, {0, -1}, {-1, 0}, {1, 0},
},
             _HORZ_VERT_DETLAS[] =
                 {
                     {0, 1},
                     {0, -1},
                     {1, 0},
                     {-1, 0},
},
             _DIAGONAL_DELTAS[] = {
                 {1, 1},
                 {1, -1},
                 {-1, 1},
                 {-1, -1},
};

/* check if a player is in check */
bool _is_check(PieceColor king_color) {
  Piece *king = (king_color == BLACK ? black_pieces : white_pieces) + 4,
        *attacker;
  uint8_t i, x, y, d;

  /* check for pawns */
  y = king->y + (king_color == BLACK ? 1 : -1);
  x = king->x - 1;
  if (x < BOARD_SIZE && y < BOARD_SIZE) {
    Piece *attacker = board[y][x];
    if (attacker && attacker->color != king_color && attacker->type == PAWN) {
      return true;
    }
  }
  x = king->x + 1;
  if (x < BOARD_SIZE && y < BOARD_SIZE) {
    attacker = board[y][x];
    if (attacker && attacker->color != king_color && attacker->type == PAWN) {
      return true;
    }
  }

  /* check for knights */
  for (i = 0; i < sizeof(_KNIGHT_DELTAS) / sizeof(_Delta); i++) {
    x = king->x + _KNIGHT_DELTAS[i].x;
    y = king->y + _KNIGHT_DELTAS[i].y;
    if (x < BOARD_SIZE && y < BOARD_SIZE) {
      attacker = board[y][x];
      if (attacker && attacker->color != king_color &&
          attacker->type == KNIGHT) {
        return true;
      }
    }
  }

  /* check for other king */
  for (i = 0; i < sizeof(_KING_DELTAS) / sizeof(_Delta); i++) {
    x = king->x + _KING_DELTAS[i].x;
    y = king->y + _KING_DELTAS[i].y;
    if (x < BOARD_SIZE && y < BOARD_SIZE) {
      attacker = board[y][x];
      if (attacker && attacker->color != king_color && attacker->type == KING) {
        return true;
      }
    }
  }

  /* check for horizontal/vertical pieces */
  for (i = 0; i < sizeof(_HORZ_VERT_DETLAS) / sizeof(_Delta); i++) {
    for (d = 1;; d++) {
      x = king->x + d * _HORZ_VERT_DETLAS[i].x;
      y = king->y + d * _HORZ_VERT_DETLAS[i].y;
      if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
        break;
      }
      attacker = board[y][x];
      if (attacker) {
        if (attacker->color != king_color &&
            (attacker->type == ROOK || attacker->type == QUEEN)) {
          return true;
        }
        break;
      }
    }
  }

  /* check for diagonal pieces */
  for (i = 0; i < sizeof(_DIAGONAL_DELTAS) / sizeof(_Delta); i++) {
    for (d = 1;; d++) {
      x = king->x + d * _DIAGONAL_DELTAS[i].x;
      y = king->y + d * _DIAGONAL_DELTAS[i].y;
      if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
        break;
      }
      attacker = board[y][x];
      if (attacker) {
        if (attacker->color != king_color &&
            (attacker->type == BISHOP || attacker->type == QUEEN)) {
          return true;
        }
        break;
      }
    }
  }

  return false;
}

/* return reachable squares */
typedef struct {
  uint8_t x, y;
} _Square;

uint8_t _get_reachable(_Square *reachable, Piece *piece) {
  uint8_t num_reachable = 0, x, y, i, d;
  PieceColor active_color = piece->color;

  /* pawn movement */
  if (piece->type == PAWN) {
    int8_t forward = piece->color == BLACK ? 1 : -1;
    /* double move forward */
    x = piece->x;
    i = piece->y + forward;
    y = piece->y + 2 * forward;
    if (!piece->has_moved && !board[i][x] && !board[y][x]) {
      reachable[num_reachable++] = (_Square){x, y};
    }
    /* standard move forward */
    y = piece->y + forward;
    if (!board[y][x]) {
      reachable[num_reachable++] = (_Square){x, y};
    }
    /* piece taking */
    x = piece->x - 1;
    if (x < BOARD_SIZE && board[y][x] && board[y][x]->color != active_color) {
      reachable[num_reachable++] = (_Square){x, y};
    }
    x = piece->x + 1;
    if (x < BOARD_SIZE && board[y][x] && board[y][x]->color != active_color) {
      reachable[num_reachable++] = (_Square){x, y};
    }
    /* en passant */
    /* TODO */
  }
  /* knight movement */
  else if (piece->type == KNIGHT) {
    for (i = 0; i < sizeof(_KNIGHT_DELTAS) / sizeof(_Delta); i++) {
      x = piece->x + _KNIGHT_DELTAS[i].x;
      y = piece->y + _KNIGHT_DELTAS[i].y;
      if (x < BOARD_SIZE && y < BOARD_SIZE &&
          (!board[y][x] || board[y][x]->color != active_color)) {
        reachable[num_reachable++] = (_Square){x, y};
      }
    }
  }
  /* king movement */
  else if (piece->type == KING) {
    for (i = 0; i < sizeof(_KING_DELTAS) / sizeof(_Delta); i++) {
      x = piece->x + _KING_DELTAS[i].x;
      y = piece->y + _KING_DELTAS[i].y;
      if (x < BOARD_SIZE && y < BOARD_SIZE &&
          (!board[y][x] || board[y][x]->color != active_color)) {
        reachable[num_reachable++] = (_Square){x, y};
      }
    }
    /* check for castling */
    if (!piece->has_moved && !_is_check(piece->color)) {
      y = piece->y;
      if (board[y][0] && !board[y][0]->has_moved && !board[y][1] &&
          !board[y][2] && !board[y][3]) {
        reachable[num_reachable++] = (_Square){2, y};
      }
      if (board[y][7] && !board[y][7]->has_moved && !board[y][6] &&
          !board[y][5]) {
        reachable[num_reachable++] = (_Square){6, y};
      }
    }
  } else {
    /* horizontal/vertical sliding  */
    if (piece->type == ROOK || piece->type == QUEEN) {
      for (i = 0; i < sizeof(_HORZ_VERT_DETLAS) / sizeof(_Delta); i++) {
        for (d = 1;; d++) {
          x = piece->x + d * _HORZ_VERT_DETLAS[i].x;
          y = piece->y + d * _HORZ_VERT_DETLAS[i].y;
          if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
            break;
          }
          Piece *target = board[y][x];
          if (target) {
            if (target->color != active_color) {
              reachable[num_reachable++] = (_Square){x, y};
            }
            break;
          }
          reachable[num_reachable++] = (_Square){x, y};
        }
      }
    }
    /* diagonal sliding */
    if (piece->type == BISHOP || piece->type == QUEEN) {
      for (i = 0; i < sizeof(_DIAGONAL_DELTAS) / sizeof(_Delta); i++) {
        for (d = 1;; d++) {
          x = piece->x + d * _DIAGONAL_DELTAS[i].x;
          y = piece->y + d * _DIAGONAL_DELTAS[i].y;
          if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
            break;
          }
          Piece *target = board[y][x];
          if (target) {
            if (target->color != active_color) {
              reachable[num_reachable++] = (_Square){x, y};
            }
            break;
          }
          reachable[num_reachable++] = (_Square){x, y};
        }
      }
    }
  }
  return num_reachable;
}

/* apply a move, retuning the necessary information to undo the move */
typedef struct {
  uint8_t x1, y1;
  Piece *piece, *target, *en_passant_setup;
  bool has_moved;
} _MoveInfo;

_MoveInfo _apply_move(Piece *piece, Piece *target, PieceType promotion_type,
                      uint8_t x2, uint8_t y2) {
  _MoveInfo move = {
      .x1 = piece->x,
      .y1 = piece->y,
      .piece = piece,
      .target = target,
      .en_passant_setup = _en_passant_setup,
      .has_moved = piece->has_moved,
  };

  /* check for capturing */
  if (target) {
    target->is_live = false;
  }

  /* check for en passant setup */
  /* TODO */

  /* check for en passant capture */
  /* TODO */

  /* check for pawn promotion */
  /* TODO */

  /* check for castling */
  if (piece->type == KING && !piece->has_moved && x2 == 2) {
    Piece *rook = board[y2][0];
    rook->x = 3;
    rook->has_moved = true;
    board[y2][3] = rook;
    board[y2][0] = NULL;
  }
  if (piece->type == KING && !piece->has_moved && x2 == 6) {
    Piece *rook = board[y2][7];
    rook->x = 5;
    rook->has_moved = true;
    board[y2][5] = rook;
    board[y2][7] = NULL;
  }

  /* move piece */
  board[piece->y][piece->x] = NULL;
  piece->x = x2;
  piece->y = y2;
  piece->has_moved = true;
  board[y2][x2] = piece;

  return move;
}

/* undo a move */
void _undo_move(_MoveInfo *move) {
  /* un-capture target */
  if (move->target) {
    move->target->is_live = true;
  }

  /* check for castling */
  if (move->piece->type == KING && !move->has_moved && move->piece->x == 2) {
    Piece *rook = board[move->y1][3];
    rook->x = 0;
    rook->has_moved = false;
    board[move->y1][0] = rook;
    board[move->y1][3] = NULL;
  }
  if (move->piece->type == KING && !move->has_moved && move->piece->x == 6) {
    Piece *rook = board[move->y1][5];
    rook->x = 7;
    rook->has_moved = false;
    board[move->y1][7] = rook;
    board[move->y1][5] = NULL;
  }

  /* move piece back */
  board[move->piece->y][move->piece->x] = move->target;
  board[move->y1][move->x1] = move->piece;
  move->piece->x = move->x1;
  move->piece->y = move->y1;
  move->piece->has_moved = move->has_moved;
}

/* update game state */
GameState get_state() {
  uint8_t p, num_reachable, i;
  /* check if color up next has any moves or is in checkmate */
  Piece *active_pieces = move_color == BLACK ? black_pieces : white_pieces;
  for (p = 0; p < NUM_PIECES_PER_SIDE; p++) {
    /* check if there are any moves that don't result in check */
    Piece *piece = &active_pieces[p], *target;
    if (piece->is_live) {
      _Square reachable[_MAX_MOVES];
      num_reachable = _get_reachable(reachable, piece);
      for (i = 0; i < num_reachable; i++) {
        target = board[reachable[i].y][reachable[i].x];
        _MoveInfo move =
            _apply_move(piece, target, PAWN, reachable[i].x, reachable[i].y);
        if (!_is_check(move_color)) {
          _undo_move(&move);
          return IN_PLAY;
        }
        _undo_move(&move);
      }
    }
  }
  if (_is_check(move_color)) {
    return move_color == BLACK ? WHITE_WIN : BLACK_WIN;
  } else {
    return DRAW;
  }
}

MoveStatus make_move(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                     PieceType promotion_type) {
  uint8_t num_reachable, i;

  /* get piece */
  Piece *piece;
  if (!(piece = board[y1][x1]) || piece->color != move_color) {
    return INVALID_PIECE;
  }

  /* get target */
  Piece *target;
  if ((target = board[y2][x2]) && target->color == move_color) {
    return INVALID_TARGET;
  }

  /* ensure that target is reachable */
  _Square reachable[_MAX_MOVES];
  num_reachable = _get_reachable(reachable, piece);
  for (i = 0; i < num_reachable; i++) {
    if (x2 == reachable[i].x && y2 == reachable[i].y) {
      goto found;
    }
  }
  return INVALID_TARGET;
found:;

  /* apply move */
  _MoveInfo move = _apply_move(piece, target, promotion_type, x2, y2);

  /* ensure that move doesn't put/leave king in check */
  if (_is_check(move_color)) {
    _undo_move(&move);
    return IN_CHECK;
  }

  /* update state */
  move_color = move_color == BLACK ? WHITE : BLACK;

  return SUCCESS;
}

/* give a rating to a state, with positive being in favor of color */
int _rate_state(PieceColor color) {
  int rating = 0, multiplier;
  uint8_t i;
  PieceColor active_color = BLACK;
  Piece *active_pieces = black_pieces;
  Piece *target_pieces = white_pieces;

  /* apply for both colors */
weigh_color:;
  multiplier = color == active_color ? 1 : -1;
  for (i = 0; i < NUM_PIECES_PER_SIDE; i++) {
    Piece *piece = &active_pieces[i];
    if (piece->is_live) {
      /* weigh piece value */
      int piece_ratings[] = {100, 300, 300, 500, 900};
      rating += multiplier * piece_ratings[piece->type];
      /* weigh distance to enemy king */
      uint8_t dist = abs((int)target_pieces[4].x - piece->x) +
                     abs((int)target_pieces[4].y - piece->y);
      rating += multiplier * (16 - dist);
    }
  }

  /* weigh check */
  if (_is_check(active_color)) {
    rating -= 90;
  }

  /* weigh king position */
  if (target_pieces[4].x < 2 || target_pieces[4].x > 5) {
    rating += 40;
  }
  if (target_pieces[4].y < 2 || target_pieces[4].y > 5) {
    rating += 40;
  }

  /* loop back */
  if (active_color == BLACK) {
    active_color = WHITE;
    active_pieces = white_pieces;
    target_pieces = black_pieces;
    goto weigh_color;
  }

  return rating;
}

int best_move(uint8_t *x1, uint8_t *y1, uint8_t *x2, uint8_t *y2,
               PieceType *promotion_type, uint8_t search_depth) {
  Piece *active_pieces = move_color == BLACK ? black_pieces : white_pieces;
  uint8_t p, num_reachable, i;
  int rating, best_rating = INT_MIN;
  *promotion_type = PAWN;

  /* check all pieces, finding best one to move */
  for (p = 0; p < NUM_PIECES_PER_SIDE; p++) {
    Piece *piece = &active_pieces[p], *target;
    if (piece->is_live) {
      _Square reachable[_MAX_MOVES];
      num_reachable = _get_reachable(reachable, piece);
      for (i = 0; i < num_reachable; i++) {
        target = board[reachable[i].y][reachable[i].x];
        uint8_t x = piece->x, y = piece->y;
        _MoveInfo move =
            _apply_move(piece, target, PAWN, reachable[i].x, reachable[i].y);
        if (!_is_check(move_color)) {
          /* move results in a checkmate */
          move_color = move_color == BLACK ? WHITE : BLACK;
          GameState state = get_state();
          move_color = move_color == BLACK ? WHITE : BLACK;
          if (state == (move_color == BLACK ? BLACK_WIN : WHITE_WIN)) {
            rating = INT_MAX;
          } 
          /* move results in a draw */
          else if (state == DRAW) {
            rating = 0;
          } 
          /* keep recursing */
          else if (search_depth > 1) {
            uint8_t a;
            PieceType p;
            move_color = move_color == BLACK ? WHITE : BLACK;
            rating = -best_move(&a, &a, &a, &a, &p, search_depth - 1);
            move_color = move_color == BLACK ? WHITE : BLACK;
          } 
          /* rate position */
          else {
            rating = _rate_state(move_color);
          }
          if (rating > best_rating) {
            best_rating = rating;
            *x1 = x;
            *y1 = y;
            *x2 = reachable[i].x;
            *y2 = reachable[i].y;
          }
        }
        _undo_move(&move);
      }
    }
  }
  return best_rating;
}

/* draw current game state, for testing purposes */
void _draw_state() {
  uint8_t x, y;
  for (y = 0; y < BOARD_SIZE; y++) {
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
}

/* get the number of positions at a given depth, for testing purposes */
unsigned _get_poses(uint8_t depth) {
  unsigned num_moves = 0;
  uint8_t p, num_reachable, i;
  Piece *active_pieces = move_color == BLACK ? black_pieces : white_pieces;
  for (p = 0; p < NUM_PIECES_PER_SIDE; p++) {
    Piece *piece = &active_pieces[p], *target;
    if (piece->is_live) {
      _Square reachable[_MAX_MOVES];
      num_reachable = _get_reachable(reachable, piece);
      for (i = 0; i < num_reachable; i++) {
        target = board[reachable[i].y][reachable[i].x];
        _MoveInfo move =
            _apply_move(piece, target, PAWN, reachable[i].x, reachable[i].y);
        if (!_is_check(move_color)) {
          GameState state = get_state();
          if (state == IN_PLAY && depth > 1) {
            move_color = move_color == BLACK ? WHITE : BLACK;
            num_moves += _get_poses(depth - 1);
            move_color = move_color == BLACK ? WHITE : BLACK;
          } else {
            num_moves++;
          }
        }
        _undo_move(&move);
      }
    }
  }
  return num_moves;
}

/* test the number of poses */
void _test_poses(const unsigned expected_poses[], uint8_t num_depths) {
  uint8_t i;
  for (i = 0; i < num_depths; i++) {
    printf("Testing depth %d ...\n", i + 1);
    unsigned poses = _get_poses(i + 1);
    if (poses == expected_poses[i]) {
      printf("Correct!\n");
    } else {
      printf("Error: expected %u, got %u\n", expected_poses[i], poses);
    }
  }
}

void test() {
  /* set to starting state and test */
  init_game();
  printf("Testing from start:\n");
  const unsigned start_poses[] = {20, 400, 8902, 197281, 4865906, 119060324};
  _test_poses(start_poses, 6);

  /* set to position 5 and try again */
  black_pieces[4].x = 5;
  black_pieces[4].has_moved = true;
  board[0][4] = NULL;
  board[0][5] = &black_pieces[4];

  black_pieces[5].x = 4;
  black_pieces[5].y = 1;
  black_pieces[5].has_moved = true;
  board[1][4] = &black_pieces[5];

  black_pieces[6].x = 5;
  black_pieces[6].y = 6;
  black_pieces[6].has_moved = true;
  board[0][6] = NULL;
  board[6][5] = &black_pieces[6];

  black_pieces[10].has_moved = true;
  black_pieces[10].y = 2;
  black_pieces[10].has_moved = true;
  board[1][2] = NULL;
  board[2][2] = &black_pieces[10];

  black_pieces[11].is_live = false;

  white_pieces[5].x = 2;
  white_pieces[5].y = 4;
  white_pieces[5].has_moved = true;
  board[7][5] = NULL;
  board[4][2] = &white_pieces[5];

  white_pieces[6].x = 4;
  white_pieces[6].y = 6;
  white_pieces[6].has_moved = true;
  board[7][6] = NULL;
  board[6][4] = &white_pieces[6];

  white_pieces[11].y = 1;
  white_pieces[11].has_moved = true;
  board[6][3] = NULL;
  board[1][3] = &white_pieces[11];

  white_pieces[12].is_live = false;

  white_pieces[13].is_live = false;

  printf("Testing from position 5:\n");
  const unsigned pos_5_poses[] = {44, 1486, 62379, 2103487, 89941194};
  _test_poses(pos_5_poses, 5);
}
