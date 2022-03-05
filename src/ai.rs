use crate::chess;

pub fn get_best_move(board: &mut chess::Board) -> chess::Action {
    let mut moves = board.get_legal_actions();
    moves.pop().unwrap()
}
