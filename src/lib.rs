use wasm_bindgen::prelude::*;

mod chess;

pub trait Deserialize {
    fn deserialize(s: &str) -> Self;
}

impl Deserialize for chess::Board {
    fn deserialize(s: &str) -> Self {
        let mut chars = s.chars();

        let mut squares = [[chess::Piece::EMPTY; 8]; 8];
        for y in 0..8 {
            for x in 0..8 {
                let c: char = chars.next().unwrap();
                let color = if 'a' <= c && c <= 'z' {
                    chess::Color::Black
                } else {
                    chess::Color::White
                };
                let c = c.to_ascii_lowercase();
                let type_ = match c {
                    'p' => chess::Type::Pawn,
                    'r' => chess::Type::Rook,
                    'n' => chess::Type::Knight,
                    'b' => chess::Type::Bishop,
                    'q' => chess::Type::Queen,
                    'k' => chess::Type::King,
                    _ => panic!("Invalid piece type"),
                };
                squares[y][x] = chess::Piece::new(color, type_);
            }
        }

        let castle_statuses: Vec<bool> = (0..4).map(|_| chars.next().unwrap() == 't').collect();

        let can_en_passant: Option<u8> = if chars.next().unwrap() == 'f' {
            None
        } else {
            Some(chars.next().unwrap() as u8 - '0' as u8)
        };
        let turn: chess::Color = if chars.next().unwrap() == 'b' {
            chess::Color::Black
        } else {
            chess::Color::White
        };
        chess::Board {
            squares,
            can_castle_black_queenside: castle_statuses[0],
            can_castle_black_kingside: castle_statuses[1],
            can_castle_white_queenside: castle_statuses[2],
            can_castle_white_kingside: castle_statuses[3],
            can_en_passant,
            turn,
        }
    }
}

pub trait Serialize {
    fn serialize(&self) -> String;
}

impl Serialize for chess::Board {
    fn serialize(&self) -> String {
        let mut s = String::new();
        for y in 0..8 {
            for x in 0..8 {
                let piece = self.squares[y][x];
                if piece == chess::Piece::EMPTY {
                    s.push('.');
                } else {
                    let c = match piece.type_() {
                        chess::Type::Pawn => 'P',
                        chess::Type::Rook => 'R',
                        chess::Type::Knight => 'N',
                        chess::Type::Bishop => 'B',
                        chess::Type::Queen => 'Q',
                        chess::Type::King => 'K',
                    };
                    let c = if piece.color() == chess::Color::Black {
                        c.to_ascii_lowercase()
                    } else {
                        c
                    };
                    s.push(c);
                }
            }
        }
        s
    }
}

impl Serialize for Vec<chess::Board> {
    fn serialize(&self) -> String {
        self.iter()
            .map(|board| board.serialize())
            .collect::<Vec<String>>()
            .join("\n")
    }
}

impl Serialize for chess::GameState {
    fn serialize(&self) -> String {
        match self {
            chess::GameState::Playing => "Playing".to_string(),
            chess::GameState::Check(chess::Color::Black) => format!("CheckBlack"),
            chess::GameState::Check(chess::Color::White) => format!("CheckWhite"),
            chess::GameState::Checkmate(chess::Color::Black) => format!("CheckmateBlack"),
            chess::GameState::Checkmate(chess::Color::White) => format!("CheckmateWhite"),
            chess::GameState::Stalemate => "Stalemate".to_string(),
        }
    }
}

#[wasm_bindgen]
pub fn get_legal_moves(board: &str) -> String {
    chess::get_legal_moves(chess::Board::deserialize(board)).serialize()
}

#[wasm_bindgen]
pub fn get_best_move(board: &str) -> String {
    chess::get_best_move(chess::Board::deserialize(board)).serialize()
}

#[wasm_bindgen]
pub fn get_state(board: &str) -> String {
    chess::get_state(chess::Board::deserialize(board)).serialize()
}
