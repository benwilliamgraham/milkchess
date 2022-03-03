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

#[derive(Clone, Copy)]
pub struct Piece {
    data: u8,
}

// pieces are serialized as u8
// 0b11111111 for empty
// 0b000MCTTT where M is if the piece has moved, C is color, T is type
impl Piece {
    pub const EMPTY: Piece = Piece { data: 0b11111111 };

    pub fn new(color: Color, type_: Type, has_moved: bool) -> Piece {
        let mut data = 0;
        data |= color as u8;
        data |= (type_ as u8) << 3;
        data |= (has_moved as u8) << 4;
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
            0 => Type::Pawn,
            1 => Type::Rook,
            2 => Type::Knight,
            3 => Type::Bishop,
            4 => Type::Queen,
            5 => Type::King,
            _ => panic!("Invalid piece type"),
        }
    }

    pub fn has_moved(&self) -> bool {
        self.data & 0b00010000 != 0
    }
}

pub type Board = [[Piece; 8]; 8];

pub fn get_legal_moves(board: Board, turn: Color) -> Vec<Board> {
    let mut legal_moves = Vec::new();
    legal_moves
}

pub fn get_best_move(board: Board, turn: Color) -> Board {
    board
}

pub fn get_state(board: Board) -> GameState {
    GameState::Playing
}
