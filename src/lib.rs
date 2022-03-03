use wasm_bindgen::prelude::*;

mod chess;

pub trait FromString {
    fn from_string(s: &str) -> Self;
}

impl FromString for chess::Color {
    fn from_string(s: &str) -> Self {
        match s {
            "white" => chess::Color::White,
            "black" => chess::Color::Black,
            _ => panic!("Invalid color"),
        }
    }
}

impl FromString for chess::Board {
    fn from_string(s: &str) -> Self {
        let mut board = [[chess::Piece::EMPTY; 8]; 8];
        board
    }
}

pub trait ToString {
    fn to_string(&self) -> String;
}

impl ToString for chess::Board {
    fn to_string (&self) -> String {
        let mut s = String::new();
        s
    }
}

impl ToString for Vec<chess::Board> {
    fn to_string(&self) -> String {
        let mut s = String::new();
        s
    }
}

impl ToString for chess::GameState {
    fn to_string(&self) -> String {
        let mut s = String::new();
        s
    }
}

#[wasm_bindgen]
pub fn get_legal_moves(board: &str, turn: &str) -> String {
    chess::get_legal_moves(
        chess::Board::from_string(board),
        chess::Color::from_string(turn),
    )
    .to_string()
}

#[wasm_bindgen]
pub fn get_best_move(board: &str, turn: &str) -> String {
    chess::get_best_move(
        chess::Board::from_string(board),
        chess::Color::from_string(turn),
    )
    .to_string()
}

#[wasm_bindgen]
pub fn get_state(board: &str) -> String {
    chess::get_state(chess::Board::from_string(board)).to_string()
}
