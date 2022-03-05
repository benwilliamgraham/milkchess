use wasm_bindgen::prelude::*;

extern crate console_error_panic_hook;
use std::panic;

mod ai;
mod chess;

trait Deserialize {
    fn deserialize(s: &str) -> Self;
}

impl Deserialize for chess::Board {
    fn deserialize(s: &str) -> Self {
        panic::set_hook(Box::new(console_error_panic_hook::hook));
        let mut chars = s.chars();

        let mut squares = [[chess::Piece::EMPTY; 8]; 8];
        for y in 0..8 {
            for x in 0..8 {
                let c = chars.next().unwrap();
                if c == '.' {
                    continue;
                }
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
                    _ => panic!("Invalid piece type: {}", c),
                };
                squares[y][x] = chess::Piece::new(color, type_);
            }
        }

        let castle_statuses: Vec<bool> = (0..4).map(|_| chars.next().unwrap() == 't').collect();

        let can_en_passant_char = chars.next().unwrap();
        let can_en_passant: Option<u8> = if can_en_passant_char == 'f' {
            None
        } else {
            Some(can_en_passant_char as u8 - '0' as u8)
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

trait Serialize {
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
        for &castle in &[
            self.can_castle_black_queenside,
            self.can_castle_black_kingside,
            self.can_castle_white_queenside,
            self.can_castle_white_kingside,
        ] {
            if castle {
                s.push('t');
            } else {
                s.push('f');
            }
        }
        if let Some(can_en_passant) = self.can_en_passant {
            s.push((can_en_passant + '0' as u8) as char);
        } else {
            s.push('f');
        }
        if self.turn == chess::Color::Black {
            s.push('b');
        } else {
            s.push('w');
        }
        s
    }
}

#[wasm_bindgen]
pub fn get_legal_actions(board: &str) -> String {
    let mut board = chess::Board::deserialize(board);
    board.serialize()
    // board.get_legal_actions()
    //     .iter()
    //     .map(|action| {
    //         board.apply_action(action);
    //         let s = board.serialize();
    //         board.undo_action(action);
    //         s
    //     })
    //     .collect::<Vec<String>>()
    //     .join("\n")
}

#[wasm_bindgen]
pub fn get_best_move(board: &str) -> String {
    let mut board = chess::Board::deserialize(board);
    let action = ai::get_best_move(&mut board);
    board.apply_action(&action);
    let s = board.serialize();
    board.undo_action(&action);
    s
}

#[wasm_bindgen]
pub fn get_state(board: &str) -> String {
    "Hye".to_string()
}
