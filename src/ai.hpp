#include "chess.hpp"
#pragma once

namespace ai {

// find the best move given the current state for a given player
chess::Move best_move(chess::Game &game, chess::Player *player);

} // namespace ai
