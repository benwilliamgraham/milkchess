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
        data |= (color as u8) << 3;
        data |= (type_ as u8) & 0b00000111;
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
            _ => panic!("Invalid piece type: {}", data),
        }
    }
}

struct Move {
    from: (u8, u8),
    to: (u8, u8),
}

struct PawnDoubleMove {
    from: (u8, u8),
    prev_can_en_passant: Option<u8>
}

struct Capture {
    from: (u8, u8),
    to: (u8, u8),
    captured: Piece,
}

struct EnPassant {
    from: (u8, u8),
    to: (u8, u8),
}

struct Castle {
    to: (u8, u8),
}

struct Promotion {
    from: (u8, u8),
    to: (u8, u8),
    promoted_to: Piece,
}

struct PromotionCapture {
    from: (u8, u8),
    to: (u8, u8),
    captured: Piece,
    promoted_to: Piece,
}

pub enum Action {
    Move(Move),
    PawnDoubleMove(PawnDoubleMove),
    Capture(Capture),
    EnPassant(EnPassant),
    Castle(Castle),
    Promotion(Promotion),
    PromotionCapture(PromotionCapture),
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
            Action::Move(Move { from, to }) => {
                let piece = self.squares[from.1 as usize][from.0 as usize];
                self.squares[to.1 as usize][to.0 as usize] = piece;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
            }
            Action::PawnDoubleMove(PawnDoubleMove {
                from,
                prev_can_en_passant: _,
            }) => {
                let piece = self.squares[from.1 as usize][from.0 as usize];
                let target_y = match piece.color() {
                    Color::Black => 3,
                    Color::White => 4,
                };
                self.squares[target_y][from.0 as usize] = piece;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
                self.can_en_passant = Some(from.0);
            }
            Action::Capture(Capture {
                from,
                to,
                captured: _,
            }) => {
                let piece = self.squares[from.1 as usize][from.0 as usize];
                self.squares[to.1 as usize][to.0 as usize] = piece;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
            }
            Action::EnPassant(EnPassant { from, to }) => {
                let piece = self.squares[from.1 as usize][from.0 as usize];
                self.squares[to.1 as usize][to.0 as usize] = piece;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
                self.squares[from.1 as usize][to.0 as usize] = Piece::EMPTY;
            }
            Action::Castle(Castle { to }) => {
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
            Action::Promotion(Promotion { from, to, promoted_to: promoted }) => {
                self.squares[to.1 as usize][to.0 as usize] = *promoted;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
            }
            Action::PromotionCapture(PromotionCapture {
                from,
                to,
                captured: _,
                promoted_to: promoted,
            }) => {
                self.squares[to.1 as usize][to.0 as usize] = *promoted;
                self.squares[from.1 as usize][from.0 as usize] = Piece::EMPTY;
            }
        }
    }

    pub fn undo_action(&mut self, action: &Action) {
        match action {
            Action::Move(Move { from, to }) => {
                let piece = self.squares[to.1 as usize][to.0 as usize];
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = Piece::EMPTY;
            }
            Action::PawnDoubleMove(PawnDoubleMove { from, prev_can_en_passant: prev_status }) => {
                let target_y = match self.squares[from.1 as usize][from.0 as usize].color() {
                    Color::Black => 4,
                    Color::White => 3,
                };
                let piece = self.squares[target_y][from.0 as usize];
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[target_y][from.0 as usize] = Piece::EMPTY;
                self.can_en_passant = *prev_status;
            }
            Action::Capture(Capture { from, to, captured }) => {
                let piece = self.squares[to.1 as usize][to.0 as usize];
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = *captured;
            }
            Action::EnPassant(EnPassant { from, to }) => {
                let piece = self.squares[to.1 as usize][to.0 as usize];
                let captured_color = if piece.color() == Color::Black {
                    Color::White
                } else {
                    Color::Black
                };
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = Piece::EMPTY;
                self.squares[from.1 as usize][to.0 as usize] =
                    Piece::new(captured_color, Type::Pawn);
            }
            Action::Castle(Castle { to }) => {
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
            }
            Action::Promotion(Promotion { from, to, promoted_to: promoted }) => {
                let piece = self.squares[to.1 as usize][to.0 as usize];
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = *promoted;
            }
            Action::PromotionCapture(PromotionCapture {
                from,
                to,
                promoted_to: _,
                captured,
            }) => {
                let piece = self.squares[to.1 as usize][to.0 as usize];
                self.squares[from.1 as usize][from.0 as usize] = piece;
                self.squares[to.1 as usize][to.0 as usize] = *captured;
            }
        }
    }

    pub fn is_check(&self) -> bool {
        false
    }

    pub fn is_checkmate(&self) -> bool {
        false
    }

    pub fn is_stalemate(&self) -> bool {
        false
    }

    pub fn get_possible_actions(&mut self) -> Vec<Action> {
        let mut possible_actions = Vec::new();
        for y in 0..8 {
            for x in 0..8 {
                let piece = self.squares[y][x];
                if piece == Piece::EMPTY || piece.color() != self.turn {
                    continue;
                }

                let add_diagonal_moves = || {};

                let add_horiz_vert_moves = || {};

                match piece.type_() {
                    Type::Pawn => {
                        let has_moved = piece.color() == Color::Black && y != 1
                            || piece.color() == Color::White && y != 6;
                        // check one square forward
                        let forw_1 = if piece.color() == Color::Black {
                            y + 1
                        } else {
                            y - 1
                        };
                        if self.squares[forw_1][x] == Piece::EMPTY {
                            possible_actions.push(Action::Move(Move {
                                from: (x as u8, y as u8),
                                to: (x as u8, forw_1 as u8),
                            }));
                            // check for promotion
                            if forw_1 == 0 || forw_1 == 7 {
                                for promotion_type in
                                    [Type::Queen, Type::Rook, Type::Bishop, Type::Knight]
                                {
                                    possible_actions.push(Action::Promotion(Promotion {
                                        from: (x as u8, y as u8),
                                        to: (x as u8, forw_1 as u8),
                                        promoted_to: Piece::new(piece.color(), promotion_type),
                                    }));
                                }
                            }
                            // check two squares forward
                            let forw_2 = if piece.color() == Color::Black {
                                y + 2
                            } else {
                                y - 2
                            };
                            if !has_moved && self.squares[forw_2][x] == Piece::EMPTY {
                                possible_actions.push(Action::Move(Move {
                                    from: (x as u8, y as u8),
                                    to: (x as u8, forw_2 as u8),
                                }));
                            }
                        }
                        // check diagonal capture + possible promotion
                        let mut add_diagonal_captures = |side: usize| {
                            let target = self.squares[forw_1][side];
                            if target != Piece::EMPTY && target.color() != piece.color() {
                                // normal capture
                                if y != 0 && y != 7 {
                                    possible_actions.push(Action::Capture(Capture {
                                        from: (x as u8, y as u8),
                                        to: (side as u8, forw_1 as u8),
                                        captured: target,
                                    }));
                                }
                                // promotion capture
                                else {
                                    for promotion_type in
                                        [Type::Queen, Type::Rook, Type::Bishop, Type::Knight]
                                    {
                                        possible_actions.push(Action::PromotionCapture(
                                            PromotionCapture {
                                                from: (x as u8, y as u8),
                                                to: (side as u8, forw_1 as u8),
                                                promoted_to: Piece::new(piece.color(), promotion_type),
                                                captured: target,
                                            },
                                        ));
                                    }
                                }
                            }
                        };
                        if x != 0 {
                            add_diagonal_captures(x - 1);
                        }
                        if x != 7 {
                            add_diagonal_captures(x + 1);
                        }

                        // check en passant
                        if (piece.color() == Color::Black && y == 4)
                            || (piece.color() == Color::White && y == 3)
                        {
                            match self.can_en_passant {
                                Some(target_x) => {
                                    if (x != 0 && target_x == x as u8 - 1)
                                        || (target_x == x as u8 + 1)
                                    {
                                        possible_actions.push(Action::EnPassant(EnPassant {
                                            from: (x as u8, y as u8),
                                            to: (target_x as u8, forw_1 as u8),
                                        }));
                                    }
                                }
                                None => {}
                            }
                        }
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
        possible_actions
    }

    pub fn get_legal_actions(&mut self) -> Vec<Action> {
        // filter out moves that would put the king in check
        let mut legal_actions = self.get_possible_actions();
        legal_actions.retain(|action| {
            self.apply_action(&action);
            let is_check = self.is_check();
            self.undo_action(&action);
            !is_check
        });
        legal_actions
    }
}
