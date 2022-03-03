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

pub trait ToString {
    fn to_string(&self) -> String;
}

#[wasm_bindgen]
pub fn get_legal_moves(board: &str, turn: &str) -> String {
    "Moves".into()
}

#[wasm_bindgen]
pub fn get_best_move(board: &str, turn: &str) -> String {
    "Best Move".into()
}

#[wasm_bindgen]
pub fn get_state(board: &str) -> String {
    "Playing".into()
}
