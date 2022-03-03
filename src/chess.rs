use std::convert::TryFrom;

pub enum GameState {
    Playing,
    Check(Color), // the color of the player who is in check
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

pub struct Board {
    pub squares : [[Piece; 8]; 8],
    pub can_castle_black_queenside : bool,
    pub can_castle_black_kingside : bool,
    pub can_castle_white_queenside : bool,
    pub can_castle_white_kingside : bool,
    pub can_en_passant : Option<u8>,
    pub turn : Color,
}

pub fn get_legal_moves(board: Board) -> Vec<Board> {
    let mut legal_moves = Vec::new();
    legal_moves
}

pub fn get_best_move(board: Board) -> Board {
    board
}

pub fn get_state(board: Board) -> GameState {
    GameState::Playing
}
