#include "chess.hpp"
#include <stdio.h>
#include <stdlib.h>

using namespace chess;

Game::Game() {
  // initialize game
  black.color = BLACK;
  white.color = WHITE;
  const Piece::Type piece_order[] = {
      Piece::ROOK, Piece::KNIGHT, Piece::BISHOP, Piece::QUEEN,
      Piece::KING, Piece::BISHOP, Piece::KNIGHT, Piece::ROOK, /* first row */
      Piece::PAWN, Piece::PAWN,   Piece::PAWN,   Piece::PAWN,
      Piece::PAWN, Piece::PAWN,   Piece::PAWN,   Piece::PAWN /* second row */
  };
  for (uint32_t i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
    board[i / BOARD_SIZE][i % BOARD_SIZE] = nullptr;
  }
  for (uint8_t i = 0; i < sizeof(piece_order) / sizeof(Piece::Type); ++i) {
    uint8_t x = i % BOARD_SIZE, black_y = i / BOARD_SIZE,
            white_y = BOARD_SIZE - i / BOARD_SIZE - 1;
    black.pieces[i] = {
        .color = BLACK,
        .type = piece_order[i],
        .x = x,
        .y = black_y,
    };
    white.pieces[i] = {
        .color = WHITE,
        .type = piece_order[i],
        .x = x,
        .y = white_y,
    };
    if (piece_order[i] == Piece::KING) {
      black.king = &black.pieces[i];
      white.king = &white.pieces[i];
    }
    board[black_y][x] = &black.pieces[i];
    board[white_y][x] = &white.pieces[i];
  }
}

bool Game::is_check(Player *player) {
  // check for pawns
  uint8_t y = player->king->y + (player->color == BLACK ? 1 : -1);
  uint8_t x = player->king->x - 1;
  if (x < BOARD_SIZE && y < BOARD_SIZE) {
    Piece *attacker = board[y][x];
    if (attacker && attacker->color != player->color &&
        attacker->type == Piece::PAWN) {
      return true;
    }
  }
  x = player->king->x + 1;
  if (x < BOARD_SIZE && y < BOARD_SIZE) {
    Piece *attacker = board[y][x];
    if (attacker && attacker->color != player->color &&
        attacker->type == Piece::PAWN) {
      return true;
    }
  }
  // check for knights
  for (Delta delta : KNIGHT_DELTAS) {
    x = player->king->x + delta.x;
    y = player->king->y + delta.y;
    if (x < BOARD_SIZE && y < BOARD_SIZE) {
      Piece *attacker = board[y][x];
      if (attacker && attacker->color != player->color &&
          attacker->type == Piece::KNIGHT) {
        return true;
      }
    }
  }
  // check for other king
  for (Delta delta : KING_DELTAS) {
    x = player->king->x + delta.x;
    y = player->king->y + delta.y;
    if (x < BOARD_SIZE && y < BOARD_SIZE) {
      Piece *attacker = board[y][x];
      if (attacker && attacker->color != player->color &&
          attacker->type == Piece::KING) {
        return true;
      }
    }
  }
  // check for horizontal/vertical pieces
  for (Delta delta : HORZ_VERT_DETLAS) {
    for (uint8_t d = 1;; d++) {
      x = player->king->x + d * delta.x;
      y = player->king->y + d * delta.y;
      if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
        break;
      }
      Piece *attacker = board[y][x];
      if (attacker) {
        if (attacker->color != player->color &&
            (attacker->type == Piece::ROOK || attacker->type == Piece::QUEEN)) {
          return true;
        }
        break;
      }
    }
  }
  for (Delta delta : DIAGONAL_DELTAS) {
    for (uint8_t d = 1;; d++) {
      x = player->king->x + d * delta.x;
      y = player->king->y + d * delta.y;
      if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
        break;
      }
      Piece *attacker = board[y][x];
      if (attacker) {
        if (attacker->color != player->color &&
            (attacker->type == Piece::BISHOP ||
             attacker->type == Piece::QUEEN)) {
          return true;
        }
        break;
      }
    }
  }
  return false;
}

std::vector<Move> Game::get_moves(Player *player) {
  std::vector<Move> moves;
  for (Piece &piece : player->pieces) {
    if (!piece.is_live) {
      continue;
    }
    // pawn movement
    if (piece.type == Piece::PAWN) {
      int8_t dir = piece.color == BLACK ? 1 : -1;
      // double move forward
      uint8_t x = piece.x;
      uint8_t intermediate = piece.y + dir;
      uint8_t y = piece.y + 2 * dir;
      if (!piece.has_moved && !board[intermediate][x] && !board[y][x]) {
        moves.push_back(Move(&piece, x, y));
      }
      // check each for pawn promotion
      y = piece.y + dir;
      std::vector<Piece::Type> promotion_options = {Piece::NONE};
      if (y == 0 || y == 7) {
        promotion_options = {Piece::KNIGHT, Piece::BISHOP, Piece::ROOK,
                             Piece::QUEEN};
      }
      for (Piece::Type promotion_type : promotion_options) {
        x = piece.x;
        // standard move forward
        if (!board[y][x]) {
          moves.push_back(Move(&piece, x, y, nullptr, promotion_type));
        }
        // piece taking
        x = piece.x - 1;
        if (x < BOARD_SIZE && board[y][x] &&
            board[y][x]->color != player->color) {
          moves.push_back(Move(&piece, x, y, board[y][x], promotion_type));
        }
        x = piece.x + 1;
        if (x < BOARD_SIZE && board[y][x] &&
            board[y][x]->color != player->color) {
          moves.push_back(Move(&piece, x, y, board[y][x], promotion_type));
        }
      }
      // en passant
      if (last_pawn_adv2 && last_pawn_adv2->y == piece.y &&
          std::abs((int)last_pawn_adv2->x - piece.x) == 1) {
        moves.push_back(Move(&piece, last_pawn_adv2->x, y, last_pawn_adv2));
      }
    }
    // knight movement
    else if (piece.type == Piece::KNIGHT) {
      for (Delta delta : KNIGHT_DELTAS) {
        uint8_t x = piece.x + delta.x;
        uint8_t y = piece.y + delta.y;
        if (x < BOARD_SIZE && y < BOARD_SIZE) {
          if (!board[y][x]) {
            moves.push_back(Move(&piece, x, y));
          } else if (board[y][x]->color != player->color) {
            moves.push_back(Move(&piece, x, y, board[y][x]));
          }
        }
      }
    }
    // king movement
    else if (piece.type == Piece::KING) {
      for (Delta delta : KING_DELTAS) {
        uint8_t x = piece.x + delta.x;
        uint8_t y = piece.y + delta.y;
        if (x < BOARD_SIZE && y < BOARD_SIZE) {
          if (!board[y][x]) {
            moves.push_back(Move(&piece, x, y));
          } else if (board[y][x]->color != player->color) {
            moves.push_back(Move(&piece, x, y, board[y][x]));
          }
        }
      }
      // check for castling
      if (!piece.has_moved && !is_check(player)) {
        uint8_t y = piece.y;
        if (board[y][0] && !board[y][0]->has_moved && !board[y][1] &&
            !board[y][2] && !board[y][3]) {
          moves.push_back(Move(&piece, 2, y));
        }
        if (board[y][7] && !board[y][7]->has_moved && !board[y][6] &&
            !board[y][5]) {
          moves.push_back(Move(&piece, 6, y));
        }
      }
    } else {
      // horizontal/vertical sliding
      if (piece.type == Piece::ROOK || piece.type == Piece::QUEEN) {
        for (Delta delta : HORZ_VERT_DETLAS) {
          for (uint8_t d = 1;; d++) {
            uint8_t x = piece.x + d * delta.x;
            uint8_t y = piece.y + d * delta.y;
            if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
              break;
            }
            Piece *target = board[y][x];
            if (target) {
              if (target->color != player->color) {
                moves.push_back(Move(&piece, x, y, target));
              }
              break;
            }
            moves.push_back(Move(&piece, x, y));
          }
        }
      }
      // diagonal sliding
      if (piece.type == Piece::BISHOP || piece.type == Piece::QUEEN) {
        for (Delta delta : DIAGONAL_DELTAS) {
          for (uint8_t d = 1;; d++) {
            uint8_t x = piece.x + d * delta.x;
            uint8_t y = piece.y + d * delta.y;
            if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
              break;
            }
            Piece *target = board[y][x];
            if (target) {
              if (target->color != player->color) {
                moves.push_back(Move(&piece, x, y, target));
              }
              break;
            }
            moves.push_back(Move(&piece, x, y));
          }
        }
      }
    }
  }
  return moves;
}

bool Game::make_move(Move &move) {
  // apply move
  if (move.captured) {
    move.captured->is_live = false;
  }
  board[move.y1][move.x1] = nullptr;
  board[move.y2][move.x2] = move.piece;
  move.piece->x = move.x2;
  move.piece->y = move.y2;
  move.piece->has_moved = true;
  // check for castling
  if (move.piece->type == Piece::KING && !move.had_moved && move.x2 == 2) {
    Piece *rook = board[move.y2][0];
    rook->x = 3;
    rook->has_moved = true;
    board[move.y2][0] = nullptr;
    board[move.y2][3] = rook;
  }
  if (move.piece->type == Piece::KING && !move.had_moved && move.x2 == 6) {
    Piece *rook = board[move.y2][7];
    rook->x = 5;
    rook->has_moved = true;
    board[move.y2][7] = nullptr;
    board[move.y2][5] = rook;
  }
  // check for pawn promotion
  if (move.promotion_type) {
    move.piece->type = move.promotion_type;
  }
  // en passant setup
  move.last_pawn_adv2 = last_pawn_adv2;
  if (move.piece->type == Piece::PAWN &&
      std::abs((int)move.y1 - move.y2) == 2) {
    last_pawn_adv2 = move.piece;
  } else {
    last_pawn_adv2 = nullptr;
  }
  // en passant capture
  if (move.captured && move.y2 != move.captured->y) {
    board[move.captured->y][move.captured->x] = nullptr;
  }
  // make sure player isn't put in check
  if (is_check(move.piece->color == BLACK ? &black : &white)) {
    undo_move(move);
    return false;
  }
  return true;
}

void Game::undo_move(Move &move) {
  // un-apply move
  if (move.captured) {
    move.captured->is_live = true;
  }
  board[move.y1][move.x1] = move.piece;
  if (move.captured) {
    board[move.y2][move.x2] = nullptr;
    board[move.captured->y][move.captured->x] = move.captured;
  } else {
    board[move.y2][move.x2] = nullptr;
  }
  move.piece->x = move.x1;
  move.piece->y = move.y1;
  move.piece->has_moved = move.had_moved;
  // check for castling
  if (move.piece->type == Piece::KING && !move.had_moved && move.x2 == 2) {
    Piece *rook = board[move.y2][3];
    rook->x = 0;
    rook->has_moved = false;
    board[move.y2][3] = nullptr;
    board[move.y2][0] = rook;
  }
  if (move.piece->type == Piece::KING && !move.had_moved && move.x2 == 6) {
    Piece *rook = board[move.y2][5];
    rook->x = 7;
    rook->has_moved = false;
    board[move.y2][5] = nullptr;
    board[move.y2][7] = rook;
  }
  // check for pawn promotion
  if (move.promotion_type) {
    move.piece->type = Piece::PAWN;
  }
  // en passant setup
  last_pawn_adv2 = move.last_pawn_adv2;
}

Game::State Game::get_state(Player *player) {
  for (Move &move : get_moves(player)) {
    if (make_move(move)) {
      undo_move(move);
      return IN_PLAY;
    }
    undo_move(move);
  }
  if (is_check(player)) {
    return LOSS;
  } else {
    return DRAW;
  }
}

// get the number of positions at a given depth, for testing purposes
unsigned _get_poses(Game &game, Player *player, uint8_t depth) {
  unsigned num_moves = 0;
  for (Move &move : game.get_moves(player)) {
    if (game.make_move(move)) {
      if (depth > 1) {
        num_moves += _get_poses(
            game, player == &game.black ? &game.white : &game.black, depth - 1);
      } else {
        num_moves++;
      }
      game.undo_move(move);
    }
  }
  return num_moves;
}

void Game::test() {
  std::vector<unsigned> expected_poses{20,     400,     8902,
                                       197281, 4865609, 119060324};
  for (uint8_t i = 0; i < expected_poses.size(); i++) {
    printf("Testing depth %d ...\n", i + 1);
    unsigned poses = _get_poses(*this, &white, i + 1);
    if (poses == expected_poses[i]) {
      printf("Correct!\n");
    } else {
      printf("Error: expected %u, got %u\n", expected_poses[i], poses);
    }
  }
}
