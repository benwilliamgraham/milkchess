pub enum GameState {
    Playing,
    BlackWin,
    WhiteWin,
}

pub enum Color {
    Black = 0,
    White = 1,
}

pub enum Type {
    Pawn = 0,
    Rook = 1,
    Knight = 2,
    Bishop = 3,
    Queen = 4,
    King = 5,
}

pub struct Piece {
    data: u8,
}

impl Piece {
    // pieces are serialized as u8
    // 0b11111111 for empty
    // 0b000MCTTT where M is if the piece has moved, C is color, T is type
    const EMPTY: Piece = Piece { data: 0b11111111 };

    pub fn new(color: Color, type_: Type, has_moved: bool) -> Piece {
        let mut data = 0;
        data |= color as u8;
        data |= (type_ as u8) << 3;
        data |= (has_moved as u8) << 4;
        Piece { data }
    }
}

pub type Board = [[Piece; 8]; 8];
