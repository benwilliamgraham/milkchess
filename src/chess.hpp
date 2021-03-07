#include <stdint.h>
#include <tuple>
#include <vector>

namespace milkchess {

#define BOARD_SIZE 8
#define NUM_PIECES_PER_SIDE 16

class Game;

enum Color { BLACK, WHITE };

struct Piece {
  Color color;
  enum Type { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } type;
  uint8_t x, y;
  bool is_live = true;
  bool has_moved = false;
};

struct Move {
  uint8_t x1, y1, x2, y2;
  bool had_moved;
  Piece *piece;
  Piece *captured;
  Piece::Type promotion_type;
  Piece *last_pawn_adv2;
  Move(Piece *piece, uint8_t x, uint8_t y, Piece *captured = nullptr,
       Piece::Type promotion_type = Piece::NONE)
      : x1(piece->x), y1(piece->y), x2(x), y2(y), had_moved(piece->has_moved),
        piece(piece), captured(captured), promotion_type(promotion_type) {}
};

struct Player {
  Color color;
  std::vector<Piece> pieces;
  Piece *king;
  Player() : pieces(NUM_PIECES_PER_SIDE) {}
};

struct Suggestion {
  Move move;
  double rating;
  unsigned depth;
};

class Game {
public:
  enum State { IN_PLAY, LOSS, DRAW };
  Piece *board[BOARD_SIZE][BOARD_SIZE], *last_pawn_adv2 = nullptr;
  Player black, white;
  Game();
  // determines if the piece can be taken in a move
  bool is_check(Player *player);
  // get all possible moves for the active player, ordered best->worst
  std::vector<Move> get_moves(Player *player);
  // apply a move, returning true if successful
  bool make_move(Move &move);
  // undo a move
  void undo_move(Move &move);
  // check the state of the game for a given player
  State get_state(Player *player);
  // suggest the best move along with it's rating
  Suggestion suggest_move(Player *player, unsigned time_ms);
  // test the chess engine
  void test();
};

} // namespace milkchess
