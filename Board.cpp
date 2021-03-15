#include "Board.h"
#include "Game.h"
#include <iostream>
#include "Window.h"
#include "BoardState.h"
#include "Piece.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define HIGHLIGHT_COLOR {0,255,0,100}
#define AMOUNT_OF_BOX .8
#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
int Board::boxXWidth;
int Board::boxYHeight;
int Board::boardXBoxes;
int Board::boardYBoxes;


void Board::init() {
	Piece::init();
	boardXBoxes = 8;
	boardYBoxes = 8;
	boxXWidth = Window::screenWidth / boardXBoxes;
	boxYHeight = Window::screenHeight / boardYBoxes;

	boardColor1 = { 234,233,210,255 };

	boardColor2 = { 75,115,153,255 };

	boardState = new BoardState();

	unsigned int** board = new unsigned int* [boardXBoxes];
	for (int i = 0; i < boardXBoxes; ++i) {
		board[i] = new unsigned int[boardYBoxes];
	}
	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {

			board[x][y] = 0;
		}
	}
	boardState->setBoard(board);
	draggingPiece = false;
	draggingPieceX = -1;
	draggingPieceY = -1;
	loadBoardFromFen(STARTING_FEN, boardState);

	calculateLegalMoves(boardState);
	
	printBoardState(boardState);


}

Board::~Board() {
	Piece::destroyImages();

	unsigned int** board = boardState->getBoard();
	for (int x = 0; x < boardXBoxes; x++) {
		delete(board[x]);
	}
	delete(board);
	delete(boardState);
}

void Board::render(BoardState* currentBoardState) {
	renderBoard();
	if (draggingPiece) {
		renderHighlightMoves();
	}
	renderPieces(currentBoardState);
	if (draggingPiece) {
		renderDraggedPiece();
	}

}


void Board::renderDraggedPiece() {
	int w, h, mouseX, mouseY;
	SDL_QueryTexture(draggingPieceTexture, NULL, NULL, &w, &h);
	SDL_GetMouseState(&mouseX, &mouseY);
	SDL_Rect fromRect, toRect;
	fromRect.w = w;
	fromRect.h = h;
	fromRect.x = fromRect.y = 0;

	toRect.w = boxXWidth * AMOUNT_OF_BOX;
	toRect.h = boxYHeight * AMOUNT_OF_BOX;



	toRect.x = mouseX - (toRect.w / 2);
	toRect.y = mouseY - (toRect.h / 2);


	SDL_RenderCopy(Window::renderer, draggingPieceTexture, &fromRect, &toRect);

}

BoardState* Board::getBoardState() {
	return boardState;
}

void Board::renderBoard() {

	SDL_Rect drawRect;
	drawRect.w = boxXWidth;
	drawRect.h = boxYHeight;
	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {

			SDL_Color currentColor = (x + y) % 2 == 0 ? boardColor1 : boardColor2;
			drawRect.x = Game::boardTopLeftX + x * boxXWidth;
			drawRect.y = Game::boardTopLeftY + y * boxYHeight;
			SDL_SetRenderDrawColor(Window::renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
			SDL_RenderFillRect(Window::renderer, &drawRect);



		}
	}
}


void Board::renderPieces(BoardState* currentBoardState) {
	unsigned int** board = currentBoardState->getBoard();
	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {
			if (board[x][y] != 0) {
				if (x != draggingPieceX || y != draggingPieceY) {
					renderPiece(x, y, currentBoardState);
				}
			}
		}
	}
}

void Board::renderPiece(int x, int y, BoardState* currentBoardState) {
	unsigned int currentPiece = currentBoardState->getBoard()[x][y];
	SDL_Texture* currentTexture = getTextureAtLocation(x, y, currentBoardState);
	renderPieceTexture(currentTexture, x, y);
}

SDL_Texture* Board::getTextureAtLocation(int x, int y, BoardState* currentBoardState) {
	unsigned int currentPiece = currentBoardState->getBoard()[x][y];
	if (currentPiece == (Piece::black | Piece::king)) {
		return Piece::blackKingTexture;
	}
	else if (currentPiece == (Piece::black | Piece::queen)) {
		return Piece::blackQueenTexture;
	}
	else if (currentPiece == (Piece::black | Piece::bishop)) {
		return Piece::blackBishopTexture;
	}
	else if (currentPiece == (Piece::black | Piece::knight)) {
		return Piece::blackKnightTexture;
	}
	else if (currentPiece == (Piece::black | Piece::pawn)) {
		return Piece::blackPawnTexture;
	}
	else if (currentPiece == (Piece::black | Piece::rook)) {
		return Piece::blackRookTexture;
	}
	else if (currentPiece == (Piece::white | Piece::king)) {
		return Piece::whiteKingTexture;
	}
	else if (currentPiece == (Piece::white | Piece::queen)) {
		return Piece::whiteQueenTexture;
	}
	else if (currentPiece == (Piece::white | Piece::bishop)) {
		return Piece::whiteBishopTexture;
	}
	else if (currentPiece == (Piece::white | Piece::knight)) {
		return Piece::whiteKnightTexture;
	}
	else if (currentPiece == (Piece::white | Piece::pawn)) {
		return Piece::whitePawnTexture;
	}
	else if (currentPiece == (Piece::white | Piece::rook)) {
		return Piece::whiteRookTexture;
	}
}

void Board::renderPieceTexture(SDL_Texture* texture, int x, int y) {
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	SDL_Rect fromRect, toRect;
	fromRect.w = w;
	fromRect.h = h;
	fromRect.x = fromRect.y = 0;

	toRect.w = boxXWidth * AMOUNT_OF_BOX;
	toRect.h = boxYHeight * AMOUNT_OF_BOX;


	toRect.x = Game::boardTopLeftX + x * boxXWidth + (boxXWidth - toRect.w) / 2;
	toRect.y = Game::boardTopLeftY + y * boxYHeight + (boxYHeight - toRect.h) / 2;


	SDL_RenderCopy(Window::renderer, texture, &fromRect, &toRect);

}

void Board::update() {

}


void Board::resize() {
	boxXWidth = Window::screenWidth / boardXBoxes;
	boxYHeight = Window::screenHeight / boardYBoxes;

	if (boxXWidth > boxYHeight) {
		boxXWidth = boxYHeight;
	}
	else {
		boxYHeight = boxXWidth;
	}
}


int Board::getHeight() {
	return boardYBoxes * boxYHeight;
}

int Board::getWidth() {
	return boardXBoxes * boxXWidth;
}

void Board::printBoardState(BoardState* currentBoardState) {
	if (currentBoardState->getWhiteCanKingsideCastle()) {
		std::cout << "White can kingside castle." << std::endl;
	}
	else {
		std::cout << "White cannot kingside castle." << std::endl;
	}
	if (currentBoardState->getWhiteCanQueensideCastle()) {
		std::cout << "White can queenside castle." << std::endl;
	}
	else {
		std::cout << "White cannot queenside castle." << std::endl;
	}
	if (currentBoardState->getBlackCanKingsideCastle()) {
		std::cout << "Black can kingside castle." << std::endl;
	}
	else {
		std::cout << "Black cannot kingside castle." << std::endl;
	}
	if (currentBoardState->getBlackCanQueensideCastle()) {
		std::cout << "Black can Queenside castle." << std::endl;
	}
	else {
		std::cout << "Black cannot Queenside castle." << std::endl;
	}
	if (currentBoardState->getEnPassantX() != -1) {
		std::cout << "En passant opportunity at x: " << boardState->getEnPassantX() << " and y: " << boardState->getEnPassantY() << std::endl;
	}
	else {
		std::cout << " no en passant :( holy hell" << std::endl;
	}

}

void Board::loadBoardFromFen(const char* fenNotation, BoardState* currentBoardState) {
	int index = 0;
	int column;

	unsigned int** board = currentBoardState->getBoard();
	for (int rank = 0; rank < boardYBoxes; rank++) {
		column = 0;
		while (fenNotation[index] != '/' && fenNotation[index] != ' ') {
			if (isdigit(fenNotation[index])) {
				index++;
				column += (fenNotation[index] - '0');
			}
			else {

				switch (fenNotation[index]) {
				case 'P':
					board[column][rank] = Piece::white | Piece::pawn;
					break;
				case 'p':
					board[column][rank] = Piece::black | Piece::pawn;
					break;
				case 'R':
					board[column][rank] = Piece::white | Piece::rook;
					break;
				case 'r':
					board[column][rank] = Piece::black | Piece::rook;
					break;
				case 'N':
					board[column][rank] = Piece::white | Piece::knight;
					break;
				case 'n':
					board[column][rank] = Piece::black | Piece::knight;
					break;
				case 'B':
					board[column][rank] = Piece::white | Piece::bishop;
					break;
				case 'b':
					board[column][rank] = Piece::black | Piece::bishop;
					break;
				case 'Q':
					board[column][rank] = Piece::white | Piece::queen;
					break;
				case 'q':
					board[column][rank] = Piece::black | Piece::queen;
					break;
				case 'K':
					board[column][rank] = Piece::white | Piece::king;
					break;
				case 'k':
					board[column][rank] = Piece::black | Piece::king;
					break;
				}
				++index;
				++column;
			}
		}
		++index;
	}
	//now we are out of the long /'s, and are onto the current players turn.
	currentBoardState->setCurrentTurn(fenNotation[index]);
	index += 2;
	currentBoardState->setWhiteCanKingsideCastle(false);
	currentBoardState->setBlackCanKingsideCastle(false);
	currentBoardState->setWhiteCanQueensideCastle(false);
	currentBoardState->setBlackCanQueensideCastle(false);

	//now we are onto the castling.
	if (fenNotation[index] == '-') {

		index += 2;
	}
	else {
		while (fenNotation[index] != ' ') {
			switch (fenNotation[index]) {
			case 'K':
				currentBoardState->setWhiteCanKingsideCastle(true);
				break;
			case 'k':
				currentBoardState->setBlackCanKingsideCastle(true);
				break;
			case 'Q':
				currentBoardState->setWhiteCanQueensideCastle(true);
				break;
			case 'q':
				currentBoardState->setBlackCanQueensideCastle(true);
				break;
			}
			++index;
		}
		++index;
	}

	//now we are onto the en-passant option.

	if (fenNotation[index] == '-') {
		index += 2;
		currentBoardState->setEnPassantX(-1);
		currentBoardState->setEnPassantY(-1);
	}
	else {
		currentBoardState->setEnPassantX(fenNotation[index] - 'a');
		++index;
		currentBoardState->setEnPassantY(boardYBoxes - (fenNotation[index] - '0'));
		++index;
	}

	//come back and do move clocks later, ignore these for now becaues I'm lazy
	//and don't want to parse c-strings :)


}



void Board::handleMouseButtonDown(SDL_MouseButtonEvent& b, BoardState* currentBoardState) {
	int x, y, boardX, boardY;
	if (b.button == SDL_BUTTON_LEFT) {
		SDL_GetMouseState(&x, &y);
		if (x < Game::boardTopLeftX || x > Game::boardTopLeftX + getWidth()) {

		}
		else if (y < Game::boardTopLeftY || y > Game::boardTopLeftY + getHeight()) {

		}
		else {
			boardX = (x - Game::boardTopLeftX) / boxXWidth;
			boardY = (y - Game::boardTopLeftY) / boxYHeight;
			if (!draggingPiece) {
				attemptPickupPiece(boardX, boardY, currentBoardState);
			}
			else {
				attemptPlacePiece(boardX, boardY, currentBoardState);
			}
		}

	}
	else if (b.button == SDL_BUTTON_RIGHT) {
		stopDraggingPiece();
	}
}

void Board::attemptPickupPiece(int x, int y, BoardState* currentBoardState) {

	//if there's a piece on the board space we're clicking.
	if (currentBoardState->getBoard()[x][y] != 0) {
		//if it's the piece for the curernt players turn.

		if (pieceIsCurrentPlayersPiece(x, y, currentBoardState)) {
			draggingPiece = true;
			draggingPieceX = x;
			draggingPieceY = y;
			draggingPieceTexture = getTextureAtLocation(x, y, currentBoardState);
			createHighlightMoves(x, y);
		}

	}
}

void Board::attemptPlacePiece(int x, int y, BoardState* currentBoardState) {
	Move newMove = { draggingPieceX,draggingPieceY,x,y };
	unsigned int** board = currentBoardState->getBoard();
	int enPassantX = currentBoardState->getEnPassantX();
	int enPassantY = currentBoardState->getEnPassantY();
	//inLegalMoves passes in a reference so we update the castling and sturff here.
	if (inLegalMoves(newMove)) {
		
		makeMove(newMove, currentBoardState);
		switchTurns(currentBoardState);
		stopDraggingPiece();
		calculateLegalMoves(currentBoardState);
		
	}
	else {
		stopDraggingPiece();
	}

}
void Board::switchTurns(BoardState* currentBoardState) {
	currentBoardState->setCurrentTurn((currentBoardState->getCurrentTurn() == 'w') ? 'b' : 'w');

	
}

bool Board::inLegalMoves(struct Move& newMove) {

	for (int i = 0; i < legalMoves.size(); i++) {
		
		if (newMove.fromX == legalMoves.at(i).fromX &&
			newMove.toX == legalMoves.at(i).toX &&
			newMove.fromY == legalMoves.at(i).fromY &&
			newMove.toY == legalMoves.at(i).toY) {

			newMove.kingSideCastle = legalMoves.at(i).kingSideCastle;
			newMove.queenSideCastle = legalMoves.at(i).queenSideCastle;
			newMove.enPassant = legalMoves.at(i).enPassant;
			
			return true;
		}
	}
	return false;
}
bool Board::inPseudoMoves(struct Move& newMove) {
	
	for (int i = 0; i < pseudoLegalMoves.size(); i++) {
		
		if (newMove.fromX == pseudoLegalMoves.at(i).fromX &&
			newMove.toX == pseudoLegalMoves.at(i).toX &&
			newMove.fromY == pseudoLegalMoves.at(i).fromY &&
			newMove.toY == pseudoLegalMoves.at(i).toY) {

			newMove.kingSideCastle = pseudoLegalMoves.at(i).kingSideCastle;
			newMove.queenSideCastle = pseudoLegalMoves.at(i).queenSideCastle;
			newMove.enPassant = pseudoLegalMoves.at(i).enPassant;
			
			return true;
		}
	}
	return false;
}

void Board::stopDraggingPiece() {
	draggingPiece = false;
	draggingPieceX = draggingPieceY = -1;
}

void Board::clearMoves() {
	pseudoLegalMoves.clear();
	legalMoves.clear();
}

std::vector<Move> Board::calculatePseudoLegalMoves(BoardState* currentBoardState) {
	std::vector<Move> returnVec;
	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {
			if (currentBoardState->getBoard()[x][y] != 0) {

				//if the piece is a piece that can be moved this turn.
				if (pieceIsCurrentPlayersPiece(x, y, currentBoardState)) {
					calculateMovesAt(x, y, currentBoardState, returnVec);
				}
			}

		}
	}

	return returnVec;
}

void Board::calculateLegalMoves(BoardState* currentBoardState) {
	legalMoves.clear();
	//std::cout << "Calling this once" << std::endl;
	
	BoardState* newBoardState;
	pseudoLegalMoves = calculatePseudoLegalMoves(currentBoardState);
	for (int i = 0; i < pseudoLegalMoves.size(); i++) {
		
		newBoardState = BoardState::copyBoardState(currentBoardState);
		
		makeMove(pseudoLegalMoves.at(i), newBoardState);
		
	
		if (!kingInCheck(newBoardState)) {
	
			//because kingInCheck changed the vector... but it's not working
			
		
			legalMoves.push_back({ pseudoLegalMoves.at(i).fromX,
				pseudoLegalMoves.at(i).fromY,
				pseudoLegalMoves.at(i).toX,
				pseudoLegalMoves.at(i).toY,
				pseudoLegalMoves.at(i).kingSideCastle,
				pseudoLegalMoves.at(i).queenSideCastle,
				pseudoLegalMoves.at(i).enPassant }
				);
			
		}
		
		delete(newBoardState);
		
	}
}

void Board::makeMove(struct Move move, BoardState* currentBoardState) {
	
	unsigned int** board = currentBoardState->getBoard();
	int enPassantX = currentBoardState->getEnPassantX();
	int enPassantY = currentBoardState->getEnPassantY();

	if (isEnPassant(move.fromX, move.fromY, move.toX, move.fromX, currentBoardState)) {
		//the piece removed depends on the turn.
		if (currentBoardState->getCurrentTurn() == 'w') {
			board[enPassantX][enPassantY + 1] = 0;
		}
		else {
			board[enPassantX][enPassantY - 1] = 0;
		}

	}
	if (move.kingSideCastle) {
		std::cout << "Kingside Castle" << std::endl;
		board[move.toX - 1][move.toY] = board[move.toX + 1][move.toY];
		board[move.toX + 1][move.toY] = 0;
	}
	else if (move.queenSideCastle) {
		std::cout << "Queenside Castle" << std::endl;
		board[move.toX + 1][move.toY] = board[move.toX - 2][move.toY];
		board[move.toX - 2][move.toY] = 0;
	}

	updateEnPassant(move.fromX, move.fromY, move.toX, move.toY, currentBoardState);
	updateCastling(move.fromX, move.fromY, move.toX, move.toY, currentBoardState);
	board[move.toX][move.toY] = board[move.fromX][move.fromY];
	board[move.fromX][move.fromY] = 0;

}

//returns true if a piece is a current players piece, false otherwise.
bool Board::pieceIsCurrentPlayersPiece(int x, int y, BoardState* currentBoardState) {
	char currentTurn = currentBoardState->getCurrentTurn();
	unsigned int** board = currentBoardState->getBoard();
	if ((currentTurn == 'w' && (board[x][y] & Piece::white) == Piece::white) || (currentTurn == 'b' && (board[x][y] & Piece::black) == Piece::black)) {
		return true;
	}
	return false;
}

void Board::calculateMovesAt(int x, int y, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {
	unsigned int** board = currentBoardState->getBoard();
	if ((board[x][y] & Piece::rook) == Piece::rook) {

		calculateRookMoves(x, y, currentBoardState,currentPseudo);
	}
	else if ((board[x][y] & Piece::queen) == Piece::queen) {
		calculateQueenMoves(x, y, currentBoardState,currentPseudo);
	}
	else if ((board[x][y] & Piece::knight) == Piece::knight) {
		calculateKnightMoves(x, y, currentBoardState,currentPseudo);
	}
	else if ((board[x][y] & Piece::king) == Piece::king) {

		calculateKingMoves(x, y, currentBoardState,currentPseudo);
	}
	else if ((board[x][y] & Piece::bishop) == Piece::bishop) {
		calculateBishopMoves(x, y, currentBoardState,currentPseudo);
	}
	else if ((board[x][y] & Piece::pawn) == Piece::pawn) {
		calculatePawnMoves(x, y, currentBoardState,currentPseudo);
	}
}

void Board::calculateRookMoves(int x, int y, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {
	unsigned int** board = currentBoardState->getBoard();
	//going to the right on the board.
	for (int currX = x + 1; currX < boardXBoxes; ++currX) {
		if (board[currX][y] == 0) {
			currentPseudo.push_back({ x,y,currX,y, false, false, false });

		}
		else if (pieceIsCurrentPlayersPiece(currX, y, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(currX, y, currentBoardState)) {
			currentPseudo.push_back({ x,y,currX,y, false, false, false });
			break;

		}
	}
	//going to the left on the board.
	for (int currX = x - 1; currX >= 0; --currX) {
		if (board[currX][y] == 0) {
			currentPseudo.push_back({ x,y,currX,y, false, false, false });

		}
		else if (pieceIsCurrentPlayersPiece(currX, y, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(currX, y, currentBoardState)) {
			currentPseudo.push_back({ x,y,currX,y, false, false, false });
			break;

		}
	}
	//going up on the board
	for (int currY = y - 1; currY >= 0; --currY) {
		if (board[x][currY] == 0) {
			currentPseudo.push_back({ x,y,x,currY, false, false, false });

		}
		else if (pieceIsCurrentPlayersPiece(x, currY, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x, currY, currentBoardState)) {
			currentPseudo.push_back({ x,y,x,currY, false, false, false });
			break;

		}
	}
	//going down the board
	for (int currY = y + 1; currY < boardYBoxes; ++currY) {
		if (board[x][currY] == 0) {
			currentPseudo.push_back({ x,y,x,currY, false, false, false });

		}
		else if (pieceIsCurrentPlayersPiece(x, currY, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x, currY, currentBoardState)) {
			currentPseudo.push_back({ x,y,x,currY, false, false, false });
			break;

		}
	}


}
void Board::calculateBishopMoves(int x, int y, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {
	//going down and to the right
	unsigned int** board = currentBoardState->getBoard();
	for (int change = 1; x + change < boardXBoxes && y + change < boardYBoxes; ++change) {
		if (board[x + change][y + change] == 0) {
			currentPseudo.push_back({ x,y,x + change,y + change, false, false, false });

		}
		else if (pieceIsCurrentPlayersPiece(x + change, y + change, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x + change, y + change, currentBoardState)) {
			currentPseudo.push_back({ x,y,x + change,y + change, false, false, false });
			break;
		}
	}
	//going down and to the left
	for (int change = 1; x - change >= 0 && y + change < boardYBoxes; ++change) {
		if (board[x - change][y + change] == 0) {
			currentPseudo.push_back({ x,y,x - change,y + change, false, false, false });

		}
		else if (pieceIsCurrentPlayersPiece(x - change, y + change, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x - change, y + change, currentBoardState)) {
			currentPseudo.push_back({ x,y,x - change,y + change, false, false, false });
			break;
		}

	}
	//going up and to the left.
	for (int change = 1; x - change >= 0 && y - change >= 0; ++change) {
		if (board[x - change][y - change] == 0) {
			currentPseudo.push_back({ x,y,x - change,y - change, false, false, false });

		}
		else if (pieceIsCurrentPlayersPiece(x - change, y - change, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x - change, y - change, currentBoardState)) {
			currentPseudo.push_back({ x,y,x - change,y - change, false, false, false });
			break;
		}
	}

	//going up and to the right
	for (int change = 1; x + change < boardXBoxes && y - change >= 0; ++change) {
		if (board[x + change][y - change] == 0) {
			currentPseudo.push_back({ x,y,x + change,y - change, false, false, false });

		}
		else if (pieceIsCurrentPlayersPiece(x + change, y - change, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x + change, y - change, currentBoardState)) {
			currentPseudo.push_back({ x,y,x + change,y - change, false, false, false });
			break;
		}
	}


}
void Board::calculateQueenMoves(int x, int y, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {
	calculateBishopMoves(x, y, currentBoardState,currentPseudo);
	calculateRookMoves(x, y, currentBoardState,currentPseudo);
}
void Board::calculateKingMoves(int x, int y, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {
	unsigned int** board = currentBoardState->getBoard();
	//king moving down
	if (y + 1 < boardYBoxes) {
		for (int xChange = -1; xChange <= 1; xChange++) {
			if (x + xChange < 0 || x + xChange >= boardXBoxes) {
				continue;
			}
			if (board[x + xChange][y + 1] == 0 || !pieceIsCurrentPlayersPiece(x + xChange, y + 1, currentBoardState)) {
				currentPseudo.push_back({ x,y,x + xChange,y + 1, false, false, false });
			}
		}
	}

	//king moving up
	if (y - 1 >= 0) {
		for (int xChange = -1; xChange <= 1; xChange++) {
			if (x + xChange < 0 || x + xChange >= boardXBoxes) {
				continue;
			}
			if (board[x + xChange][y - 1] == 0 || !pieceIsCurrentPlayersPiece(x + xChange, y - 1, currentBoardState)) {
				currentPseudo.push_back({ x,y,x + xChange,y - 1, false, false, false });
			}
		}
	}

	//king moving to the left
	if (x - 1 >= 0) {
		if (board[x - 1][y] == 0 || !pieceIsCurrentPlayersPiece(x - 1, y, currentBoardState)) {
			currentPseudo.push_back({ x,y,x - 1,y, false, false, false });
		}
	}
	//king moving to the right
	if (x + 1 < boardXBoxes) {
		if (board[x + 1][y] == 0 || !pieceIsCurrentPlayersPiece(x + 1, y, currentBoardState)) {
			currentPseudo.push_back({ x,y,x + 1,y, false, false, false });
		}
	}

	//add castling NOW
	calculateCastlingMoves(x, y, currentBoardState,currentPseudo);

}

//just to separate this a bit from the king moves, because it's kinda busy.
void Board::calculateCastlingMoves(int x, int y, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {
	unsigned int** board = currentBoardState->getBoard();
	if ((board[x][y] & Piece::white) == Piece::white) {
		if (currentBoardState->getWhiteCanKingsideCastle()) {
			if (board[x + 1][y] == 0 && board[x + 2][y] == 0) {
				currentPseudo.push_back({ x,y,x + 2,y,true,false,false });
			}
		}
		if (currentBoardState->getWhiteCanQueensideCastle()) {
			if (board[x - 1][y] == 0 && board[x - 2][y] == 0) {
				currentPseudo.push_back({ x,y,x - 2,y,false,true,false });
			}
		}
	}
	else {
		if (currentBoardState->getBlackCanKingsideCastle()) {
			if (board[x + 1][y] == 0 && board[x + 2][y] == 0) {
				currentPseudo.push_back({ x,y,x + 2,y,true,false,false });
			}
		}
		if (currentBoardState->getBlackCanQueensideCastle()) {
			if (board[x - 1][y] == 0 && board[x - 2][y] == 0) {
				currentPseudo.push_back({ x,y,x - 2,y,false,true,false });
			}
		}
	}

}

//update castling status.
void Board::updateCastling(int fromX, int fromY, int toX, int toY, BoardState* currentBoardState) {
	//if the king moved.
	int queenSideX = 0;
	int kingSideX = boardXBoxes - 1;
	int whiteY = boardYBoxes - 1;
	int blackY = 0;

	unsigned int** board = boardState->getBoard();
	if (currentBoardState->getCurrentTurn() == 'w') {
		if ((board[fromX][fromY] & Piece::king) == Piece::king) {
			currentBoardState->setWhiteCanKingsideCastle(false);
			currentBoardState->setWhiteCanQueensideCastle(false);
			currentBoardState->setWhiteCanQueensideCastle(false);
		}
		if (fromX == queenSideX && fromY == whiteY) {
			currentBoardState->setWhiteCanQueensideCastle(false);
		}
		else if (fromX == kingSideX && fromY == whiteY) {
			currentBoardState->setWhiteCanKingsideCastle(false);
		}
		if (toX == kingSideX && toY == blackY) {
			currentBoardState->setBlackCanKingsideCastle(false);
		}
		else if (toX == queenSideX && toY == blackY) {
			currentBoardState->setBlackCanQueensideCastle(false);
		}
	}
	else {
		if ((board[fromX][fromY] & Piece::king) == Piece::king) {
			currentBoardState->setBlackCanKingsideCastle(false);
			currentBoardState->setBlackCanQueensideCastle(false);
		}
		if (fromX == queenSideX && fromY == blackY) {
			currentBoardState->setBlackCanQueensideCastle(false);
		}
		else if (fromX == kingSideX && fromY == blackY) {
			currentBoardState->setBlackCanKingsideCastle(false);
		}
		//if the move takes a rook.
		if (toX == kingSideX && toY == whiteY) {
			currentBoardState->setWhiteCanKingsideCastle(false);
		}
		else if (toX == queenSideX && toY == whiteY) {
			currentBoardState->setWhiteCanQueensideCastle(false);
		}
	}


}


void Board::calculateKnightMoves(int x, int y, BoardState* currentBoardState,std::vector<Move>& currentPseudo) {
	//knight moving up.
	unsigned int** board = currentBoardState->getBoard();
	if (y - 2 >= 0) {
		if (x + 1 < boardXBoxes) {
			if (board[x + 1][y - 2] == 0 || !pieceIsCurrentPlayersPiece(x + 1, y - 2, currentBoardState)) {
				currentPseudo.push_back({ x, y, x + 1, y - 2, false, false, false });
			}
		}
		if (x - 1 >= 0) {
			if (board[x - 1][y - 2] == 0 || !pieceIsCurrentPlayersPiece(x - 1, y - 2, currentBoardState)) {
				currentPseudo.push_back({ x, y, x - 1, y - 2, false, false, false });
			}
		}
	}
	//knight moving down
	if (y + 2 < boardYBoxes) {
		if (x + 1 < boardXBoxes) {
			if (board[x + 1][y + 2] == 0 || !pieceIsCurrentPlayersPiece(x + 1, y + 2, currentBoardState)) {
				currentPseudo.push_back({ x, y, x + 1, y + 2, false, false, false });
			}
		}
		if (x - 1 >= 0) {
			if (board[x - 1][y + 2] == 0 || !pieceIsCurrentPlayersPiece(x - 1, y + 2, currentBoardState)) {
				currentPseudo.push_back({ x, y, x - 1, y + 2, false, false, false });
			}
		}
	}
	//knight moving left
	if (x - 2 >= 0) {
		if (y + 1 < boardYBoxes) {
			if (board[x - 2][y + 1] == 0 || !pieceIsCurrentPlayersPiece(x - 2, y + 1, currentBoardState)) {
				currentPseudo.push_back({ x, y, x - 2, y + 1, false, false, false });
			}
		}
		if (y - 1 >= 0) {
			if (board[x - 2][y - 1] == 0 || !pieceIsCurrentPlayersPiece(x - 2, y - 1, currentBoardState)) {
				currentPseudo.push_back({ x, y, x - 2, y - 1, false, false, false });
			}
		}
	}
	//knight moving right
	if (x + 2 < boardXBoxes) {
		if (y + 1 < boardYBoxes) {
			if (board[x + 2][y + 1] == 0 || !pieceIsCurrentPlayersPiece(x + 2, y + 1, currentBoardState)) {
				currentPseudo.push_back({ x, y, x + 2, y + 1, false, false, false });
			}
		}
		if (y - 1 >= 0) {
			if (board[x + 2][y - 1] == 0 || !pieceIsCurrentPlayersPiece(x + 2, y - 1, currentBoardState)) {
				currentPseudo.push_back({ x, y, x + 2, y - 1, false, false, false });
			}
		}
	}



}
void Board::calculatePawnMoves(int x, int y, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {
	unsigned int** board = currentBoardState->getBoard();
	//do promotion later.
	if (currentBoardState->getCurrentTurn() == 'w') {
		//moving forward one.
		if (y - 1 >= 0) {
			//pawns cant take vertically.
			if (board[x][y - 1] == 0) {
				currentPseudo.push_back({ x,y,x,y - 1 });
				//we only can move forward 2 if the space is open as well.
				if (y == boardYBoxes - 2) {	//if it's in the starting position.
					if (y - 2 >= 0) {	//this shouldn't be necessary except for tiny boards
						if (board[x][y - 2] == 0) {
							currentPseudo.push_back({x,y,x,y - 2, false, false, false });
						}
					}
				}
			}
		}

		//pawns capture diagonally.
		if (y - 1 >= 0) {
			//if we're not at the edge of the board
			if (x - 1 >= 0) {
				if (board[x - 1][y - 1] != 0 && !pieceIsCurrentPlayersPiece(x - 1, y - 1, currentBoardState)) {
					currentPseudo.push_back({ x,y,x - 1,y - 1, false, false, false });
				}
				//en passant
				else if (x - 1 == currentBoardState->getEnPassantX() && y - 1 == currentBoardState->getEnPassantY()) {
					currentPseudo.push_back({ x,y,x - 1,y - 1,false,false,true });
				}
			}
			if (x + 1 < boardXBoxes) {
				if (board[x + 1][y - 1] != 0 && !pieceIsCurrentPlayersPiece(x + 1, y - 1, currentBoardState)) {
					currentPseudo.push_back({ x,y,x + 1,y - 1, false, false, false });
				}
				//en passant
				else if (x + 1 == currentBoardState->getEnPassantX() && y - 1 == currentBoardState->getEnPassantY()) {
					currentPseudo.push_back({ x,y,x + 1,y - 1,false,false,true });
				}
			}
		}




	}
	else {
		//moving forward one.
		if (y + 1 < boardYBoxes) {
			//pawns cant take vertically.
			if (board[x][y + 1] == 0) {
				currentPseudo.push_back({ x,y,x,y + 1 });
				//we only can move forward 2 if the space is open as well.
				if (y == 1) {	//if it's in the starting position.
					if (y + 2 < boardYBoxes) {	//this shouldn't be necessary except for tiny boards
						if (board[x][y + 2] == 0) {
							currentPseudo.push_back({ x,y,x,y + 2, false, false, false });
						}
					}
				}
			}
		}

		//pawns capture diagonally.
		if (y + 1 < boardYBoxes) {
			//if we're not at the edge of the board
			if (x - 1 >= 0) {
				if (board[x - 1][y + 1] != 0 && !pieceIsCurrentPlayersPiece(x - 1, y + 1, currentBoardState)) {
					currentPseudo.push_back({ x,y,x - 1,y + 1, false, false, false });
				}
				else if (x - 1 == currentBoardState->getEnPassantX() && y + 1 == currentBoardState->getEnPassantY()) {
					currentPseudo.push_back({ x,y,x - 1,y + 1,false,false,true });
				}
			}
			if (x + 1 < boardXBoxes) {
				if (board[x + 1][y + 1] != 0 && !pieceIsCurrentPlayersPiece(x + 1, y + 1, currentBoardState)) {
					currentPseudo.push_back({ x,y,x + 1,y + 1, false, false, false });
				}
				else if (x + 1 == currentBoardState->getEnPassantX() && y + 1 == currentBoardState->getEnPassantY()) {
					currentPseudo.push_back({ x,y,x + 1,y + 1,false,false,true });
				}
			}
		}


	}


}


void Board::renderHighlightMoves() {
	SDL_Rect highlightRect;
	highlightRect.w = boxXWidth;
	highlightRect.h = boxYHeight;
	SDL_Color drawColor = HIGHLIGHT_COLOR;
	SDL_SetRenderDrawColor(Window::renderer, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
	for (int i = 0; i < highlightMoves.size(); i++) {
		highlightRect.x = Game::boardTopLeftX + highlightMoves.at(i).toX * boxXWidth;
		highlightRect.y = Game::boardTopLeftY + highlightMoves.at(i).toY * boxYHeight;
		SDL_RenderFillRect(Window::renderer, &highlightRect);
	}
}

void Board::createHighlightMoves(int x, int y) {
	highlightMoves.clear();
	for (int i = 0; i < legalMoves.size(); i++) {
		if (legalMoves.at(i).fromX == x &&
			legalMoves.at(i).fromY == y) {
			highlightMoves.push_back({
				legalMoves.at(i).fromX,
				legalMoves.at(i).fromY,
				legalMoves.at(i).toX,
				legalMoves.at(i).toY
				});
		}
	}
}


void Board::updateEnPassant(int fromX, int fromY, int toX, int toY, BoardState* currentBoardState) {

	if ((boardState->getBoard()[fromX][fromY] & Piece::pawn) == Piece::pawn) {
		if (abs(fromY - toY) == 2) {
			boardState->setEnPassantX(fromX);
			boardState->setEnPassantY((fromY + toY) / 2);

		}
		else {
			boardState->setEnPassantX(-1);
			boardState->setEnPassantY(-1);

		}
	}
	else {
		boardState->setEnPassantX(-1);
		boardState->setEnPassantY(-1);
	}
}

void Board::findKingLocation(int* xPos, int* yPos, BoardState* currentBoardState) {
	char currentPlayer = currentBoardState->getCurrentTurn();
	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {
			unsigned int currentSquare = currentBoardState->getBoard()[x][y];
			if (currentPlayer == 'w') {
				if (currentSquare == (Piece::white | Piece::king)) {
					*xPos = x;
					*yPos = y;
					return;
				}
			}
			else {
				if (currentSquare == (Piece::black | Piece::king)) {
					*xPos = x;
					*yPos = y;
					return;
				}
			}
		}
	}
}

bool Board::isEnPassant(int fromX, int fromY, int toX, int toY, BoardState* currentBoardState) {

	if ((currentBoardState->getBoard()[fromX][fromY] & Piece::pawn) == Piece::pawn) {
		if (toX == currentBoardState->getEnPassantX() && toY == currentBoardState->getEnPassantY()) {

			return true;
		}
	}

	return false;
}

bool Board::squareAttacked(int x, int y, BoardState* currentBoardState) {
	BoardState* newBoardState = BoardState::copyBoardState(currentBoardState);
	switchTurns(newBoardState);

	std::vector<Move> currentPseudo = calculatePseudoLegalMoves(newBoardState);

	for (int i = 0; i < currentPseudo.size(); i++) {
		if (currentPseudo.at(i).toX == x && currentPseudo.at(i).toY == y) {
			return true;

		}
	}
	delete(newBoardState);
	return false;
}

//figure out... is the king in check?
bool Board::kingInCheck(BoardState* currentBoardState) {
	//if the current players king is under attack then the king is in check.
	int kingX, kingY;
	findKingLocation(&kingX, &kingY, currentBoardState);

	bool result = squareAttacked(kingX, kingY, currentBoardState);

	
	return result;

}