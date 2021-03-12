#include "chess.hpp"
#pragma once

namespace ai {

struct MoveChoice {
  chess::Move move;
  int current_rating, target_rating;
};

// find the best move given the current state for a given player
MoveChoice best_move(chess::Game &game, chess::Player *player);

} // namespace ai
