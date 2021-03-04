#include "chess.hpp"
#include <chrono>
#include <cstdlib>
#include <limits.h>
#include <list>
#include <stdio.h>
#include <stdlib.h>

using namespace milkchess;

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
  active = &white;
  opponent = &black;
}

struct _Delta {
  int8_t x, y;
} const _KNIGHT_DELTAS[] =
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
  for (_Delta delta : _KNIGHT_DELTAS) {
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
  for (_Delta delta : _KING_DELTAS) {
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
  for (_Delta delta : _HORZ_VERT_DETLAS) {
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
  for (_Delta delta : _DIAGONAL_DELTAS) {
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

std::vector<Move> Game::get_moves() {
  std::vector<Move> good_moves;
  std::vector<Move> bad_moves;
  for (Piece &piece : active->pieces) {
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
        bad_moves.push_back(Move(&piece, x, y));
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
          bad_moves.push_back(Move(&piece, x, y, nullptr, promotion_type));
        }
        // piece taking
        x = piece.x - 1;
        if (x < BOARD_SIZE && board[y][x] &&
            board[y][x]->color == opponent->color) {
          good_moves.push_back(Move(&piece, x, y, board[y][x], promotion_type));
        }
        x = piece.x + 1;
        if (x < BOARD_SIZE && board[y][x] &&
            board[y][x]->color == opponent->color) {
          good_moves.push_back(Move(&piece, x, y, board[y][x], promotion_type));
        }
      }
      // en passant
      if (last_pawn_adv2 && last_pawn_adv2->y == piece.y &&
          std::abs((int)last_pawn_adv2->x - piece.x) == 1) {
        good_moves.push_back(
            Move(&piece, last_pawn_adv2->x, y, last_pawn_adv2));
      }
    }
    // knight movement
    else if (piece.type == Piece::KNIGHT) {
      for (_Delta delta : _KNIGHT_DELTAS) {
        uint8_t x = piece.x + delta.x;
        uint8_t y = piece.y + delta.y;
        if (x < BOARD_SIZE && y < BOARD_SIZE) {
          if (!board[y][x]) {
            bad_moves.push_back(Move(&piece, x, y));
          } else if (board[y][x]->color == opponent->color) {
            good_moves.push_back(Move(&piece, x, y, board[y][x]));
          }
        }
      }
    }
    // king movement
    else if (piece.type == Piece::KING) {
      for (_Delta delta : _KING_DELTAS) {
        uint8_t x = piece.x + delta.x;
        uint8_t y = piece.y + delta.y;
        if (x < BOARD_SIZE && y < BOARD_SIZE) {
          if (!board[y][x]) {
            bad_moves.push_back(Move(&piece, x, y));
          } else if (board[y][x]->color == opponent->color) {
            good_moves.push_back(Move(&piece, x, y, board[y][x]));
          }
        }
      }
      // check for castling
      if (!piece.has_moved && !is_check(active)) {
        uint8_t y = piece.y;
        if (board[y][0] && !board[y][0]->has_moved && !board[y][1] &&
            !board[y][2] && !board[y][3]) {
          good_moves.push_back(Move(&piece, 2, y));
        }
        if (board[y][7] && !board[y][7]->has_moved && !board[y][6] &&
            !board[y][5]) {
          good_moves.push_back(Move(&piece, 6, y));
        }
      }
    } else {
      // horizontal/vertical sliding
      if (piece.type == Piece::ROOK || piece.type == Piece::QUEEN) {
        for (_Delta delta : _HORZ_VERT_DETLAS) {
          for (uint8_t d = 1;; d++) {
            uint8_t x = piece.x + d * delta.x;
            uint8_t y = piece.y + d * delta.y;
            if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
              break;
            }
            Piece *target = board[y][x];
            if (target) {
              if (target->color == opponent->color) {
                good_moves.push_back(Move(&piece, x, y, target));
              }
              break;
            }
            bad_moves.push_back(Move(&piece, x, y));
          }
        }
      }
      // diagonal sliding
      if (piece.type == Piece::BISHOP || piece.type == Piece::QUEEN) {
        for (_Delta delta : _DIAGONAL_DELTAS) {
          for (uint8_t d = 1;; d++) {
            uint8_t x = piece.x + d * delta.x;
            uint8_t y = piece.y + d * delta.y;
            if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
              break;
            }
            Piece *target = board[y][x];
            if (target) {
              if (target->color == opponent->color) {
                good_moves.push_back(Move(&piece, x, y, target));
              }
              break;
            }
            bad_moves.push_back(Move(&piece, x, y));
          }
        }
      }
    }
  }
  // combine moves and return
  good_moves.insert(good_moves.end(), bad_moves.begin(), bad_moves.end());
  return good_moves;
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
  if (is_check(active)) {
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

Game::State Game::get_state() {
  for (Move &move : get_moves()) {
    if (make_move(move)) {
      undo_move(move);
      return IN_PLAY;
    }
    undo_move(move);
  }
  if (is_check(active)) {
    return active->color == BLACK ? WHITE_WIN : BLACK_WIN;
  } else {
    return DRAW;
  }
}

int Game::rate_state(Player *player) {
  int rating = 0, multiplier = 1;
  Player *rating_player = player;
  Player *other_player = player == active ? opponent : active;
weigh_player:
  // weigh peices
  for (Piece &piece : rating_player->pieces) {
    if (piece.is_live) {
      // add weight
      int piece_ratings[] = {1, 3, 3, 5, 9, 0};
      int piece_rating = piece_ratings[piece.type - 1];
      rating += multiplier * piece_rating * 4;
      // weigh proximity to king
      uint8_t dist = abs((int)other_player->king->x - piece.x) +
                     abs((int)other_player->king->y - piece.y);
      rating += multiplier * ((16 - dist) >> 2) * piece_rating;
    }
  }
  // swap to other side and rate again
  if (rating_player == player) {
    std::swap(rating_player, other_player);
    multiplier = -1;
    goto weigh_player;
  }
  return rating;
}

// perform aplha-beta pruning to find the best move
int _alpha_beta(Game &game, unsigned depth, int a, int b, Player *maximizing,
                bool last_capture = false) {
  // if is a terminal node
  if (depth == 0) {
    return game.rate_state(maximizing);
  }
  // attempt to search, testing for end states if not successful
  bool move_found = false;
  int rating;
  if (game.active == maximizing) {
    rating = -INT_MAX;
    for (Move &move : game.get_moves()) {
      if (game.make_move(move)) {
        move_found = true;
        std::swap(game.active, game.opponent);
        rating = std::max(rating, _alpha_beta(game, depth - 1, a, b, maximizing,
                                              move.captured));
        std::swap(game.active, game.opponent);
        a = std::max(a, rating);
        game.undo_move(move);
        if (a >= b) {
          break;
        }
      }
    }
    return rating;
  } else {
    rating = INT_MAX;
    for (Move &move : game.get_moves()) {
      if (game.make_move(move)) {
        move_found = true;
        std::swap(game.active, game.opponent);
        rating = std::min(rating, _alpha_beta(game, depth - 1, a, b, maximizing,
                                              move.captured));
        std::swap(game.active, game.opponent);
        b = std::min(b, rating);
        game.undo_move(move);
        if (b <= a) {
          break;
        }
      }
    }
  }
  // check for draw
  if (!move_found && !game.is_check(game.active)) {
    return 0;
  }
  return rating;
}

// given two rated moves, return the best
struct RatedMove {
  Move &move;
  int rating;
};

bool _best_rated_move(RatedMove a, RatedMove b) { return a.rating > b.rating; }

Suggestion Game::suggest_move(unsigned time_ms) {
  // initially start with no rating
  std::vector<Move> moves = get_moves();
  std::list<RatedMove> rated_moves;
  for (Move &move : moves) {
    if (make_move(move)) {
      rated_moves.push_back({move, 0});
      undo_move(move);
    }
  }
  // start timer
  auto start = std::chrono::high_resolution_clock::now();
  // continue iterating until timer is out
  unsigned depth = 2;
  for (;; depth++) {
    int current_best = -INT_MAX;
    for (RatedMove &rated_move : rated_moves) {
      make_move(rated_move.move);
      std::swap(active, opponent);
      int rating = _alpha_beta(*this, depth, current_best, INT_MAX, opponent);
      rated_move.rating = rating;
      current_best = std::max(current_best, rating);
      std::swap(active, opponent);
      undo_move(rated_move.move);
      if (std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::high_resolution_clock::now() - start)
              .count() > time_ms) {
        goto time_up;
      }
    }
    // sort moves by quality and either return best or start over
    rated_moves.sort(_best_rated_move);
  }
time_up:
  rated_moves.sort(_best_rated_move);
  return {
      .move = rated_moves.front().move,
      .rating = rated_moves.front().rating,
      .depth = depth,
  };
}

// get the number of positions at a given depth, for testing purposes
unsigned _get_poses(Game &game, uint8_t depth) {
  unsigned num_moves = 0;
  for (Move &move : game.get_moves()) {
    if (game.make_move(move)) {
      if (depth > 1) {
        std::swap(game.active, game.opponent);
        num_moves += _get_poses(game, depth - 1);
        std::swap(game.active, game.opponent);
      } else {
        num_moves++;
      }
      game.undo_move(move);
    }
  }
  return num_moves;
}

// test the number of poses
void _test_poses(Game &game, std::vector<unsigned> expected_poses) {
  for (uint8_t i = 0; i < expected_poses.size(); i++) {
    printf("Testing depth %d ...\n", i + 1);
    unsigned poses = _get_poses(game, i + 1);
    if (poses == expected_poses[i]) {
      printf("Correct!\n");
    } else {
      printf("Error: expected %u, got %u\n", expected_poses[i], poses);
    }
  }
}

void Game::test() {
  printf("Testing from start:\n");
  _test_poses(*this, {20, 400, 8902, 197281, 4865609, 119060324});
  printf("Testing from position 5:\n");
  black.pieces[4].x = 5;
  black.pieces[4].has_moved = true;
  board[0][4] = nullptr;
  board[0][5] = &black.pieces[4];

  black.pieces[5].x = 4;
  black.pieces[5].y = 1;
  black.pieces[5].has_moved = true;
  board[1][4] = &black.pieces[5];

  black.pieces[6].x = 5;
  black.pieces[6].y = 6;
  black.pieces[6].has_moved = true;
  board[0][6] = nullptr;
  board[6][5] = &black.pieces[6];

  black.pieces[10].has_moved = true;
  black.pieces[10].y = 2;
  black.pieces[10].has_moved = true;
  board[1][2] = nullptr;
  board[2][2] = &black.pieces[10];

  black.pieces[11].is_live = false;

  white.pieces[5].x = 2;
  white.pieces[5].y = 4;
  white.pieces[5].has_moved = true;
  board[7][5] = nullptr;
  board[4][2] = &white.pieces[5];

  white.pieces[6].x = 4;
  white.pieces[6].y = 6;
  white.pieces[6].has_moved = true;
  board[7][6] = nullptr;
  board[6][4] = &white.pieces[6];

  white.pieces[11].y = 1;
  white.pieces[11].has_moved = true;
  board[6][3] = nullptr;
  board[1][3] = &white.pieces[11];

  white.pieces[12].is_live = false;

  white.pieces[13].is_live = false;

  _test_poses(*this, {44, 1486, 62379, 2103487, 89941194});
}
