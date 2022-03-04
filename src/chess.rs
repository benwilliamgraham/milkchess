use std::convert::TryFrom;

pub enum GameState {
    Playing,
    Check(Color),     // the color of the player who is in check
    Checkmate(Color), // the color of the player who was checkmated
    Stalemate,
}

#[derive(PartialEq)]
pub enum Color {
    Black = 0,
    White = 1,
}

pub enum Type {
    Pawn = 1,
    Rook = 2,
    Knight = 3,
    Bishop = 4,
    Queen = 5,
    King = 6,
}

#[derive(Clone, Copy, PartialEq)]
pub struct Piece {
    data: u8,
}

// pieces are serialized as u8
// 0 for empty
// 0bCTTT where C is color and T is type
impl Piece {
    pub const EMPTY: Piece = Piece { data: 0 };

    pub fn new(color: Color, type_: Type) -> Piece {
        let mut data = 0;
        data |= color as u8;
        data |= (type_ as u8) << 3;
        Piece { data }
    }

    pub fn color(&self) -> Color {
        if self.data & 0b00001000 == 0 {
            Color::Black
        } else {
            Color::White
        }
    }

    pub fn type_(&self) -> Type {
        let data = self.data & 0b00000111;
        match data {
            data if data == Type::Pawn as u8 => Type::Pawn,
            data if data == Type::Rook as u8 => Type::Rook,
            data if data == Type::Knight as u8 => Type::Knight,
            data if data == Type::Bishop as u8 => Type::Bishop,
            data if data == Type::Queen as u8 => Type::Queen,
            data if data == Type::King as u8 => Type::King,
            _ => panic!("Invalid piece type"),
        }
    }
}

struct Move {
    from: (u8, u8),
    to: (u8, u8),
}

struct Capture {
    from: (u8, u8),
    to: (u8, u8),
    captured: Piece,
}

struct EnPassant {
    from: (u8, u8),
    to: (u8, u8),
    captured: (u8, u8),
}

struct Castle {
    to: (u8, u8),
}

struct Promotion {
    from: (u8, u8),
    to: (u8, u8),
    promoted: Piece,
}

pub enum Action {
    Move(Move),
    Capture(Capture),
    EnPassant(EnPassant),
    Castle(Castle),
    Promotion(Promotion),
}

pub struct Board {
    pub squares: [[Piece; 8]; 8],
    pub can_castle_black_queenside: bool,
    pub can_castle_black_kingside: bool,
    pub can_castle_white_queenside: bool,
    pub can_castle_white_kingside: bool,
    pub can_en_passant: Option<u8>,
    pub turn: Color,
}

impl Board {
    pub fn apply_action(&mut self, action: &Action) {
        match action {
            Action::Move(move_) => {
                let from = move_.from;
                let to = move_.to;
                let piece = self.squares[from.1 as usize][from.0 as usize];
                self.squares[to.1 as usize][to.0 as usize] = piece;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
            }
            Action::Capture(capture) => {
                let from = capture.from;
                let to = capture.to;
                let piece = self.squares[from.1 as usize][from.0 as usize];
                self.squares[to.1 as usize][to.0 as usize] = piece;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
            }
            Action::EnPassant(en_passant) => {
                let from = en_passant.from;
                let to = en_passant.to;
                let captured = en_passant.captured;
                let piece = self.squares[from.1 as usize][from.0 as usize];
                self.squares[to.1 as usize][to.0 as usize] = piece;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
                self.squares[captured.1 as usize][captured.0 as usize] = Piece::EMPTY;
            }
            Action::Castle(castle) => {
                let to = castle.to;
                let (king_from, rook_from, rook_to) = match to {
                    (2, 0) => ((4, 0), (0, 0), (3, 0)),
                    (6, 0) => ((4, 0), (7, 0), (5, 0)),
                    (2, 7) => ((4, 7), (0, 7), (3, 7)),
                    (6, 7) => ((4, 7), (7, 7), (5, 7)),
                    _ => panic!("Invalid castle action"),
                };
                let king = self.squares[to.1 as usize][to.0 as usize];
                let rook = self.squares[rook_to.1 as usize][rook_to.0 as usize];
                self.squares[to.1 as usize][to.0 as usize] = king;
                self.squares[king_from.1 as usize][king_from.0 as usize] = Piece::EMPTY;
                self.squares[rook_to.1 as usize][rook_to.0 as usize] = rook;
                self.squares[rook_from.1 as usize][rook_from.0 as usize] = Piece::EMPTY;
            }
            Action::Promotion(promotion) => {
                let from = promotion.from;
                let to = promotion.to;
                let promoted = promotion.promoted;
                let piece = self.squares[from.1 as usize][from.0 as usize];
                self.squares[to.1 as usize][to.0 as usize] = promoted;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
            }
        }
    }

    pub fn undo_action(&mut self, action: &Action) {
        match action {
            Action::Move(move_) => {
                let from = move_.from;
                let to = move_.to;
                let piece = self.squares[to.1 as usize][to.0 as usize];
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = Piece::EMPTY;
            }
            Action::Capture(capture) => {
                let from = capture.from;
                let to = capture.to;
                let captured = capture.captured;
                let piece = self.squares[to.1 as usize][to.0 as usize];
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = captured;
            }
            Action::EnPassant(en_passant) => {
                let from = en_passant.from;
                let to = en_passant.to;
                let captured = en_passant.captured;
                let piece = self.squares[to.1 as usize][to.0 as usize];
                let captured_color = if piece.color() == Color::Black {
                    Color::White
                } else {
                    Color::Black
                };
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = Piece::EMPTY;
                self.squares[captured.1 as usize][captured.0 as usize] =
                    Piece::new(captured_color, Type::Pawn);
            }
            Action::Castle(castle) => {
                let to = castle.to;
                let (king_from, rook_from, rook_to) = match to {
                    (2, 0) => ((4, 0), (0, 0), (3, 0)),
                    (6, 0) => ((4, 0), (7, 0), (5, 0)),
                    (2, 7) => ((4, 7), (0, 7), (3, 7)),
                    (6, 7) => ((4, 7), (7, 7), (5, 7)),
                    _ => panic!("Invalid castle action"),
                };
                let king = self.squares[to.1 as usize][to.0 as usize];
                let rook = self.squares[rook_to.1 as usize][rook_to.0 as usize];
                self.squares[king_from.1 as usize][king_from.0 as usize] = king;
                self.squares[to.1 as usize][to.0 as usize] = Piece::EMPTY;
                self.squares[rook_from.1 as usize][rook_from.0 as usize] = rook;
                self.squares[rook_to.1 as usize][rook_to.0 as usize] = Piece::EMPTY;
            },
            Action::Promotion(promotion) => {
                let from = promotion.from;
                let to = promotion.to;
                let promoted = promotion.promoted;
                let piece = self.squares[to.1 as usize][to.0 as usize];
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = promoted;
            }
        }
    }

    fn is_check(&self) -> bool {
        false
    }

    fn is_checkmate(&self) -> bool {
        false
    }
}

pub fn get_legal_actions(board: &Board) -> Vec<Action> {
    let mut legal_moves = Vec::new();
    for y in 0..8 {
        for x in 0..8 {
            let piece = board.squares[y][x];
            if piece == Piece::EMPTY || piece.color() != board.turn {
                continue;
            }

            fn add_diagonal_moves() {}

            fn add_horiz_vert_moves() {}

            match piece.type_() {
                Type::Pawn => {
                    let dir: i8 = if piece.color() == Color::Black { 1 } else { -1 };
                }
                Type::Knight => {}
                Type::Bishop => {
                    add_diagonal_moves();
                }
                Type::Rook => {
                    add_horiz_vert_moves();
                }
                Type::Queen => {
                    add_diagonal_moves();
                    add_horiz_vert_moves();
                }
                Type::King => {}
            }
        }
    }
    legal_moves
}

pub fn get_best_move(board: &Board) -> Action {
    Action::Move(Move {
        from: (0, 0),
        to: (0, 0),
    })
}

pub fn get_state(board: &Board) -> GameState {
    GameState::Playing
}
