import init, { get_legal_actions, get_best_move, get_state } from "./pkg/milkchess.js";

const gameDiv = document.getElementById("game");
let divWidth = window.innerWidth;
let divHeight = window.innerHeight;

class Color {
    static Black = new Color("black");
    static White = new Color("white");

    constructor(name) {
        this.name = name;
    }
}

class Type {
    static Pawn = new Type("pawn");
    static Rook = new Type("rook");
    static Knight = new Type("knight");
    static Bishop = new Type("bishop");
    static Queen = new Type("queen");
    static King = new Type("king");

    constructor(name) {
        this.name = name;
    }
}

class Piece {
    constructor(color, type, x, y) {
        this.color = color;
        this.type = type;
        this.x = x;
        this.y = y;
    }

    draw(size) {
        let pieceImg = document.createElement("img");
        pieceImg.src = `./assets/${this.color.name}_${this.type.name}.svg`;
        pieceImg.style.width = size + "px";
        pieceImg.style.height = size + "px";
        pieceImg.style.position = "absolute";
        pieceImg.style.top = this.y * size + "px";
        pieceImg.style.left = this.x * size + "px";
        pieceImg.onmousedown = function () {
            pieceImg.drag = true;
            // re-append to the end of the div so that it appears on top
            gameDiv.appendChild(pieceImg);
            return false;
        };
        pieceImg.onmousemove = function (ev) {
            if (pieceImg.drag) {
                pieceImg.style.left = ev.clientX - pieceImg.width / 2 + "px";
                pieceImg.style.top = ev.clientY - pieceImg.height / 2 + "px";
                return false;
            }
            return true;
        };
        pieceImg.onmouseup = function () {
            pieceImg.drag = false;
            let square = {
                x: Math.min(Math.max(Math.floor(
                    pieceImg.offsetLeft / size + 0.5),
                    0), 7),
                y: Math.min(Math.max(Math.floor(
                    pieceImg.offsetTop / size + 0.5),
                    0), 7)
            };
            pieceImg.style.top = square.y * size + "px";
            pieceImg.style.left = square.x * size + "px";
            console.log(board.serialize());
            console.log(get_legal_actions(board.serialize()));
            return false;
        };
        pieceImg.drag = false;

        gameDiv.appendChild(pieceImg);
    }
}

const board = new class {
    constructor() {
        this.squares = [];
        this.can_castle = {
            black: { queenSide: true, kingSide: true },
            white: { queenSide: true, kingSide: true },
        };
        this.can_en_passant = false;
        this.turn = Color.White;

        this.reset();
    }

    reset() {
        for (let y = 0; y < 8; y++) {
            this.squares[y] = [];
            for (let x = 0; x < 8; x++) {
                this.squares[y][x] = null;
            }
        }
        this.deserialize(
            "rnbqkbnr" +
            "pppppppp" +
            "........" +
            "........" +
            "........" +
            "........" +
            "PPPPPPPP" +
            "RNBQKBNR" +
            "ttttfw"
        );
    }

    draw(size) {
        let boardImg = document.createElement("img");
        boardImg.src = "./assets/board.svg";
        boardImg.style.width = size + "px";
        boardImg.style.height = size + "px";
        boardImg.style.top = 0 + "px";
        boardImg.style.left = 0 + "px";
        boardImg.style.position = "absolute";
        gameDiv.appendChild(boardImg);

        for (let x = 0; x < 8; x++) {
            for (let y = 0; y < 8; y++) {
                let piece = this.squares[y][x];
                if (piece != null) {
                    piece.draw(size / 8)
                }
            }
        }
    }

    serialize() {
        let serialized = "";
        for (let y = 0; y < 8; y++) {
            for (let x = 0; x < 8; x++) {
                let piece = this.squares[y][x];
                if (piece != null) {
                    let letter = {
                        "pawn": "p",
                        "rook": "r",
                        "knight": "n",
                        "bishop": "b",
                        "queen": "q",
                        "king": "k"
                    }[piece.type.name];
                    if (piece.color === Color.White) {
                        letter = letter.toUpperCase();
                    }
                    serialized += letter;
                } else {
                    serialized += ".";
                }
            }
        }
        for (let color in this.can_castle) {
            for (let side in this.can_castle[color]) {
                if (this.can_castle[color][side]) {
                    serialized += "t";
                } else {
                    serialized += "f";
                }
            }
        }
        if (this.can_en_passant) {
            serialized += this.can_en_passant;
        } else {
            serialized += "f";
        }
        serialized += this.turn == Color.Black ? "b" : "w";
        return serialized;
    }

    deserialize(serialized) {
        for (let y = 0; y < 8; y++) {
            for (let x = 0; x < 8; x++) {
                let piece = serialized[y * 8 + x];
                if (piece === ".") {
                    this.squares[y][x] = null;
                } else {
                    let color = piece.toLowerCase() === piece ? Color.Black : Color.White;
                    let type = {
                        "p": Type.Pawn,
                        "r": Type.Rook,
                        "n": Type.Knight,
                        "b": Type.Bishop,
                        "q": Type.Queen,
                        "k": Type.King
                    }[piece.toLowerCase()];
                    this.squares[y][x] = new Piece(color, type, x, y);
                }
            }
        }
        let i = 8 * 8;
        for (let color in this.can_castle) {
            for (let side in this.can_castle[color]) {
                this.can_castle[color][side] = serialized[i] === "t";
                i++;
            }
        }
        if (serialized[i++] === "f") {
            this.can_en_passant = false;
        } else {
            this.can_en_passant = parseInt(serialized[i]);
        }
        this.turn = serialized[i] === "b" ? Color.Black : Color.White;
    }
}

window.onresize = function () {
    divWidth = window.innerWidth;
    divHeight = window.innerHeight;
    redraw();
};

function redraw() {
    gameDiv.innerHTML = "";
    gameDiv.style.width = divWidth + "px";
    gameDiv.style.height = divHeight + "px";

    const statusHeight = Math.min(divHeight / 8, divWidth / 8);
    const boardSize = Math.min(divHeight - statusHeight, divWidth);
    board.draw(boardSize);

    let statusDiv = document.createElement("div");
    statusDiv.style.width = boardSize + "px";
    statusDiv.style.height = statusHeight + "px";
    statusDiv.style.position = "absolute";
    statusDiv.style.top = boardSize + "px";
    statusDiv.style.fontSize = statusHeight / 2 + "px";
    statusDiv.innerHTML = "Hello, world!";
    gameDiv.appendChild(statusDiv);
}

init();
redraw();
