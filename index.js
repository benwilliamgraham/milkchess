import init, { greet } from "./pkg/milkchess.js";

const gameDiv = document.getElementById("game");
let divWidth = window.innerWidth;
let divHeight = window.innerHeight;

const board = [];

class Piece {
    constructor(color, type, x, y) {
        this.color = color;
        this.type = type;
        this.x = x;
        this.y = y;
    }
}

window.onresize = function () {
    divWidth = window.innerWidth;
    divHeight = window.innerHeight;
    redraw();
};

function set_board_start_position() {
    for (let y = 0; y < 8; y++) {
        board[y] = [];
        for (let x = 0; x < 8; x++) {
            board[y][x] = null;
        }
    }
    for (let x = 0; x < 8; x++) {
        let order = ["rook", "knight", "bishop", "queen", "king", "bishop", "knight", "rook"];
        board[0][x] = new Piece("black", order[x], x, 0);
        board[1][x] = new Piece("black", "pawn", x, 1);
        board[6][x] = new Piece("white", "pawn", x, 6);
        board[7][x] = new Piece("white", order[x], x, 7);
    }
}

function redraw() {
    gameDiv.innerHTML = "";
    gameDiv.style.width = divWidth + "px";
    gameDiv.style.height = divHeight + "px";

    const statusHeight = Math.min(divHeight / 8, divWidth / 8);
    const boardHeight = Math.min(divHeight - statusHeight, divWidth);

    let boardImg = document.createElement("img");
    boardImg.src = "./assets/board.svg";
    boardImg.style.width = boardHeight + "px";
    boardImg.style.height = boardHeight + "px";
    boardImg.style.position = "absolute";
    gameDiv.appendChild(boardImg);

    let statusDiv = document.createElement("div");
    statusDiv.style.width = boardHeight + "px";
    statusDiv.style.height = statusHeight + "px";
    statusDiv.style.position = "absolute";
    statusDiv.style.top = boardHeight + "px";
    statusDiv.style.fontSize = statusHeight / 2 + "px";
    statusDiv.innerHTML = "Hello, world!";
    gameDiv.appendChild(statusDiv);

    for (let x = 0; x < 8; x++) {
        for (let y = 0; y < 8; y++) {
            let piece = board[y][x];
            if (piece == null) {
                continue;
            }
            let pieceImg = document.createElement("img");
            pieceImg.src = "./assets/" + piece.color + "_" + piece.type + ".svg";
            pieceImg.style.width = boardHeight / 8 + "px";
            pieceImg.style.height = boardHeight / 8 + "px";
            pieceImg.style.position = "absolute";
            pieceImg.style.top = y * boardHeight / 8 + "px";
            pieceImg.style.left = x * boardHeight / 8 + "px";
            pieceImg.onmousedown = function () {
                pieceImg.drag = true;
                return false;
            };
            pieceImg.onmousemove = function (ev) {
                if (pieceImg.drag) {
                    pieceImg.style.left = ev.clientX - pieceImg.width / 2 + "px";
                    pieceImg.style.top = ev.clientY - pieceImg.height / 2 + "px";
                }
            };
            pieceImg.onmouseup = function () {
                pieceImg.drag = false;
                let square = {
                    x: Math.min(Math.max(Math.floor(
                        pieceImg.offsetLeft / boardHeight * 8 + 0.5),
                        0), 7),
                    y: Math.min(Math.max(Math.floor(
                        pieceImg.offsetTop / boardHeight * 8 + 0.5),
                        0), 7)
                };
                pieceImg.style.top = square.y * boardHeight / 8 + "px";
                pieceImg.style.left = square.x * boardHeight / 8 + "px";
                return false;
            };
            pieceImg.drag = false;

            gameDiv.appendChild(pieceImg);
        }
    }
}

init();
set_board_start_position();
redraw();
