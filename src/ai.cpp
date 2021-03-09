#include "ai.hpp"
#include <limits.h>
#include <list>
#include <map>
#include <stdio.h>
#include <stdlib.h>

using namespace chess;
using namespace ai;

// a combination move and rating
struct _RatedMove {
  Move &move;
  int rating;

  static bool best_move(const _RatedMove &lhs, const _RatedMove &rhs) {
    return lhs.rating > rhs.rating;
  }
};

// create a hashed version of the board for set lookup
struct _HashBoard {
  unsigned squares[BOARD_SIZE * BOARD_SIZE / 2];

  _HashBoard(Game &game) {
    for (uint8_t y = 0; y < BOARD_SIZE; y++) {
      for (uint8_t x = 0; x < BOARD_SIZE; x++) {
        Piece *piece = game.board[y][x];
        unsigned value = 0;
        if (piece) {
          value = piece->type | (piece->color == BLACK ? 0b10000000 : 0);
        }
        if (x % 2 == 1) {
          squares[(y * BOARD_SIZE + x) / 2] |= value << 4;
        } else {
          squares[(y * BOARD_SIZE + x) / 2] = value;
        }
      }
    }
  }

  bool operator<(const _HashBoard &rhs) const {
    for (uint8_t i = 0; i < BOARD_SIZE * BOARD_SIZE / 2; i++) {
      if (squares[i] < rhs.squares[i]) {
        return true;
      } else if (squares[i] > rhs.squares[i]) {
        return false;
      }
    }
    return false;
  }

  bool operator==(const _HashBoard &rhs) {
    for (uint8_t i = 0; i < BOARD_SIZE * BOARD_SIZE / 2; i++) {
      if (squares[i] != rhs.squares[i]) {
        return false;
      }
    }
    return true;
  }
};

struct _Entry {
  int rating;
  unsigned depth;
  enum Flag { EXACT, LOWERBOUND, UPPERBOUND } flag;
};

// get the score for a single player
int _rate_player(Game &game, Player *player) {
  int score = 0;
  uint8_t x, y;
  for (Piece &piece : player->pieces) {
    if (piece.is_live) {
      unsigned piece_multipliers[] = {0, 1, 3, 3, 5, 9, 0};
      unsigned square_multipliers[] = {4, 4, 5, 6, 6, 5, 4, 4};
      unsigned piece_multiplier = piece_multipliers[piece.type];
      score += 64 * piece_multiplier * square_multipliers[piece.y] *
               square_multipliers[piece.x];
      if (piece.type == Piece::PAWN) {
        if (piece.x == 0 || piece.x == 7) {
          score +=
              piece_multiplier * square_multipliers[x] * square_multipliers[y];
        } else {
          score += 2 * piece_multiplier * square_multipliers[x] *
                   square_multipliers[y];
        }
      } else if (piece.type == Piece::KNIGHT) {
        for (Delta delta : KNIGHT_DELTAS) {
          x = piece.x + delta.x, y = piece.y + delta.y;
          if (x < BOARD_SIZE && y < BOARD_SIZE) {
            score += piece_multiplier * square_multipliers[x] *
                     square_multipliers[y];
          }
        }
      } else {
        if (piece.type == Piece::BISHOP || piece.type == Piece::QUEEN) {
          for (Delta delta : DIAGONAL_DELTAS) {
            for (unsigned d = 1;; d++) {
              x = piece.x + delta.x * d, y = piece.y + delta.y * d;
              if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
                break;
              }
              score += piece_multiplier * square_multipliers[x] *
                       square_multipliers[y];
              if (game.board[y][x]) {
                break;
              }
            }
          }
        } else if (piece.type == Piece::ROOK || piece.type == Piece::QUEEN) {
          for (Delta delta : HORZ_VERT_DETLAS) {
            for (unsigned d = 1;; d++) {
              x = piece.x + delta.x * d, y = piece.y + delta.y * d;
              if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
                break;
              }
              score += piece_multiplier * square_multipliers[x] *
                       square_multipliers[y];
              if (game.board[y][x]) {
                break;
              }
            }
          }
        }
      }
    }
  }
  return score;
}

int _negamax(Game &game, Player *max, Player *min, unsigned depth, int a, int b,
             int8_t color, std::map<_HashBoard, _Entry> trans_table) {
  int a_orig = a;

  // check if the state has already been reached
  _HashBoard hash_board(game);
  std::map<_HashBoard, _Entry>::iterator entry = trans_table.find(hash_board);
  if (entry != trans_table.end() && entry->second.depth >= depth) {
    switch (entry->second.flag) {
    case _Entry::EXACT:
      return entry->second.rating;
    case _Entry::LOWERBOUND:
      a = std::max(a, entry->second.rating);
      break;
    case _Entry::UPPERBOUND:
      b = std::min(b, entry->second.rating);
      break;
    }
  }

  // if the node is terminal
  if (depth == 0) {
    return (_rate_player(game, max) - _rate_player(game, min)) * color;
  }

  std::vector<Move> moves = game.get_moves(max);
  std::list<_RatedMove> rated_moves;
  for (Move &move : moves) {
    if (game.make_move(move)) {
      rated_moves.push_back(
          {.move = move,
           .rating = _rate_player(game, max) - _rate_player(game, min)});
      game.undo_move(move);
    }
  }
  rated_moves.sort(_RatedMove::best_move);
  int rating = -INT_MAX;
  for (_RatedMove &rated_move : rated_moves) {
    if (game.make_move(rated_move.move)) {
      rating = std::max(rating, -_negamax(game, min, max, depth - 1, -b, -a,
                                          -color, trans_table));
      a = std::max(a, rating);
      game.undo_move(rated_move.move);
      if (a >= b) {
        break;
      }
    }
  }
  // check for draw
  if (!rated_moves.size()) {
    rating = 0;
  }

  _Entry new_entry = {
      .rating = rating,
      .depth = depth,
  };
  if (rating <= a_orig) {
    new_entry.flag = _Entry::UPPERBOUND;
  } else if (rating >= b) {
    new_entry.flag = _Entry::LOWERBOUND;
  } else {
    new_entry.flag = _Entry::EXACT;
  }
  trans_table[hash_board] = new_entry;
  return rating;
}

Move ai::best_move(Game &game, Player *player) {
  Player *max = player;
  Player *min = player == &game.black ? &game.white : &game.black;
  std::vector<Move> moves = game.get_moves(max);
  std::list<_RatedMove> rated_moves;
  for (Move &move : moves) {
    if (game.make_move(move)) {
      rated_moves.push_back(
          {.move = move,
           .rating = _rate_player(game, max) - _rate_player(game, min)});
      game.undo_move(move);
    }
  }
  for (unsigned depth = 4; depth <= 5; depth++) {
    rated_moves.sort(_RatedMove::best_move);
    int rating = -INT_MAX;
    std::map<_HashBoard, _Entry> trans_table;
    for (_RatedMove &rated_move : rated_moves) {
      if (game.make_move(rated_move.move)) {
        int res = -_negamax(game, min, max, depth, -INT_MAX, -rating, -1,
                            trans_table);
        // short-circuit checkmate
        if (res == INT_MAX) {
          return rated_move.move;
        }
        rated_move.rating = res;
        rating = std::max(rating, res);
        game.undo_move(rated_move.move);
      }
    }
  }
  rated_moves.sort(_RatedMove::best_move);
  return rated_moves.front().move;
}
