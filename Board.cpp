#include "Board.h"
#include "Game.h"
#include <iostream>
#include "Window.h"
#include "KingThreatenedInfo.h"
#include "BoardState.h"
#include "Piece.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define HIGHLIGHT_COLOR {0,255,0,50}
#define ATTACK_COLOR {255,0,0,100}
#define DANGER_COLOR {255,128,0,100}
#define WIN_COLOR {255,215,0,200}
#define LAST_MOVE_COLOR {0,0,255,100}
#define AMOUNT_OF_BOX .8
#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TEST_FEN "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
#define totalTests 4
int Board::boxXWidth;
int Board::boxYHeight;
int Board::boardXBoxes;
int Board::boardYBoxes;

const uint8_t KingThreatenedInfo::straightLeftThreatened = 0b00000001;
const uint8_t KingThreatenedInfo::upLeftThreatened = 0b00000010;
const uint8_t KingThreatenedInfo::straightUpThreatened = 0b00000100;
const uint8_t KingThreatenedInfo::upRightThreatened = 0b00001000;
const uint8_t KingThreatenedInfo::straightRightThreatened = 0b00010000;
const uint8_t KingThreatenedInfo::downRightThreatened = 0b00100000;
const uint8_t KingThreatenedInfo::straightDownThreatened = 0b01000000;
const uint8_t KingThreatenedInfo::downLeftThreatened = 0b10000000;


void Board::reset() {
	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {

			boardState->getBoard()[x][y] = 0;
		}
	}
	draggingPiece = false;
	draggingPieceBox.x = -1;
	draggingPieceBox.y = -1;
	loadBoardFromFen(TEST_FEN, boardState);
	initializePieceLocations(boardState);
	updateAllThreats('w',boardState);
	updateAllThreats('b', boardState);
	moveStack.clear();

	legalMoves = calculateLegalMoves(boardState);

	highlightKingBox.x = -1;
	highlightKingBox.y = -1;
	winnerKing.x = winnerKing.y = -1;
	gameOver = false;
}

void Board::init() {
	Piece::init();
	boardXBoxes = 8;
	boardYBoxes = 8;
	boxXWidth = Window::screenWidth / boardXBoxes;
	boxYHeight = Window::screenHeight / boardYBoxes;

	boardColor1 = { 234,233,210,255 };

	boardColor2 = { 75,115,153,255 };

	boardState = new BoardState();

	uint8_t** board = new uint8_t * [boardXBoxes];
	for (int i = 0; i < boardXBoxes; ++i) {
		board[i] = new uint8_t[boardYBoxes];
	}
	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {

			board[x][y] = 0;
		}
	}
	boardState->setBoard(board);
	draggingPiece = false;
	draggingPieceBox.x = -1;
	draggingPieceBox.y = -1;
	loadBoardFromFen(STARTING_FEN, boardState);
	initializePieceLocations(boardState);
	initializeKingsThreatened(boardState);


	legalMoves = calculateLegalMoves(boardState);


	highlightKingBox.x = -1;
	highlightKingBox.y = -1;
	winnerKing.x = winnerKing.y = -1;
	gameOver = false;
	moveStack.init();


}

Board::~Board() {
	Piece::destroyImages();


	delete(boardState);
	moveStack.clear();
}

void Board::render(BoardState* currentBoardState) {
	renderBoard();
	if (draggingPiece) {
		renderHighlightMoves();
	}
	else if (moveStack.getSize() != 0) {
		renderPreviousMove();
	}
	if (highlightKingBox.x != -1 || winnerKing.x != -1) {
		renderKingBox();
	}

	//renderAttackedSquares();


	renderPieces(currentBoardState);
	if (draggingPiece) {
		renderDraggedPiece();
		//we only want to render previous moves if we're not rendering a pieces
	}

	if (waitingForPromotionChoice) {
		renderPromotionOptions();
	}

}
void Board::renderPreviousMove() {
	StoreMove* prevMoveHistory = moveStack.getTop();
	Move prevMove = prevMoveHistory->getMove();

	renderBox(prevMove.fromBox, LAST_MOVE_COLOR);
	renderBox(prevMove.toBox, LAST_MOVE_COLOR);
}

void Board::renderBox(Box box, SDL_Color color) {

	SDL_Rect highlightRect;
	highlightRect.w = boxXWidth;
	highlightRect.h = boxYHeight;
	SDL_Color drawColor = color;
	SDL_SetRenderDrawColor(Window::renderer, color.r, color.g, color.b, color.a);

	highlightRect.x = Game::boardTopLeftX + box.x * boxXWidth;
	highlightRect.y = Game::boardTopLeftY + box.y * boxYHeight;
	SDL_RenderFillRect(Window::renderer, &highlightRect);

}

void Board::renderPromotionOptions() {
	SDL_Rect renderRect;
	renderRect.w = boxXWidth * 2;
	renderRect.h = boxYHeight * 2;
	renderRect.x = Game::boardTopLeftX;
	renderRect.y = Game::boardTopLeftY + (getHeight() - renderRect.h) / 2;
	int w, h;
	SDL_Rect fromRect;
	fromRect.x = fromRect.y = 0;
	if (boardState->getCurrentTurn() == 'w') {
		SDL_QueryTexture(Piece::whiteQueenTexture, NULL, NULL, &w, &h);
		fromRect.w = w;
		fromRect.h = h;
		SDL_RenderCopy(Window::renderer, Piece::whiteQueenTexture, &fromRect, &renderRect);
		renderRect.x += renderRect.w;

		SDL_QueryTexture(Piece::whiteRookTexture, NULL, NULL, &w, &h);
		fromRect.w = w;
		fromRect.h = h;
		SDL_RenderCopy(Window::renderer, Piece::whiteRookTexture, &fromRect, &renderRect);
		renderRect.x += renderRect.w;

		SDL_QueryTexture(Piece::whiteBishopTexture, NULL, NULL, &w, &h);
		fromRect.w = w;
		fromRect.h = h;
		SDL_RenderCopy(Window::renderer, Piece::whiteBishopTexture, &fromRect, &renderRect);
		renderRect.x += renderRect.w;

		SDL_QueryTexture(Piece::whiteKnightTexture, NULL, NULL, &w, &h);
		fromRect.w = w;
		fromRect.h = h;
		SDL_RenderCopy(Window::renderer, Piece::whiteKnightTexture, &fromRect, &renderRect);


	}
	else {
		SDL_QueryTexture(Piece::blackQueenTexture, NULL, NULL, &w, &h);
		fromRect.w = w;
		fromRect.h = h;
		SDL_RenderCopy(Window::renderer, Piece::blackQueenTexture, &fromRect, &renderRect);
		renderRect.x += renderRect.w;

		SDL_QueryTexture(Piece::blackRookTexture, NULL, NULL, &w, &h);
		fromRect.w = w;
		fromRect.h = h;
		SDL_RenderCopy(Window::renderer, Piece::blackRookTexture, &fromRect, &renderRect);
		renderRect.x += renderRect.w;

		SDL_QueryTexture(Piece::blackBishopTexture, NULL, NULL, &w, &h);
		fromRect.w = w;
		fromRect.h = h;
		SDL_RenderCopy(Window::renderer, Piece::blackBishopTexture, &fromRect, &renderRect);
		renderRect.x += renderRect.w;

		SDL_QueryTexture(Piece::blackKnightTexture, NULL, NULL, &w, &h);
		fromRect.w = w;
		fromRect.h = h;
		SDL_RenderCopy(Window::renderer, Piece::blackKnightTexture, &fromRect, &renderRect);

	}


}

void Board::togglePromotionOptions() {
	std::cout << "toggling" << std::endl;
	waitingForPromotionChoice = !waitingForPromotionChoice;
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

	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {

			SDL_Color currentColor = (x + y) % 2 == 0 ? boardColor1 : boardColor2;
			renderBox({ x,y }, currentColor);



		}
	}
}
void Board::renderAttackedSquares() {
	for (int x = 0; x < boardXBoxes; x++) {
		for (int y = 0; y < boardYBoxes; y++) {
			if (squareAttacked({ x,y }, boardState)) {
				renderBox({ x,y }, DANGER_COLOR);
			}
		}
	}
}


void Board::renderPieces(BoardState* currentBoardState) {
	/*
	uint8_t** board = currentBoardState->getBoard();
	for (int x = 0; x < boardXBoxes; ++x) {
		for (int y = 0; y < boardYBoxes; ++y) {
			if (board[x][y] != 0) {

				if (x != draggingPieceBox.x || y != draggingPieceBox.y) {
					//std::cout << " rendering: " << int(board[x][y]) << std::endl;
					renderPiece(x, y, currentBoardState);
				}
			}
		}
	}*/
	if (draggingPiece) {
		if (currentBoardState->getBoard()[draggingPieceBox.x][draggingPieceBox.y] != (Piece::white | Piece::king)) {
			renderPiece(whiteLocations.getKingLocation(), currentBoardState);
		}
		if (currentBoardState->getBoard()[draggingPieceBox.x][draggingPieceBox.y] != (Piece::black | Piece::king)) {
			renderPiece(blackLocations.getKingLocation(), currentBoardState);
		}
	}
	else {
		renderPiece(whiteLocations.getKingLocation(), currentBoardState);
		renderPiece(blackLocations.getKingLocation(), currentBoardState);
	}



	for (int i = 0; i < whiteLocations.getBishopLocations().size(); i++) {
		renderPiece(whiteLocations.getBishopLocations().at(i), boardState);
	}
	for (int i = 0; i < whiteLocations.getQueenLocations().size(); i++) {
		renderPiece(whiteLocations.getQueenLocations().at(i), boardState);
	}
	for (int i = 0; i < whiteLocations.getPawnLocations().size(); i++) {
		renderPiece(whiteLocations.getPawnLocations().at(i), boardState);
	}
	for (int i = 0; i < whiteLocations.getRookLocations().size(); i++) {
		renderPiece(whiteLocations.getRookLocations().at(i), boardState);
	}
	for (int i = 0; i < whiteLocations.getKnightLocations().size(); i++) {
		renderPiece(whiteLocations.getKnightLocations().at(i), boardState);
	}

	for (int i = 0; i < blackLocations.getBishopLocations().size(); i++) {
		renderPiece(blackLocations.getBishopLocations().at(i), boardState);
	}
	for (int i = 0; i < blackLocations.getQueenLocations().size(); i++) {
		renderPiece(blackLocations.getQueenLocations().at(i), boardState);
	}
	for (int i = 0; i < blackLocations.getPawnLocations().size(); i++) {
		renderPiece(blackLocations.getPawnLocations().at(i), boardState);
	}
	for (int i = 0; i < blackLocations.getRookLocations().size(); i++) {
		renderPiece(blackLocations.getRookLocations().at(i), boardState);
	}
	for (int i = 0; i < blackLocations.getKnightLocations().size(); i++) {
		renderPiece(blackLocations.getKnightLocations().at(i), boardState);
	}


}

void Board::renderPiece(Box pieceBox, BoardState* currentBoardState) {
	uint8_t currentPiece = currentBoardState->getBoard()[pieceBox.x][pieceBox.y];
	SDL_Texture* currentTexture = getTextureAtLocation(pieceBox.x, pieceBox.y, currentBoardState);
	renderPieceTexture(currentTexture, pieceBox.x, pieceBox.y);
}

SDL_Texture* Board::getTextureAtLocation(int x, int y, BoardState* currentBoardState) {
	uint8_t currentPiece = currentBoardState->getBoard()[x][y];
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

	uint8_t** board = currentBoardState->getBoard();
	for (int rank = 0; rank < boardYBoxes; rank++) {
		column = 0;
		while (fenNotation[index] != '/' && fenNotation[index] != ' ') {
			if (isdigit(fenNotation[index])) {


				column += (fenNotation[index] - '0');
				index++;
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
			if (waitingForPromotionChoice) {
				tryPickingPromotionPiece(boardX, boardY, currentBoardState);
			}
			else {
				if (!draggingPiece) {
					attemptPickupPiece(boardX, boardY, currentBoardState);
				}
				else {

					attemptPlacePiece(boardX, boardY, currentBoardState);
					///	std::cout << "Hereis the current white king threated info: " << int(whiteThreatened.threatenedInfo) << std::endl;
						//std::cout << "Hereis the current black king threated info: " << int(blackThreatened.threatenedInfo) << std::endl;

				}
			}
		}

	}
	else if (b.button == SDL_BUTTON_RIGHT) {
		if (draggingPiece) {
			if (currentBoardState->getCurrentTurn() == 'w') {
				whiteLocations.addPiece(currentBoardState->getBoard()[draggingPieceBox.x][draggingPieceBox.y], draggingPieceBox);
			}
			else {
				blackLocations.addPiece(currentBoardState->getBoard()[draggingPieceBox.x][draggingPieceBox.y], draggingPieceBox);
			}
			stopDraggingPiece();
		}


	}
}


void Board::tryPickingPromotionPiece(int x, int y, BoardState* currentBoardState) {
	if (y == 3 || y == 4) {
		switch (x / 2) {
		case 0:
			promoteQueen(currentBoardState);
			break;
		case 1:
			promoteRook(currentBoardState);
			break;
		case 2:
			promoteBishop(currentBoardState);
			break;

		case 3:
			promoteKnight(currentBoardState);
			break;
		}
		waitingForPromotionChoice = false;
		nextTurn(currentBoardState);

	}
}

void Board::promoteQueen(BoardState* currentBoardState) {
	makeMove({ {promotionMove.fromBox.x,promotionMove.fromBox.y},
		{promotionMove.toBox.x,promotionMove.toBox.y},
		false,false,false,true,'q' }, currentBoardState);

}
void Board::promoteRook(BoardState* currentBoardState) {
	makeMove({ promotionMove.fromBox.x,promotionMove.fromBox.y,
		promotionMove.toBox.x,promotionMove.toBox.y,
		false,false,false,true,'r' }, currentBoardState);
}
void Board::promoteKnight(BoardState* currentBoardState) {
	makeMove({ {promotionMove.fromBox.x,promotionMove.fromBox.y},
		{promotionMove.toBox.x,promotionMove.toBox.y },
		false,false,false,true,'n' }, currentBoardState);
}
void Board::promoteBishop(BoardState* currentBoardState) {
	makeMove({ {promotionMove.fromBox.x,promotionMove.fromBox.y },
		{promotionMove.toBox.x,promotionMove.toBox.y },
		false,false,false,true,'b' }, currentBoardState);
}


void Board::attemptPickupPiece(int x, int y, BoardState* currentBoardState) {

	//if there's a piece on the board space we're clicking.
	if (currentBoardState->getBoard()[x][y] != 0) {
		//if it's the piece for the curernt players turn.

		if (pieceIsCurrentPlayersPiece(x, y, currentBoardState)) {
			draggingPiece = true;
			draggingPieceBox.x = x;
			draggingPieceBox.y = y;
			draggingPieceTexture = getTextureAtLocation(x, y, currentBoardState);
			createHighlightMoves(x, y);
			if (currentBoardState->getCurrentTurn() == 'w') {
				whiteLocations.removePiece(currentBoardState->getBoard()[x][y], { x,y });
			}
			else {
				blackLocations.removePiece(currentBoardState->getBoard()[x][y], { x,y });
			}

		}

	}
}

void Board::attemptPlacePiece(int x, int y, BoardState* currentBoardState) {


	Move newMove = { draggingPieceBox.x,draggingPieceBox.y,x,y };
	uint8_t** board = currentBoardState->getBoard();
	int enPassantX = currentBoardState->getEnPassantX();
	int enPassantY = currentBoardState->getEnPassantY();

	//inLegalMoves passes in a reference so we update the castling and sturff here.
	if (inLegalMoves(newMove)) {
		if (currentBoardState->getCurrentTurn() == 'w') {
			whiteLocations.addPiece(currentBoardState->getBoard()[draggingPieceBox.x][draggingPieceBox.y], draggingPieceBox);
		}
		else {
			blackLocations.addPiece(currentBoardState->getBoard()[draggingPieceBox.x][draggingPieceBox.y], draggingPieceBox);
		}
		if (newMove.isPromotion) {
			promotionMove = {
				{
					newMove.fromBox.x,
					newMove.fromBox.y
				},
				{
					newMove.toBox.x,
					newMove.toBox.y
				},
				false,
				false,
				false,
				true,
				newMove.promotionType
			};

			waitingForPromotionChoice = true;
		}
		else {

			makeMove(newMove, currentBoardState);

			nextTurn(currentBoardState);
		}
		stopDraggingPiece();

	}
	else {
		if (currentBoardState->getCurrentTurn() == 'w') {
			whiteLocations.addPiece(currentBoardState->getBoard()[draggingPieceBox.x][draggingPieceBox.y], draggingPieceBox);
		}
		else {
			blackLocations.addPiece(currentBoardState->getBoard()[draggingPieceBox.x][draggingPieceBox.y], draggingPieceBox);
		}
		stopDraggingPiece();
	}

}

void Board::nextTurn(BoardState* currentBoardState) {

	legalMoves = calculateLegalMoves(currentBoardState);

	if (isGameOver(currentBoardState) == 1) {
		gameOver = true;

	}
	updateHighlightKingBox();
}

void Board::switchTurns(BoardState* currentBoardState) {
	currentBoardState->setCurrentTurn((currentBoardState->getCurrentTurn() == 'w') ? 'b' : 'w');


}

bool Board::inLegalMoves(struct Move& newMove) {

	for (int i = 0; i < legalMoves.size(); i++) {

		if (
			newMove.toBox.x == legalMoves.at(i).toBox.x &&
			newMove.toBox.y == legalMoves.at(i).toBox.y &&
			newMove.fromBox.x == legalMoves.at(i).fromBox.x &&
			newMove.fromBox.y == legalMoves.at(i).fromBox.y
			) {

			newMove.kingSideCastle = legalMoves.at(i).kingSideCastle;
			newMove.queenSideCastle = legalMoves.at(i).queenSideCastle;
			newMove.enPassant = legalMoves.at(i).enPassant;
			newMove.isPromotion = legalMoves.at(i).isPromotion;
			newMove.promotionType = legalMoves.at(i).promotionType;

			return true;
		}
	}
	return false;
}

bool Board::inPseudoMoves(struct Move& newMove) {

	for (int i = 0; i < pseudoLegalMoves.size(); i++) {

		if (newMove.fromBox.x == pseudoLegalMoves.at(i).fromBox.y &&
			newMove.fromBox.y == pseudoLegalMoves.at(i).fromBox.y &&
			newMove.toBox.x == pseudoLegalMoves.at(i).toBox.x &&
			newMove.toBox.y == pseudoLegalMoves.at(i).toBox.y) {

			newMove.kingSideCastle = pseudoLegalMoves.at(i).kingSideCastle;
			newMove.queenSideCastle = pseudoLegalMoves.at(i).queenSideCastle;
			newMove.enPassant = pseudoLegalMoves.at(i).enPassant;
			newMove.isPromotion = pseudoLegalMoves.at(i).isPromotion;
			newMove.promotionType = pseudoLegalMoves.at(i).promotionType;

			return true;
		}
	}
	return false;
}

void Board::stopDraggingPiece() {
	draggingPiece = false;
	draggingPieceBox.x = draggingPieceBox.y = -1;
}

void Board::clearMoves() {
	pseudoLegalMoves.clear();
	legalMoves.clear();
}

std::vector<Move> Board::calculatePseudoLegalMoves(BoardState* currentBoardState) {
	std::vector<Move> returnVec;
	if (currentBoardState->getCurrentTurn() == 'w') {
		calculateKingMoves(whiteLocations.getKingLocation(), currentBoardState, returnVec);
		std::vector<Box> moves = whiteLocations.getBishopLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculateBishopMoves(moves.at(i), currentBoardState, returnVec);
		}
		moves = whiteLocations.getRookLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculateRookMoves(moves.at(i), currentBoardState, returnVec);
		}
		moves = whiteLocations.getQueenLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculateQueenMoves(moves.at(i), currentBoardState, returnVec);
		}
		moves = whiteLocations.getKnightLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculateKnightMoves(moves.at(i), currentBoardState, returnVec);
		}
		moves = whiteLocations.getPawnLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculatePawnUpMoves(moves.at(i), currentBoardState, returnVec);
		}

	}
	else {
		calculateKingMoves(blackLocations.getKingLocation(), currentBoardState, returnVec);
		std::vector<Box> moves = blackLocations.getBishopLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculateBishopMoves(moves.at(i), currentBoardState, returnVec);
		}
		moves = blackLocations.getRookLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculateRookMoves(moves.at(i), currentBoardState, returnVec);
		}
		moves = blackLocations.getQueenLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculateQueenMoves(moves.at(i), currentBoardState, returnVec);
		}
		moves = blackLocations.getKnightLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculateKnightMoves(moves.at(i), currentBoardState, returnVec);
		}
		moves = blackLocations.getPawnLocations();
		for (int i = 0; i < moves.size(); i++) {
			calculatePawnDownMoves(moves.at(i), currentBoardState, returnVec);
		}
	}




	return returnVec;
}

std::vector<Move> Board::calculateLegalMoves(BoardState* currentBoardState) {

	std::vector<Move> currentLegal;
	//std::cout << "Calling this once" << std::endl;

	pseudoLegalMoves = calculatePseudoLegalMoves(currentBoardState);
	char currentTurn = currentBoardState->getCurrentTurn();
	Box kingLocation;

	if (currentTurn == 'w') {

		kingLocation = whiteLocations.getKingLocation();
		calculateKingLegalMoves(kingLocation, currentBoardState, currentLegal, whiteThreatened);
		if (whiteThreatened.amountAttacked != 2) {
			for (int i = 0; i < whiteLocations.getRookLocations().size(); i++) {
				calculateRookLegalMoves(whiteLocations.getRookLocations().at(i), kingLocation, currentBoardState, currentLegal, whiteThreatened);
			}
			for (int i = 0; i < whiteLocations.getKnightLocations().size(); i++) {
				calculateKnightLegalMoves(whiteLocations.getKnightLocations().at(i), kingLocation, currentBoardState, currentLegal, whiteThreatened);
			}
			for (int i = 0; i < whiteLocations.getQueenLocations().size(); i++) {
				calculateQueenLegalMoves(whiteLocations.getQueenLocations().at(i), kingLocation, currentBoardState, currentLegal, whiteThreatened);
			}
			for (int i = 0; i < whiteLocations.getPawnLocations().size(); i++) {
				calculatePawnLegalMoves(whiteLocations.getPawnLocations().at(i), kingLocation, currentBoardState, currentLegal, whiteThreatened);
			}
			for (int i = 0; i < whiteLocations.getBishopLocations().size(); i++) {
				calculateBishopLegalMoves(whiteLocations.getBishopLocations().at(i), kingLocation, currentBoardState, currentLegal, whiteThreatened);
			}
		}

	}
	else {

		kingLocation = blackLocations.getKingLocation();
		calculateKingLegalMoves(kingLocation, currentBoardState, currentLegal, blackThreatened);
		if (blackThreatened.amountAttacked != 2) {
			for (int i = 0; i < blackLocations.getRookLocations().size(); i++) {
				calculateRookLegalMoves(blackLocations.getRookLocations().at(i), kingLocation, currentBoardState, currentLegal, blackThreatened);
			}
			for (int i = 0; i < blackLocations.getKnightLocations().size(); i++) {
				calculateKnightLegalMoves(blackLocations.getKnightLocations().at(i), kingLocation, currentBoardState, currentLegal, blackThreatened);
			}
			for (int i = 0; i < blackLocations.getQueenLocations().size(); i++) {
				calculateQueenLegalMoves(blackLocations.getQueenLocations().at(i), kingLocation, currentBoardState, currentLegal, blackThreatened);
			}
			for (int i = 0; i < blackLocations.getPawnLocations().size(); i++) {
				calculatePawnLegalMoves(blackLocations.getPawnLocations().at(i), kingLocation, currentBoardState, currentLegal, blackThreatened);
			}
			for (int i = 0; i < blackLocations.getBishopLocations().size(); i++) {
				calculateBishopLegalMoves(blackLocations.getBishopLocations().at(i), kingLocation, currentBoardState, currentLegal, blackThreatened);
			}
		}

	}

	/*
	for (int i = 0; i < pseudoLegalMoves.size(); i++) {



		//the king can't castle if it's in check
		if (kingInCheck(currentBoardState)) {
			if (pseudoLegalMoves.at(i).kingSideCastle) {
				continue;
			}
			else if (pseudoLegalMoves.at(i).queenSideCastle) {
				continue;
			}
		}
		//the king can't castle through check.
		if (pseudoLegalMoves.at(i).kingSideCastle || pseudoLegalMoves.at(i).queenSideCastle) {
			if (squareAttacked((pseudoLegalMoves.at(i).fromBox.x + pseudoLegalMoves.at(i).toBox.x) / 2, pseudoLegalMoves.at(i).fromBox.y, currentBoardState)) {
				continue;
			}
		}
		//there must be clear spaces between the rook and king to be able to castle.
		bool inValid = false;
		if (pseudoLegalMoves.at(i).kingSideCastle) {
			for (int x = pseudoLegalMoves.at(i).fromBox.x + 1; x < boardXBoxes - 1; x++) {
				if (currentBoardState->getBoard()[x][pseudoLegalMoves.at(i).fromBox.y] != 0) {
					inValid = true;
					break;
				}
			}
		}
		else if (pseudoLegalMoves.at(i).queenSideCastle) {
			for (int x = 1; x < pseudoLegalMoves.at(i).fromBox.x; x++) {
				if (currentBoardState->getBoard()[x][pseudoLegalMoves.at(i).fromBox.y] != 0) {
					inValid = true;
					break;
				}
			}
		}
		if (inValid) {
			continue;
		}



		makeMove(pseudoLegalMoves.at(i), currentBoardState);
		//make move switches the board state, but we wanna check the current player
		switchTurns(currentBoardState);

		if (!kingInCheck(currentBoardState)) {

			//because kingInCheck changed the vector... but it's not working


			currentLegal.push_back({
				{pseudoLegalMoves.at(i).fromBox.x,
				pseudoLegalMoves.at(i).fromBox.y
				},
				{
				pseudoLegalMoves.at(i).toBox.x,
				pseudoLegalMoves.at(i).toBox.y
				},

				pseudoLegalMoves.at(i).kingSideCastle,
				pseudoLegalMoves.at(i).queenSideCastle,
				pseudoLegalMoves.at(i).enPassant,
				pseudoLegalMoves.at(i).isPromotion,
				pseudoLegalMoves.at(i).promotionType }
			);

		}
		switchTurns(currentBoardState);

		unMakeMove(currentBoardState);



	}
	*/
	return currentLegal;
}

void Board::makeMove(struct Move move, BoardState* currentBoardState) {

	StoreMove* newStore = new StoreMove();


	uint8_t** board = currentBoardState->getBoard();
	int enPassantX = currentBoardState->getEnPassantX();
	int enPassantY = currentBoardState->getEnPassantY();
	char turn = currentBoardState->getCurrentTurn();
	newStore->setPreviousEnPassant({ enPassantX,enPassantY });
	if (isEnPassant(move.fromBox, move.toBox, currentBoardState)) {

		//the piece removed depends on the turn.
		if (turn == 'w') {
			board[enPassantX][enPassantY + 1] = 0;
			blackLocations.removePawn({ enPassantX,enPassantY + 1 });

		}
		else {
			board[enPassantX][enPassantY - 1] = 0;
			whiteLocations.removePawn({ enPassantX,enPassantY - 1 });
		}


	}

	if (move.kingSideCastle) {

		board[move.toBox.x - 1][move.toBox.y] = board[move.toBox.x + 1][move.toBox.y];
		board[move.toBox.x + 1][move.toBox.y] = 0;
		if (turn == 'w') {
			whiteLocations.updateRook({ move.toBox.x + 1,move.toBox.y }, { move.toBox.x - 1,move.toBox.y });
		}
		else {
			blackLocations.updateRook({ move.toBox.x + 1,move.toBox.y }, { move.toBox.x - 1,move.toBox.y });
		}

	}
	else if (move.queenSideCastle) {

		board[move.toBox.x + 1][move.toBox.y] = board[move.toBox.x - 2][move.toBox.y];
		board[move.toBox.x - 2][move.toBox.y] = 0;

		if (turn == 'w') {
			whiteLocations.updateRook({ move.toBox.x - 2 ,move.toBox.y }, { move.toBox.x + 1,move.toBox.y });
		}
		else {
			blackLocations.updateRook({ move.toBox.x - 2,move.toBox.y }, { move.toBox.x + 1,move.toBox.y });
		}
	}

	updateEnPassant(move.fromBox, move.toBox, currentBoardState);
	newStore->setPreviousCastles(currentBoardState->getWhiteCanKingsideCastle(),
		currentBoardState->getWhiteCanQueensideCastle(),
		currentBoardState->getBlackCanKingsideCastle(),
		currentBoardState->getBlackCanQueensideCastle());
	updateCastling(move.fromBox, move.toBox, currentBoardState);



	if (board[move.toBox.x][move.toBox.y] != 0) {
		newStore->setPieceTaken(board[move.toBox.x][move.toBox.y]);
		newStore->setTookPiece(true);
		if (turn == 'w') {
			blackLocations.removePiece(board[move.toBox.x][move.toBox.y], { move.toBox });
		}
		else {
			whiteLocations.removePiece(board[move.toBox.x][move.toBox.y], { move.toBox });
		}
	}
	else {
		newStore->setTookPiece(false);
	}
	if (move.isPromotion) {
		switch (move.promotionType) {
		case 'q':
			if (turn == 'w') {
				board[move.toBox.x][move.toBox.y] = Piece::white | Piece::queen;
				whiteLocations.getQueenLocations().push_back({ move.toBox });
				whiteLocations.removePawn(move.fromBox);
			}
			else {
				board[move.toBox.x][move.toBox.y] = Piece::black | Piece::queen;
				blackLocations.getQueenLocations().push_back({ move.toBox });
				blackLocations.removePawn(move.fromBox);
			}
			break;
		case 'r':
			if (turn == 'w') {
				board[move.toBox.x][move.toBox.y] = Piece::white | Piece::rook;
				whiteLocations.getRookLocations().push_back({ move.toBox });
				whiteLocations.removePawn(move.fromBox);
			}
			else {
				board[move.toBox.x][move.toBox.y] = Piece::black | Piece::rook;
				blackLocations.getRookLocations().push_back({ move.toBox });
				blackLocations.removePawn(move.fromBox);
			}
			break;
		case 'n':
			if (turn == 'w') {
				board[move.toBox.x][move.toBox.y] = Piece::white | Piece::knight;
				whiteLocations.getKnightLocations().push_back({ move.toBox });
				whiteLocations.removePawn(move.fromBox);
			}
			else {
				board[move.toBox.x][move.toBox.y] = Piece::black | Piece::knight;
				blackLocations.getKnightLocations().push_back({ move.toBox });
				blackLocations.removePawn(move.fromBox);
			}
			break;

		case 'b':
			if (turn == 'w') {
				board[move.toBox.x][move.toBox.y] = Piece::white | Piece::bishop;
				whiteLocations.getBishopLocations().push_back({ move.toBox });
				whiteLocations.removePawn(move.fromBox);
			}
			else {
				board[move.toBox.x][move.toBox.y] = Piece::black | Piece::bishop;
				blackLocations.getBishopLocations().push_back({ move.toBox });
				blackLocations.removePawn(move.fromBox);
			}
			break;
		}
	}
	else {

		board[move.toBox.x][move.toBox.y] = board[move.fromBox.x][move.fromBox.y];
		if (turn == 'w') {
			whiteLocations.updatePiece(board[move.toBox.x][move.toBox.y], move.fromBox, move.toBox);
		}
		else {
			blackLocations.updatePiece(board[move.toBox.x][move.toBox.y], move.fromBox, move.toBox);
		}
	}

	board[move.fromBox.x][move.fromBox.y] = 0;

	newStore->setMove(move);

	if (turn == 'w') {
		newStore->setThreatInfo(whiteThreatened.threatenedInfo, blackThreatened.threatenedInfo,
			whiteThreatened.attackedInfo, blackThreatened.attackedInfo, whiteThreatened.amountAttacked,whiteThreatened.attackedByKnight,whiteThreatened.attackedFromBox);
	}
	else {
		newStore->setThreatInfo(whiteThreatened.threatenedInfo, blackThreatened.threatenedInfo,
			whiteThreatened.attackedInfo, blackThreatened.attackedInfo, blackThreatened.amountAttacked,blackThreatened.attackedByKnight,blackThreatened.attackedFromBox);
	}
	newStore->setThreatBoxes(whiteThreatened.straightLeftBox, whiteThreatened.upLeftBox, whiteThreatened.straightUpBox,
		whiteThreatened.upRightBox, whiteThreatened.straightRightBox, whiteThreatened.downRightBox, whiteThreatened.straightDownBox,
		whiteThreatened.downLeftBox,

		blackThreatened.straightLeftBox, blackThreatened.upLeftBox, blackThreatened.straightUpBox,
		blackThreatened.upRightBox, blackThreatened.straightRightBox, blackThreatened.downRightBox, blackThreatened.straightDownBox,
		blackThreatened.downLeftBox
	);


	moveStack.push(newStore);

	//clear attack info because by virtue of how moves work, after a move you shouldn't be in check anymore.
	if (turn == 'w') {
		whiteThreatened.attackedInfo = 0;
		whiteThreatened.amountAttacked = 0;
	}
	else {
		blackThreatened.attackedInfo = 0;
		blackThreatened.amountAttacked = 0;
	}


	updateThreats(move, currentBoardState); //if we moved to a thing related to the king

	//we might be removing a threat.
	switchTurns(currentBoardState);

	updateThreats(move, currentBoardState);

}





void Board::unMakeMove(BoardState* currentBoardState) {

	if (moveStack.getSize() == 0) {
		std::cout << "There are no moves to unmake" << std::endl;
		return;
	}
	switchTurns(currentBoardState);


	uint8_t** board = currentBoardState->getBoard();
	StoreMove* previousMove = moveStack.pop();
	Move move = previousMove->getMove();
	char currentTurn = currentBoardState->getCurrentTurn();
	//if it's a promotion we move the piece back and transform it into a pawn.
	if (move.isPromotion) {
		if (currentTurn == 'w') {
			board[move.fromBox.x][move.fromBox.y] = Piece::white | Piece::pawn;
			whiteLocations.removePiece(board[move.toBox.x][move.toBox.y], move.toBox);
			whiteLocations.getPawnLocations().push_back(move.fromBox);
		}
		else {
			board[move.fromBox.x][move.fromBox.y] = Piece::black | Piece::pawn;
			blackLocations.removePiece(board[move.toBox.x][move.toBox.y], move.toBox);
			blackLocations.getPawnLocations().push_back(move.fromBox);
		}

	}
	//otherwise we can just put the piece back normally
	else {

		board[move.fromBox.x][move.fromBox.y] = board[move.toBox.x][move.toBox.y];
		if (currentTurn == 'w') {
			whiteLocations.updatePiece(board[move.fromBox.x][move.fromBox.y], move.toBox, move.fromBox);
		}
		else {
			blackLocations.updatePiece(board[move.fromBox.x][move.fromBox.y], move.toBox, move.fromBox);
		}
	}
	//if a piece was taken, we should put it back like good people.
	if (previousMove->tookPiece()) {

		board[move.toBox.x][move.toBox.y] = previousMove->getTakenPiece();
		if (currentTurn == 'w') {
			blackLocations.addPiece(previousMove->getTakenPiece(), move.toBox);
		}
		else {
			whiteLocations.addPiece(previousMove->getTakenPiece(), move.toBox);
		}
	}
	else {
		board[move.toBox.x][move.toBox.y] = 0;
	}

	currentBoardState->setBlackCanKingsideCastle(previousMove->getPreviousBlackKingSide());
	currentBoardState->setWhiteCanKingsideCastle(previousMove->getPreviousWhiteKingSide());
	currentBoardState->setWhiteCanQueensideCastle(previousMove->getPreviousWhiteQueenSide());
	currentBoardState->setBlackCanQueensideCastle(previousMove->getPreviousBlackQueenSide());
	Box enPassant = previousMove->getPreviousEnPassant();
	currentBoardState->setEnPassantX(enPassant.x);
	currentBoardState->setEnPassantY(enPassant.y);


	if (currentTurn == 'w') {
		whiteThreatened = {
			previousMove->getPreviousWhiteThreatenedInfo(),
			previousMove->getPreviousWhiteAttackedInfo(),
			{previousMove->getPreviousWhiteStraightLeftBox().x,previousMove->getPreviousWhiteStraightLeftBox().y},
			{previousMove->getPreviousWhiteUpLeftBox().x,previousMove->getPreviousWhiteUpLeftBox().y},
			{previousMove->getPreviousWhiteStraightUpBox().x,previousMove->getPreviousWhiteStraightUpBox().y},
			{previousMove->getPreviousWhiteUpRightBox().x,previousMove->getPreviousWhiteUpRightBox().y},

			{previousMove->getPreviousWhiteStraightRightBox().x,previousMove->getPreviousWhiteStraightRightBox().y},
			{previousMove->getPreviousWhiteDownRightBox().x,previousMove->getPreviousWhiteDownRightBox().y},
			{previousMove->getPreviousWhiteStraightDownBox().x,previousMove->getPreviousWhiteStraightDownBox().y},
			{previousMove->getPreviousWhiteDownLeftBox().x,previousMove->getPreviousWhiteDownLeftBox().y},
			previousMove->getPreviousAmountAttacked(),{previousMove->getPreviousAttackedFromBox().x,previousMove->getPreviousAttackedFromBox().y},previousMove->getPreviousAttackedByKnight()

		};
		blackThreatened = {
			previousMove->getPreviousBlackThreatenedInfo(),
			previousMove->getPreviousBlackAttackedInfo(),
			{previousMove->getPreviousBlackStraightLeftBox().x,previousMove->getPreviousBlackStraightLeftBox().y},
			{previousMove->getPreviousBlackUpLeftBox().x,previousMove->getPreviousBlackUpLeftBox().y},
			{previousMove->getPreviousBlackStraightUpBox().x,previousMove->getPreviousBlackStraightUpBox().y},
			{previousMove->getPreviousBlackUpRightBox().x,previousMove->getPreviousBlackUpRightBox().y},

			{previousMove->getPreviousBlackStraightRightBox().x,previousMove->getPreviousBlackStraightRightBox().y},
			{previousMove->getPreviousBlackDownRightBox().x,previousMove->getPreviousBlackDownRightBox().y},
			{previousMove->getPreviousBlackStraightDownBox().x,previousMove->getPreviousBlackStraightDownBox().y},
			{previousMove->getPreviousBlackDownLeftBox().x,previousMove->getPreviousBlackDownLeftBox().y},
			0,{-1,-1},false

		};
	}
	else {
		whiteThreatened = {
			previousMove->getPreviousWhiteThreatenedInfo(),
			previousMove->getPreviousWhiteAttackedInfo(),
			{previousMove->getPreviousWhiteStraightLeftBox().x,previousMove->getPreviousWhiteStraightLeftBox().y},
			{previousMove->getPreviousWhiteUpLeftBox().x,previousMove->getPreviousWhiteUpLeftBox().y},
			{previousMove->getPreviousWhiteStraightUpBox().x,previousMove->getPreviousWhiteStraightUpBox().y},
			{previousMove->getPreviousWhiteUpRightBox().x,previousMove->getPreviousWhiteUpRightBox().y},

			{previousMove->getPreviousWhiteStraightRightBox().x,previousMove->getPreviousWhiteStraightRightBox().y},
			{previousMove->getPreviousWhiteDownRightBox().x,previousMove->getPreviousWhiteDownRightBox().y},
			{previousMove->getPreviousWhiteStraightDownBox().x,previousMove->getPreviousWhiteStraightDownBox().y},
			{previousMove->getPreviousWhiteDownLeftBox().x,previousMove->getPreviousWhiteDownLeftBox().y},
			0,{-1,-1},false

		};
		blackThreatened = {
			previousMove->getPreviousBlackThreatenedInfo(),
			previousMove->getPreviousBlackAttackedInfo(),
			{previousMove->getPreviousBlackStraightLeftBox().x,previousMove->getPreviousBlackStraightLeftBox().y},
			{previousMove->getPreviousBlackUpLeftBox().x,previousMove->getPreviousBlackUpLeftBox().y},
			{previousMove->getPreviousBlackStraightUpBox().x,previousMove->getPreviousBlackStraightUpBox().y},
			{previousMove->getPreviousBlackUpRightBox().x,previousMove->getPreviousBlackUpRightBox().y},

			{previousMove->getPreviousBlackStraightRightBox().x,previousMove->getPreviousBlackStraightRightBox().y},
			{previousMove->getPreviousBlackDownRightBox().x,previousMove->getPreviousBlackDownRightBox().y},
			{previousMove->getPreviousBlackStraightDownBox().x,previousMove->getPreviousBlackStraightDownBox().y},
			{previousMove->getPreviousBlackDownLeftBox().x,previousMove->getPreviousBlackDownLeftBox().y},
			previousMove->getPreviousAmountAttacked(), {previousMove->getPreviousAttackedFromBox().x,previousMove->getPreviousAttackedFromBox().y},previousMove->getPreviousAttackedByKnight()

		};
	}


	//now we deal with castling and en passant stuff.
	if (move.kingSideCastle) {
		board[boardXBoxes - 1][move.fromBox.y] = board[move.fromBox.x + 1][move.fromBox.y];
		board[move.fromBox.x + 1][move.fromBox.y] = 0;
		if (currentTurn == 'w') {
			whiteLocations.updateRook({ move.fromBox.x + 1,move.fromBox.y }, { boardXBoxes - 1,move.fromBox.y });
		}
		else {
			blackLocations.updateRook({ move.fromBox.x + 1,move.fromBox.y }, { boardXBoxes - 1,move.fromBox.y });
		}
	}
	else if (move.queenSideCastle) {
		board[0][move.fromBox.y] = board[move.toBox.x + 1][move.fromBox.y];
		board[move.toBox.x + 1][move.fromBox.y] = 0;
		if (currentTurn == 'w') {
			whiteLocations.updateRook({ move.toBox.x + 1,move.fromBox.y }, { 0,move.fromBox.y });
		}
		else {
			blackLocations.updateRook({ move.toBox.x + 1,move.fromBox.y }, { 0,move.fromBox.y });
		}
	}
	else if (move.enPassant) {

		if (currentTurn == 'w') {
			board[move.toBox.x][move.fromBox.y] = Piece::black | Piece::pawn;
			blackLocations.getPawnLocations().push_back({ move.toBox.x,move.fromBox.y });
		}
		else {
			board[move.toBox.x][move.fromBox.y] = Piece::white | Piece::pawn;
			whiteLocations.getPawnLocations().push_back({ move.toBox.x,move.fromBox.y });
		}
	}
	delete previousMove;




}

void Board::updateAllThreats(char currentTurn, BoardState* currentBoardState) {
	updateStraightDownThreats(currentTurn, currentBoardState);
	updateStraightUpThreats(currentTurn, currentBoardState);
	updateStraightLeftThreats(currentTurn, currentBoardState);
	updateStraightRightThreats(currentTurn, currentBoardState);
	updateUpLeftThreats(currentTurn, currentBoardState);
	updateUpRightThreats(currentTurn, currentBoardState);
	updateDownLeftThreats(currentTurn, currentBoardState);
	updateDownRightThreats(currentTurn, currentBoardState);
}

void Board::updateMoveToThreats(Move lastMove, BoardState* currentBoardState) {
	char currentTurn = currentBoardState->getCurrentTurn();
	Box kingBox;
	if (currentTurn == 'w') {
		kingBox = whiteLocations.getKingLocation();
	}
	else {
		kingBox = blackLocations.getKingLocation();

	}
	uint8_t** board = currentBoardState->getBoard();
	if ((board[lastMove.toBox.x][lastMove.toBox.y] & Piece::king) == Piece::king) {
		updateAllThreats(currentTurn, currentBoardState);
	}
	else {
		//now we look at where the new piece moved TO.
		if (inSameRow(lastMove.toBox, kingBox)) {

			if (lastMove.toBox.x > kingBox.x) {

				updateStraightRightThreats(currentTurn, currentBoardState);
			}
			else {
				updateStraightLeftThreats(currentTurn, currentBoardState);
			}

		}
		else if (inSameCol(lastMove.toBox, kingBox)) {

			if (lastMove.toBox.y > kingBox.y) {
				updateStraightDownThreats(currentTurn, currentBoardState);
			}
			else {
				updateStraightUpThreats(currentTurn, currentBoardState);
			}

		}
		else if (inSameDiagonal(lastMove.toBox, kingBox)) {

			if (lastMove.toBox.x > kingBox.x) {
				updateUpRightThreats(currentTurn, currentBoardState);
			}
			else {
				updateDownLeftThreats(currentTurn, currentBoardState);
			}

		}
		else if (inSameReverseDiagonal(lastMove.toBox, kingBox)) {


			if (lastMove.toBox.x > kingBox.x) {

				updateDownRightThreats(currentTurn, currentBoardState);
			}
			else {

				updateUpLeftThreats(currentTurn, currentBoardState);

			}
		}
	}
	

}

//updates the current players king threats based on the last move by other player. This is after we switch turns. 
void Board::updateThreats(Move lastMove, BoardState* currentBoardState) {
	//see where they moved from first.
	char currentTurn = currentBoardState->getCurrentTurn();
	Box kingBox;
	if (currentTurn == 'w') {
		kingBox = whiteLocations.getKingLocation();
		if ((currentBoardState->getBoard()[lastMove.toBox.x][lastMove.toBox.y]) == (Piece::king | Piece::white)) {
			updateAllThreats(currentTurn, currentBoardState);
			return;
		}

	}
	else {
		kingBox = blackLocations.getKingLocation();
		if ((currentBoardState->getBoard()[lastMove.toBox.x][lastMove.toBox.y]) == (Piece::king | Piece::black)) {
			updateAllThreats(currentTurn, currentBoardState);
			return;
		}
	}
	

	if (currentTurn == 'b') {
		if(currentBoardState->getBoard()[lastMove.toBox.x][lastMove.toBox.y] == (Piece::white | Piece::knight)) {
		
			int xDiff = abs(lastMove.toBox.x - kingBox.x);
			int yDiff = abs(lastMove.toBox.y - kingBox.y);
			int sumOfLeftAndRight = xDiff + yDiff;
			//that means there's a square thats 2-1 between them.
			if (sumOfLeftAndRight == 3 && xDiff != 0 && yDiff != 0) {
				blackThreatened.amountAttacked++;
				blackThreatened.attackedByKnight = true;
				blackThreatened.attackedFromBox = { lastMove.toBox.x,lastMove.toBox.y };
			}
		} //IF THE LAST MOVE WAS A PAWN
		else if (currentBoardState->getBoard()[lastMove.toBox.x][lastMove.toBox.y] == (Piece::white | Piece::pawn)) {
			if (lastMove.toBox.x == kingBox.x + 1){
				//if it's in one of the diagonal rows
				if (lastMove.toBox.y == kingBox.y + 1) {
					blackThreatened.amountAttacked++;
					blackThreatened.threatenedInfo |= KingThreatenedInfo::downRightThreatened;
					blackThreatened.attackedInfo |= KingThreatenedInfo::downRightThreatened;
					blackThreatened.attackedFromBox = { lastMove.toBox.x,lastMove.toBox.y };
				}
			}
			else if (lastMove.toBox.x == kingBox.x - 1) {
				if (lastMove.toBox.y == kingBox.y + 1) {
					blackThreatened.amountAttacked++;
					blackThreatened.threatenedInfo |= KingThreatenedInfo::downLeftThreatened;
					blackThreatened.attackedInfo |= KingThreatenedInfo::downLeftThreatened;
					blackThreatened.attackedFromBox = { lastMove.toBox.x,lastMove.toBox.y };
				}
			}
		}
	}
	else {
		if (currentBoardState->getBoard()[lastMove.toBox.x][lastMove.toBox.y] == (Piece::black | Piece::knight)) {

			int xDiff = abs(lastMove.toBox.x - kingBox.x);
			int yDiff = abs(lastMove.toBox.y - kingBox.y);
			int sumOfLeftAndRight = xDiff + yDiff;
			//that means there's a square thats 2-1 between them.
			if (sumOfLeftAndRight == 3 && xDiff != 0 && yDiff != 0) {
				whiteThreatened.amountAttacked++;
				whiteThreatened.attackedByKnight = true;
				whiteThreatened.attackedFromBox = { lastMove.toBox.x,lastMove.toBox.y };
				
			}
		}
		else if (currentBoardState->getBoard()[lastMove.toBox.x][lastMove.toBox.y] == (Piece::black | Piece::pawn)) {
			if (lastMove.toBox.x == kingBox.x + 1) {
				//if it's in one of the diagonal rows
				if (lastMove.toBox.y == kingBox.y - 1) {
					whiteThreatened.amountAttacked++;
					whiteThreatened.threatenedInfo |= KingThreatenedInfo::upRightThreatened;
					whiteThreatened.attackedInfo |= KingThreatenedInfo::upRightThreatened;
					whiteThreatened.attackedFromBox = { lastMove.toBox.x,lastMove.toBox.y };
				}
			}
			else if (lastMove.toBox.x == kingBox.x - 1) {
				if (lastMove.toBox.y == kingBox.y - 1) {
					whiteThreatened.amountAttacked++;
					whiteThreatened.threatenedInfo |= KingThreatenedInfo::upLeftThreatened;
					whiteThreatened.attackedInfo |= KingThreatenedInfo::upLeftThreatened;
					whiteThreatened.attackedFromBox = { lastMove.toBox.x,lastMove.toBox.y };
				}
			}
		}
	}

	//if we castled just update for the rook. there's a better way to do this but have you seen the rest of my code?
	if (lastMove.kingSideCastle || lastMove.queenSideCastle) {
		updateStraightDownThreats(currentTurn, currentBoardState);
		updateStraightUpThreats(currentTurn, currentBoardState);
		updateStraightLeftThreats(currentTurn, currentBoardState);
		updateStraightRightThreats(currentTurn, currentBoardState);
	}
	if (inSameRow(kingBox, lastMove.toBox)) {
		//std::cout << "Last move was from same row as king. " << std::endl;
		if (kingBox.x < lastMove.toBox.x) {
			//std::cout << "Updating right threats" << std::endl;
			updateStraightRightThreats(currentTurn, currentBoardState);
		}
		else {
			updateStraightLeftThreats(currentTurn, currentBoardState);
		}
	}
	else if (inSameCol(kingBox, lastMove.toBox)) {
		
		if (kingBox.y < lastMove.toBox.y) {
			updateStraightDownThreats(currentTurn, currentBoardState);
		}
		else {
			updateStraightUpThreats(currentTurn, currentBoardState);
		}
	}
	else if (inSameDiagonal(kingBox, lastMove.toBox)) {

		if (kingBox.x < lastMove.toBox.x) {
			updateUpRightThreats(currentTurn, currentBoardState);
		}
		else {
			updateDownLeftThreats(currentTurn, currentBoardState);
		}
	}
	else if (inSameReverseDiagonal(kingBox, lastMove.toBox)) {

		if (kingBox.x < lastMove.toBox.x) {
			
			updateDownRightThreats(currentTurn, currentBoardState);
		
		}
		else {
			updateUpLeftThreats(currentTurn, currentBoardState);
		}
	}

	//now we look at where the new piece moved TO.
	if (inSameRow(lastMove.fromBox, kingBox) && !inSameRow(lastMove.fromBox, lastMove.toBox)) {

		if (lastMove.fromBox.x > kingBox.x) {
			updateStraightRightThreats(currentTurn, currentBoardState);
		}
		else {
			updateStraightLeftThreats(currentTurn, currentBoardState);
		}

	}
	else if (inSameCol(lastMove.fromBox, kingBox) && !inSameCol(lastMove.fromBox, lastMove.toBox)) {

		if (lastMove.fromBox.y > kingBox.y) {
			updateStraightDownThreats(currentTurn, currentBoardState);
		}
		else {
			updateStraightUpThreats(currentTurn, currentBoardState);
		}

	}
	else if (inSameDiagonal(lastMove.fromBox, kingBox) && !inSameDiagonal(lastMove.fromBox, lastMove.toBox)) {

		if (lastMove.fromBox.x > kingBox.x) {

			updateUpRightThreats(currentTurn, currentBoardState);
		}
		else {

			updateDownLeftThreats(currentTurn, currentBoardState);
		}

	}
	else if (inSameReverseDiagonal(lastMove.fromBox, kingBox) && !inSameReverseDiagonal(lastMove.fromBox, lastMove.toBox)) {


		if (lastMove.fromBox.x > kingBox.x) {
			updateDownRightThreats(currentTurn, currentBoardState);
		}
		else {
			updateUpLeftThreats(currentTurn, currentBoardState);
		}

	}





}



void Board::initializeKingsThreatened(BoardState* currentBoardState) {
	std::cout << "Threatened at beginning of initialize: " << int(blackThreatened.threatenedInfo) << std::endl;
	whiteThreatened.threatenedInfo = 0;
	blackThreatened.threatenedInfo = 0;
	updateStraightUpThreats('w', currentBoardState);
	updateStraightDownThreats('w', currentBoardState);
	updateStraightLeftThreats('w', currentBoardState);
	updateStraightRightThreats('w', currentBoardState);
	updateDownLeftThreats('w', currentBoardState);
	updateUpLeftThreats('w', currentBoardState);
	updateDownRightThreats('w', currentBoardState);
	updateUpRightThreats('w', currentBoardState);


	updateStraightUpThreats('b', currentBoardState);
	updateStraightDownThreats('b', currentBoardState);
	updateStraightLeftThreats('b', currentBoardState);
	updateStraightRightThreats('b', currentBoardState);
	updateDownLeftThreats('b', currentBoardState);
	updateUpLeftThreats('b', currentBoardState);
	updateDownRightThreats('b', currentBoardState);
	updateUpRightThreats('b', currentBoardState);
	std::cout << "Threatened at end of initialize: " << int(blackThreatened.threatenedInfo) << std::endl;
}


void Board::updateStraightUpThreats(char currentTurn, BoardState* currentBoardState) {
	int defense = 0;
	uint8_t** board = currentBoardState->getBoard();
	Box kingSquare;
	bool foundAThreat = false;
	if (currentTurn == 'w') {
		kingSquare = whiteLocations.getKingLocation();
		//up
		for (int y = kingSquare.y - 1; y >= 0; --y) {
			if (board[kingSquare.x][y] == 0) {
				continue;
			}
			else if ((board[kingSquare.x][y] & Piece::white) == Piece::white || ((board[kingSquare.x][y] & Piece::rook) != Piece::rook && (board[kingSquare.x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			else {
				if ((board[kingSquare.x][y] & Piece::rook) == Piece::rook || (board[kingSquare.x][y] & Piece::queen) == Piece::queen) {
					whiteThreatened.straightUpBox = { kingSquare.x,y };
					if (defense == 0) {
						whiteThreatened.attackedInfo |= KingThreatenedInfo::straightUpThreatened;
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::straightUpThreatened;
						whiteThreatened.amountAttacked++;
						whiteThreatened.attackedFromBox = { kingSquare.x,y };
						foundAThreat = true;
						break;
					}
					else {
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::straightUpThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			whiteThreatened.threatenedInfo &= ~(1UL << 2);
		}

	}
	else {
		kingSquare = blackLocations.getKingLocation();
		//up
		for (int y = kingSquare.y - 1; y >= 0; --y) {
			if (board[kingSquare.x][y] == 0) {
				continue;
			}
			else if ((board[kingSquare.x][y] & Piece::black) == Piece::black || ((board[kingSquare.x][y] & Piece::rook) != Piece::rook && (board[kingSquare.x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[kingSquare.x][y] & Piece::rook) == Piece::rook || (board[kingSquare.x][y] & Piece::queen) == Piece::queen) {
					blackThreatened.straightUpBox = { kingSquare.x,y };
					if (defense == 0) {
						blackThreatened.attackedInfo |= KingThreatenedInfo::straightUpThreatened;
						blackThreatened.threatenedInfo |= KingThreatenedInfo::straightUpThreatened;
						blackThreatened.amountAttacked++;
						blackThreatened.attackedFromBox = { kingSquare.x,y };
						foundAThreat = true;
						break;
					}
					else {
						blackThreatened.threatenedInfo |= KingThreatenedInfo::straightUpThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			blackThreatened.threatenedInfo &= ~(1UL << 2);
		}
	}


}

void Board::updateStraightDownThreats(char currentTurn, BoardState* currentBoardState) {
	bool foundAThreat = false;
	Box kingSquare;
	uint8_t** board = currentBoardState->getBoard();
	int defense = 0;
	//down.
	if (currentTurn == 'w') {
		kingSquare = whiteLocations.getKingLocation();
		for (int y = kingSquare.y + 1; y < boardYBoxes; ++y) {
			if (board[kingSquare.x][y] == 0) {
				continue;
			}
			else if ((board[kingSquare.x][y] & Piece::white) == Piece::white || ((board[kingSquare.x][y] & Piece::rook) != Piece::rook && (board[kingSquare.x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[kingSquare.x][y] & Piece::rook) == Piece::rook || (board[kingSquare.x][y] & Piece::queen) == Piece::queen) {
					whiteThreatened.straightDownBox = { kingSquare.x,y };
					if (defense == 0) {
						whiteThreatened.attackedInfo |= KingThreatenedInfo::straightDownThreatened;
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::straightDownThreatened;
						whiteThreatened.amountAttacked++;
						whiteThreatened.attackedFromBox = { kingSquare.x,y };
						foundAThreat = true;
						break;
					}
					else {
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::straightDownThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			whiteThreatened.threatenedInfo &= ~(1UL << 6);
		}
		foundAThreat = false;
	}
	else {
		kingSquare = blackLocations.getKingLocation();
		for (int y = kingSquare.y + 1; y < boardYBoxes; ++y) {
			if (board[kingSquare.x][y] == 0) {
				continue;
			}
			else if ((board[kingSquare.x][y] & Piece::black) == Piece::black || ((board[kingSquare.x][y] & Piece::rook) != Piece::rook && (board[kingSquare.x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
			
				if ((board[kingSquare.x][y] & Piece::rook) == Piece::rook || (board[kingSquare.x][y] & Piece::queen) == Piece::queen) {
					blackThreatened.straightDownBox = { kingSquare.x,y };
					if (defense == 0) {
						blackThreatened.attackedInfo |= KingThreatenedInfo::straightDownThreatened;
						blackThreatened.threatenedInfo |= KingThreatenedInfo::straightDownThreatened;
						blackThreatened.amountAttacked++;
						blackThreatened.attackedFromBox = { kingSquare.x,y };
						foundAThreat = true;
						break;
					}
					else {
					
						blackThreatened.threatenedInfo |= KingThreatenedInfo::straightDownThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			blackThreatened.threatenedInfo &= ~(1UL << 6);
		}
	}

}

void Board::updateStraightLeftThreats(char currentTurn, BoardState* currentBoardState) {
	uint8_t** board = currentBoardState->getBoard();
	int defense = 0;
	bool foundAThreat = false;
	Box kingSquare;
	if (currentTurn == 'w') {
		kingSquare = whiteLocations.getKingLocation();
		for (int x = kingSquare.x - 1; x >= 0; --x) {
			if (board[x][kingSquare.y] == 0) {
				continue;
			}
			else if ((board[x][kingSquare.y] & Piece::white) == Piece::white || ((board[x][kingSquare.y] & Piece::rook) != Piece::rook && (board[x][kingSquare.y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][kingSquare.y] & Piece::rook) == Piece::rook || (board[x][kingSquare.y] & Piece::queen) == Piece::queen) {
					whiteThreatened.straightLeftBox = { x,kingSquare.y };
					if (defense == 0) {
						whiteThreatened.attackedInfo |= KingThreatenedInfo::straightLeftThreatened;
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::straightLeftThreatened;
						whiteThreatened.amountAttacked++;
						whiteThreatened.attackedFromBox = { x,kingSquare.y };
						foundAThreat = true;
						break;
					}
					else {
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::straightLeftThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			whiteThreatened.threatenedInfo &= ~(1UL << 0);
		}

	}
	else {
		kingSquare = blackLocations.getKingLocation();
		for (int x = kingSquare.x - 1; x >= 0; --x) {
			if (board[x][kingSquare.y] == 0) {
				continue;
			}
			else if ((board[x][kingSquare.y] & Piece::black) == Piece::black || ((board[x][kingSquare.y] & Piece::rook) != Piece::rook && (board[x][kingSquare.y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][kingSquare.y] & Piece::rook) == Piece::rook || (board[x][kingSquare.y] & Piece::queen) == Piece::queen) {
					blackThreatened.straightLeftBox = { x,kingSquare.y };
					if (defense == 0) {
						blackThreatened.attackedInfo |= KingThreatenedInfo::straightLeftThreatened;
						blackThreatened.threatenedInfo |= KingThreatenedInfo::straightLeftThreatened;
						blackThreatened.amountAttacked++;
						blackThreatened.attackedFromBox = { x,kingSquare.y };
						foundAThreat = true;
						break;
					}
					else {
						blackThreatened.threatenedInfo |= KingThreatenedInfo::straightLeftThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		//to the left
		if (!foundAThreat) {
			blackThreatened.threatenedInfo &= ~(1UL << 0);
		}
	}

}

void Board::updateStraightRightThreats(char currentTurn, BoardState* currentBoardState) {

	uint8_t** board = currentBoardState->getBoard();
	int defense = 0;
	Box kingSquare;
	bool foundAThreat = false;
	if (currentTurn == 'w') {
		kingSquare = whiteLocations.getKingLocation();
		//black time!!!
		defense = 0;

		for (int x = kingSquare.x + 1; x < boardXBoxes; ++x) {
			if (board[x][kingSquare.y] == 0) {
				continue;
			}
			else if ((board[x][kingSquare.y] & Piece::white) == Piece::white || ((board[x][kingSquare.y] & Piece::rook) != Piece::rook && (board[x][kingSquare.y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {

					break;
				}
			}
			//it's a black piece.
			else {
				if ((board[x][kingSquare.y] & Piece::rook) == Piece::rook || (board[x][kingSquare.y] & Piece::queen) == Piece::queen) {
					whiteThreatened.straightRightBox = { x,kingSquare.y };
					if (defense == 0) {
						whiteThreatened.attackedInfo |= KingThreatenedInfo::straightRightThreatened;
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::straightRightThreatened;
						whiteThreatened.amountAttacked++;
						whiteThreatened.attackedFromBox = { x,kingSquare.y };
						foundAThreat = true;
						break;
					}
					else {
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::straightRightThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			//std::cout << "white threatened before setting it to 0: " << int(whiteThreatened.threatenedInfo) << std::endl;
			whiteThreatened.threatenedInfo &= ~(1UL << 4);
			//std::cout << "white threatened after setting it to 0: " << int(whiteThreatened.threatenedInfo) << std::endl;
		}
		foundAThreat = false;
	}
	else {
		kingSquare = blackLocations.getKingLocation();
		//black time!!!
		defense = 0;

		for (int x = kingSquare.x + 1; x < boardXBoxes; ++x) {
			if (board[x][kingSquare.y] == 0) {
				continue;
			}
			else if ((board[x][kingSquare.y] & Piece::black) == Piece::black || ((board[x][kingSquare.y] & Piece::rook) != Piece::rook && (board[x][kingSquare.y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][kingSquare.y] & Piece::rook) == Piece::rook || (board[x][kingSquare.y] & Piece::queen) == Piece::queen) {
					blackThreatened.straightRightBox = { x,kingSquare.y };
					if (defense == 0) {
						blackThreatened.attackedInfo |= KingThreatenedInfo::straightRightThreatened;
						blackThreatened.threatenedInfo |= KingThreatenedInfo::straightRightThreatened;
						blackThreatened.amountAttacked++;
						blackThreatened.attackedFromBox = { x,kingSquare.y };
						foundAThreat = true;
						break;
					}
					else {
						blackThreatened.threatenedInfo |= KingThreatenedInfo::straightRightThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			blackThreatened.threatenedInfo &= ~(1UL << 4);
		}
	}


}

void Board::updateUpLeftThreats(char currentTurn, BoardState* currentBoardState) {
	//std::cout << "Checking for up lefts" << std::endl;
	uint8_t** board = currentBoardState->getBoard();
	int defense = 0;
	bool foundAThreat = false;
	Box kingSquare;
	int x, y;
	if (currentTurn == 'w') {
		kingSquare = whiteLocations.getKingLocation();
		x = kingSquare.x;
		y = kingSquare.y;
		while (--x >= 0 && --y >= 0) {
			if (board[x][y] == 0) {
				continue;
			}
			else if ((board[x][y] & Piece::white) == Piece::white || ((board[x][y] & Piece::bishop) != Piece::bishop && (board[x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][y] & Piece::bishop) == Piece::bishop || (board[x][y] & Piece::queen) == Piece::queen) {
					whiteThreatened.upLeftBox = { x,y };

					if (defense == 0) {
						
						whiteThreatened.attackedInfo |= KingThreatenedInfo::upLeftThreatened;
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::upLeftThreatened;
						whiteThreatened.amountAttacked++;
						whiteThreatened.attackedFromBox = { x,y };
						foundAThreat = true;
						break;
					}
					else {
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::upLeftThreatened;
					
						foundAThreat = true;
						break;
					}
				}
			}
		}

		if (!foundAThreat) {
			whiteThreatened.threatenedInfo &= ~(1UL << 1);
		}

	}
	else {
		kingSquare = blackLocations.getKingLocation();
		//up to the left
		x = kingSquare.x;
		y = kingSquare.y;
		while (--x >= 0 && --y >= 0) {
			if (board[x][y] == 0) {
				continue;
			}
			else if ((board[x][y] & Piece::black) == Piece::black || ((board[x][y] & Piece::bishop) != Piece::bishop && (board[x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][y] & Piece::bishop) == Piece::bishop || (board[x][y] & Piece::queen) == Piece::queen) {
					blackThreatened.upLeftBox = { x,y };
					if (defense == 0) {
						blackThreatened.attackedInfo |= KingThreatenedInfo::upLeftThreatened;
						blackThreatened.threatenedInfo |= KingThreatenedInfo::upLeftThreatened;
						blackThreatened.amountAttacked++;
						blackThreatened.attackedFromBox = { x,y };
						foundAThreat = true;
						break;
					}
					else {
						blackThreatened.threatenedInfo |= KingThreatenedInfo::upLeftThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			blackThreatened.threatenedInfo &= ~(1UL << 1);
		}
	}

}

void Board::updateUpRightThreats(char currentTurn, BoardState* currentBoardState) {
	uint8_t** board = currentBoardState->getBoard();
	int defense = 0;
	Box kingSquare;
	int x, y;
	bool foundAThreat = false;
	if (currentTurn == 'w') {
		kingSquare = whiteLocations.getKingLocation();
		x = kingSquare.x;
		y = kingSquare.y;
		while (++x < boardXBoxes && --y >= 0) {
			if (board[x][y] == 0) {
				continue;
			}
			else if ((board[x][y] & Piece::white) == Piece::white || ((board[x][y] & Piece::bishop) != Piece::bishop && (board[x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a black
			else {
				if ((board[x][y] & Piece::bishop) == Piece::bishop || (board[x][y] & Piece::queen) == Piece::queen) {
					whiteThreatened.upRightBox = { x,y };
					if (defense == 0) {
						whiteThreatened.attackedInfo |= KingThreatenedInfo::upRightThreatened;
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::upRightThreatened;
						whiteThreatened.amountAttacked++;
						whiteThreatened.attackedFromBox = { x,y };
						foundAThreat = true;
						break;
					}
					else {
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::upRightThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			whiteThreatened.threatenedInfo &= ~(1UL << 3);
		}
		foundAThreat = false;
	}
	else {
		kingSquare = blackLocations.getKingLocation();
		x = kingSquare.x;
		y = kingSquare.y;
		while (++x < boardXBoxes && --y >= 0) {
			if (board[x][y] == 0) {
				continue;
			}
			else if ((board[x][y] & Piece::black) == Piece::black || ((board[x][y] & Piece::bishop) != Piece::bishop && (board[x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][y] & Piece::bishop) == Piece::bishop || (board[x][y] & Piece::queen) == Piece::queen) {
					blackThreatened.upRightBox = { x,y };
					if (defense == 0) {
						blackThreatened.attackedInfo |= KingThreatenedInfo::upRightThreatened;
						blackThreatened.threatenedInfo |= KingThreatenedInfo::upRightThreatened;
						blackThreatened.amountAttacked++;
						blackThreatened.attackedFromBox = { x,y };
						foundAThreat = true;
						break;
					}
					else {
						blackThreatened.threatenedInfo |= KingThreatenedInfo::upRightThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			blackThreatened.threatenedInfo &= ~(1UL << 3);
		}
	}


}

void Board::updateDownLeftThreats(char currentTurn, BoardState* currentBoardState) {
	uint8_t** board = currentBoardState->getBoard();
	int defense = 0;
	Box kingSquare;
	bool foundAThreat = false;
	int x, y;
	if (currentTurn == 'b') {
		//down to the left
		kingSquare = blackLocations.getKingLocation();
		x = kingSquare.x;
		y = kingSquare.y;
		while (--x >= 0 && ++y < boardYBoxes) {
			if (board[x][y] == 0) {
				continue;
			}
			else if ((board[x][y] & Piece::black) == Piece::black || ((board[x][y] & Piece::bishop) != Piece::bishop && (board[x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][y] & Piece::bishop) == Piece::bishop || (board[x][y] & Piece::queen) == Piece::queen) {

					blackThreatened.downLeftBox = { x,y };
					if (defense == 0) {
						blackThreatened.attackedInfo |= KingThreatenedInfo::downLeftThreatened;
						blackThreatened.threatenedInfo |= KingThreatenedInfo::downLeftThreatened;

						blackThreatened.amountAttacked++;
						blackThreatened.attackedFromBox = { x,y };
						foundAThreat = true;
						break;
					}
					else {

						blackThreatened.threatenedInfo |= KingThreatenedInfo::downLeftThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			blackThreatened.threatenedInfo &= ~(1UL << 7);
		}
		foundAThreat = false;
	}
	else {
		//down to the left
		kingSquare = whiteLocations.getKingLocation();
		x = kingSquare.x;
		y = kingSquare.y;
		while (--x >= 0 && ++y < boardYBoxes) {
			if (board[x][y] == 0) {
				continue;
			}
			else if ((board[x][y] & Piece::white) == Piece::white || ((board[x][y] & Piece::bishop) != Piece::bishop && (board[x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][y] & Piece::bishop) == Piece::bishop || (board[x][y] & Piece::queen) == Piece::queen) {
					whiteThreatened.downLeftBox = { x,y };
					if (defense == 0) {
						whiteThreatened.attackedInfo |= KingThreatenedInfo::downLeftThreatened;
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::downLeftThreatened;
						whiteThreatened.amountAttacked++;
						whiteThreatened.attackedFromBox = { x,y };
						foundAThreat = true;
						break;
					}
					else {
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::downLeftThreatened;
						foundAThreat = true;
						break;
					}
				}
			}
		}
		if (!foundAThreat) {
			whiteThreatened.threatenedInfo &= ~(1UL << 7);
		}
	}

}

void Board::updateDownRightThreats(char currentTurn, BoardState* currentBoardState) {
	uint8_t** board = currentBoardState->getBoard();
	int defense = 0;
	Box kingSquare;
	int x, y;
	bool foundAThreat = false;
	if (currentTurn == 'w') {
		kingSquare = whiteLocations.getKingLocation();
		x = kingSquare.x;
		y = kingSquare.y;
		while (++x < boardXBoxes && ++y < boardYBoxes) {
			if (board[x][y] == 0) {
				continue;
			}
			else if ((board[x][y] & Piece::white) == Piece::white || ((board[x][y] & Piece::bishop) != Piece::bishop && (board[x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][y] & Piece::bishop) == Piece::bishop || (board[x][y] & Piece::queen) == Piece::queen) {
					whiteThreatened.downRightBox = { x,y };
					if (defense == 0) {
						whiteThreatened.attackedInfo |= KingThreatenedInfo::downRightThreatened;
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::downRightThreatened;
						whiteThreatened.amountAttacked++;
						whiteThreatened.attackedFromBox = { x,y };
						foundAThreat = true;
						break;
					}
					else {
						whiteThreatened.threatenedInfo |= KingThreatenedInfo::downRightThreatened;
						foundAThreat = true;
						break;
					}
				}
			}

		}
		if (!foundAThreat) {
			whiteThreatened.threatenedInfo &= ~(1UL << 5);
		}
		foundAThreat = false;
	}
	else {
		kingSquare = blackLocations.getKingLocation();
		x = kingSquare.x;
		y = kingSquare.y;
		while (++x < boardXBoxes && ++y < boardYBoxes) {
			if (board[x][y] == 0) {
				continue;
			}
			else if ((board[x][y] & Piece::black) == Piece::black || ((board[x][y] & Piece::bishop) != Piece::bishop && (board[x][y] & Piece::queen) != Piece::queen)) {
				defense++;
				if (defense == 2) {
					break;
				}
			}
			//it's a white piece.
			else {
				if ((board[x][y] & Piece::bishop) == Piece::bishop || (board[x][y] & Piece::queen) == Piece::queen) {
					blackThreatened.downRightBox = { x,y };
					if (defense == 0) {
						blackThreatened.attackedInfo |= KingThreatenedInfo::downRightThreatened;
						blackThreatened.threatenedInfo |= KingThreatenedInfo::downRightThreatened;
						blackThreatened.amountAttacked++;
						blackThreatened.attackedFromBox = { x,y };
						foundAThreat = true;
						break;
					}
					else {
						blackThreatened.threatenedInfo |= KingThreatenedInfo::downRightThreatened;
						foundAThreat = true;
						break;
					}
				}
			}

		}
		if (!foundAThreat) {
			blackThreatened.threatenedInfo &= ~(1UL << 5);
		}
	}
}


//returns true if a piece is a current players piece, false otherwise.
bool Board::pieceIsCurrentPlayersPiece(int x, int y, BoardState* currentBoardState) {
	char currentTurn = currentBoardState->getCurrentTurn();
	uint8_t** board = currentBoardState->getBoard();
	if ((currentTurn == 'w' && (board[x][y] & Piece::white) == Piece::white) || (currentTurn == 'b' && (board[x][y] & Piece::black) == Piece::black)) {
		return true;
	}
	return false;
}

bool Board::kingAttacked(BoardState* currentBoardState) {
	char currentTurn = currentBoardState->getCurrentTurn();
	if (currentTurn == 'w') {
		return whiteThreatened.amountAttacked >= 1;
	}
	return blackThreatened.amountAttacked >= 1;
}

bool Board::doesBoxBlockAttack(Box box, BoardState* currentBoardState) {
	char currentTurn = currentBoardState->getCurrentTurn();

	uint8_t attackedInfo;
	Box attackedFromBox;
	Box kingBox;
	bool attackedByKnight;
	if (currentTurn == 'w') {
		attackedInfo = whiteThreatened.attackedInfo;
		attackedFromBox = { whiteThreatened.attackedFromBox.x,whiteThreatened.attackedFromBox.y };
		attackedByKnight = whiteThreatened.attackedByKnight;
		kingBox = { whiteLocations.getKingLocation().x,whiteLocations.getKingLocation().y };
	}
	else {
		attackedInfo = blackThreatened.attackedInfo;
		attackedFromBox = { blackThreatened.attackedFromBox.x,blackThreatened.attackedFromBox.y };
		attackedByKnight = blackThreatened.attackedByKnight;
		kingBox = { blackLocations.getKingLocation().x,blackLocations.getKingLocation().y };
	}

	//when you're attacked by a knight the only way to block it with a piece is to take it.
	if (attackedByKnight) {
		return box.x == attackedFromBox.x && box.y == attackedFromBox.y;
	}
	if (box.x == attackedFromBox.x && box.y == attackedFromBox.y) {
		return true;
	}
	

	switch (attackedInfo) {
	case KingThreatenedInfo::straightDownThreatened:
		return box.x == attackedFromBox.x && box.y < attackedFromBox.y && box.y > kingBox.y;
		break;
	case KingThreatenedInfo::straightUpThreatened:
		return box.x == attackedFromBox.x && box.y > attackedFromBox.y && box.y < kingBox.y;
		break;
	case KingThreatenedInfo::straightLeftThreatened:
		return box.y == attackedFromBox.y && box.x > attackedFromBox.x && box.x < kingBox.x;	
		break;
	case KingThreatenedInfo::straightRightThreatened:
		return box.y == attackedFromBox.y && box.x < attackedFromBox.x && box.x > kingBox.x;	
		break;
	case KingThreatenedInfo::upLeftThreatened:
		if (inSameReverseDiagonal(box, kingBox)) {
			if (box.x < kingBox.x && box.x > attackedFromBox.x) {
				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
		break;
	case KingThreatenedInfo::upRightThreatened:
		if (inSameDiagonal(box, kingBox)) {
			return box.x > kingBox.x && box.x < attackedFromBox.x;
		}
		break;
	case KingThreatenedInfo::downLeftThreatened:
		if (inSameDiagonal(box, kingBox)) {
			return box.x < kingBox.x && box.x > attackedFromBox.x;
		}
		break;
	case KingThreatenedInfo::downRightThreatened:
		if (inSameReverseDiagonal(box, kingBox)) {
			return box.x > kingBox.x && box.x < attackedFromBox.x;
		}
		return false;
		break;
	default:
		std::cout << "error in switch" << std::endl;
	}
}

void Board::attemptAddMove(Move move, BoardState* currentBoardState, std::vector<Move>& moves) {
	char currentTurn = currentBoardState->getCurrentTurn();
	if (currentTurn == 'w') {
		if (whiteThreatened.amountAttacked >= 1) {
			if (doesBoxBlockAttack(move.toBox, currentBoardState)) {
				moves.push_back({ {move.fromBox.x,move.fromBox.y},{move.toBox.x,move.toBox.y},move.kingSideCastle,move.queenSideCastle,move.enPassant,move.isPromotion,move.promotionType });
			}
		}
		else {
			moves.push_back({ {move.fromBox.x,move.fromBox.y},{move.toBox.x,move.toBox.y},move.kingSideCastle,move.queenSideCastle,move.enPassant,move.isPromotion,move.promotionType });
		}
	}
	else {
		if (blackThreatened.amountAttacked >= 1) {
			if (doesBoxBlockAttack(move.toBox, currentBoardState)) {
				moves.push_back({ {move.fromBox.x,move.fromBox.y},{move.toBox.x,move.toBox.y},move.kingSideCastle,move.queenSideCastle,move.enPassant,move.isPromotion,move.promotionType });
			}
		}
		else {
			moves.push_back({ {move.fromBox.x,move.fromBox.y},{move.toBox.x,move.toBox.y},move.kingSideCastle,move.queenSideCastle,move.enPassant,move.isPromotion,move.promotionType });
		}
	}
}

void Board::addStraightUpMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;
	//going up on the board


	for (int currY = y - 1; currY >= 0; --currY) {
		if (currentBoardState->getBoard()[x][currY] == 0) {
			attemptAddMove({ {x,y},{x,currY}, false, false, false }, currentBoardState, moves);

		}
		else if (pieceIsCurrentPlayersPiece(x, currY, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x, currY, currentBoardState)) {
			attemptAddMove({ {x,y},{x,currY}, false, false, false }, currentBoardState, moves);
			break;

		}
	}
}

//WHY ARE THERE SO MANY PAWN MOVES BEING ADDED


void Board::addStraightDownMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;
	for (int currY = y + 1; currY < boardYBoxes; ++currY) {
		if (currentBoardState->getBoard()[x][currY] == 0) {
			attemptAddMove({ {x,y},{x,currY}, false, false, false }, currentBoardState, moves);

		}
		else if (pieceIsCurrentPlayersPiece(x, currY, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x, currY, currentBoardState)) {
			attemptAddMove({ {x,y},{x,currY}, false, false, false }, currentBoardState, moves);
			break;

		}
	}
}

void Board::addStraightLeftMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	//going to the left on the board.
	int x = box.x;
	int y = box.y;
	for (int currX = x - 1; currX >= 0; --currX) {
		if (currentBoardState->getBoard()[currX][y] == 0) {
			attemptAddMove({ {x,y},{currX,y}, false, false, false }, currentBoardState, moves);

		}
		else if (pieceIsCurrentPlayersPiece(currX, y, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(currX, y, currentBoardState)) {
			attemptAddMove({ {x,y},{currX,y}, false, false, false }, currentBoardState, moves);
			break;

		}
	}
}

void Board::addStraightRightMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;
	//going to the right on the board.
	for (int currX = x + 1; currX < boardXBoxes; ++currX) {
		if (currentBoardState->getBoard()[currX][y] == 0) {
			attemptAddMove({ {x,y},{currX,y}, false, false, false }, currentBoardState, moves);

		}
		else if (pieceIsCurrentPlayersPiece(currX, y, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(currX, y, currentBoardState)) {
			attemptAddMove({ {x,y},{currX,y}, false, false, false }, currentBoardState, moves);
			break;

		}
	}
}

void Board::addUpRightMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	//going up and to the right
	int x = box.x;
	int y = box.y;
	for (int change = 1; x + change < boardXBoxes && y - change >= 0; ++change) {
		if (currentBoardState->getBoard()[x + change][y - change] == 0) {
			attemptAddMove({ {x,y},{x + change,y - change}, false, false, false }, currentBoardState, moves);
		}
		else if (pieceIsCurrentPlayersPiece(x + change, y - change, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x + change, y - change, currentBoardState)) {
			attemptAddMove({ {x,y},{x + change,y - change}, false, false, false }, currentBoardState, moves);
			break;
		}
	}
}

void Board::addUpLeftMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	//going up and to the left.
	int x = box.x;
	int y = box.y;
	for (int change = 1; x - change >= 0 && y - change >= 0; ++change) {
		if (currentBoardState->getBoard()[x - change][y - change] == 0) {
			attemptAddMove({ {x,y},{x - change,y - change}, false, false, false }, currentBoardState, moves);

		}
		else if (pieceIsCurrentPlayersPiece(x - change, y - change, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x - change, y - change, currentBoardState)) {
			attemptAddMove({ {x,y},{x - change,y - change}, false, false, false }, currentBoardState, moves);
			break;
		}
	}
}

void Board::addDownRightMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;
	for (int change = 1; x + change < boardXBoxes && y + change < boardYBoxes; ++change) {
		if (currentBoardState->getBoard()[x + change][y + change] == 0) {
			attemptAddMove({ {x,y},{x + change,y + change}, false, false, false }, currentBoardState, moves);

		}
		else if (pieceIsCurrentPlayersPiece(x + change, y + change, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x + change, y + change, currentBoardState)) {
			attemptAddMove({ {x,y},{x + change,y + change}, false, false, false }, currentBoardState, moves);
			break;
		}
	}
}

void Board::addDownLeftMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	//going down and to the left
	int x = box.x;
	int y = box.y;




	for (int change = 1; x - change >= 0 && y + change < boardYBoxes; ++change) {
		if (currentBoardState->getBoard()[x - change][y + change] == 0) {
			attemptAddMove({ {x,y},{x - change,y + change}, false, false, false }, currentBoardState, moves);

		}
		else if (pieceIsCurrentPlayersPiece(x - change, y + change, currentBoardState)) {
			break;
		}
		else if (!pieceIsCurrentPlayersPiece(x - change, y + change, currentBoardState)) {
			attemptAddMove({ {x,y},{x - change,y + change}, false, false, false }, currentBoardState, moves);
			break;
		}

	}
}


void Board::addStraightUpPawnMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	//moving forward one.
	int x = box.x;
	int y = box.y;
	if (y - 1 >= 0) {
		//pawns cant take vertically.
		if (currentBoardState->getBoard()[x][y - 1] == 0) {
			//if it's a promotion
			if (y - 1 == 0) {
				attemptAddMove({ {x,y},{x,y - 1},false,false,false,true,'q' }, currentBoardState, moves);
				attemptAddMove({ {x,y},{x,y - 1},false,false,false,true,'r' }, currentBoardState, moves);
				attemptAddMove({ {x,y},{x,y - 1},false,false,false,true,'n' }, currentBoardState, moves);
				attemptAddMove({ {x,y},{x,y - 1},false,false,false,true,'b' }, currentBoardState, moves);
			}
			else {
				attemptAddMove({ {x,y},{x,y - 1},false,false,false,false,' ' }, currentBoardState, moves);
			}

			//we only can move forward 2 if the space is open as well.
			if (y == boardYBoxes - 2) {	//if it's in the starting position.
				if (y - 2 >= 0) {	//this shouldn't be necessary except for tiny boards
					if (currentBoardState->getBoard()[x][y - 2] == 0) {
						attemptAddMove({ {x,y},{x,y - 2}, false, false, false,false,' ' }, currentBoardState, moves);
					}
				}
			}
		}
	}

}
void Board::addStraightDownPawnMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;

	//moving forward one.
	if (y + 1 < boardYBoxes) {
		//pawns cant take vertically.
		if (currentBoardState->getBoard()[x][y + 1] == 0) {
			if (y + 1 == boardYBoxes - 1) {
				attemptAddMove({ {x,y},{x,y + 1} ,false,false,false,true,'r' }, currentBoardState, moves);
				attemptAddMove({ {x,y},{x,y + 1 },false,false,false,true,'n' }, currentBoardState, moves);
				attemptAddMove({ {x,y},{x,y + 1} ,false,false,false,true,'b' }, currentBoardState, moves);
				attemptAddMove({ {x,y},{x,y + 1} ,false,false,false,true,'q' }, currentBoardState, moves);
			}
			else {
				attemptAddMove({ {x,y},{x,y + 1} ,false,false,false,false,' ' }, currentBoardState, moves);
			}

			//we only can move forward 2 if the space is open as well.
			if (y == 1) {	//if it's in the starting position.
				if (y + 2 < boardYBoxes) {	//this shouldn't be necessary except for tiny boards
					if (currentBoardState->getBoard()[x][y + 2] == 0) {
						attemptAddMove({ {x,y},{x,y + 2}, false, false, false,false , ' ' }, currentBoardState, moves);
					}
				}
			}
		}
	}

}
void Board::addDownLeftPawnMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;
	//pawns capture diagonally.
	if (y + 1 < boardYBoxes) {
		//if we're not at the edge of the board
		if (x - 1 >= 0) {
			if (currentBoardState->getBoard()[x - 1][y + 1] != 0 && !pieceIsCurrentPlayersPiece(x - 1, y + 1, currentBoardState)) {
				//promotion time
				if (y + 1 == boardYBoxes - 1) {
					attemptAddMove({ {x,y},{x - 1,y + 1} ,false,false,false,true,'q' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x - 1,y + 1} ,false,false,false,true,'r' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x - 1,y + 1} ,false,false,false,true,'b' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x - 1,y + 1} ,false,false,false,true,'n' }, currentBoardState, moves);
				}
				else {
					attemptAddMove({ {x,y},{x - 1,y + 1} ,false,false,false,false,' ' }, currentBoardState, moves);
				}
			}
			else if (x - 1 == currentBoardState->getEnPassantX() && y + 1 == currentBoardState->getEnPassantY()) {
				if (canEnPassant({ x,y }, { x - 1,y + 1 }, currentBoardState)) {
					attemptAddMove({ {x,y},{x - 1,y + 1},false,false,true,false,' ' }, currentBoardState, moves);
				}

			}
		}

	}
}
void Board::addDownRightPawnMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;
	if (y + 1 < boardYBoxes) {

		if (x + 1 < boardXBoxes) {
			if (currentBoardState->getBoard()[x + 1][y + 1] != 0 && !pieceIsCurrentPlayersPiece(x + 1, y + 1, currentBoardState)) {

				if (y + 1 == boardYBoxes - 1) {
					attemptAddMove({ {x,y},{x + 1,y + 1} ,false,false,false,true,'r' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x + 1,y + 1} ,false,false,false,true,'q' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x + 1,y + 1} ,false,false,false,true,'b' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x + 1,y + 1 }, false, false, false, true, 'n'}, currentBoardState, moves);
				}
				else {
					attemptAddMove({ {x,y},{x + 1,y + 1} ,false,false,false,false,' ' }, currentBoardState, moves);
				}
			}
			else if (x + 1 == currentBoardState->getEnPassantX() && y + 1 == currentBoardState->getEnPassantY()) {
				if (canEnPassant({ x,y }, { x + 1,y + 1 }, currentBoardState)) {
					attemptAddMove({ {x,y},{x + 1,y + 1},false,false,true,false,' ' }, currentBoardState, moves);
				}

			}
		}
	}
}
void Board::addUpRightPawnMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;

	//pawns capture diagonally.
	if (y - 1 >= 0) {

		if (x + 1 < boardXBoxes) {
			if (currentBoardState->getBoard()[x + 1][y - 1] != 0 && !pieceIsCurrentPlayersPiece(x + 1, y - 1, currentBoardState)) {
				if (y - 1 == 0) {
					attemptAddMove({ {x,y},{x + 1,y - 1}, false, false, false,true,'q' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x + 1,y - 1}, false, false, false,true,'r' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x + 1,y - 1}, false, false, false,true,'n' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x + 1,y - 1}, false, false, false,true,'b' }, currentBoardState, moves);
				}
				else {
					attemptAddMove({ {x,y},{x + 1,y - 1}, false, false, false,false,' ' }, currentBoardState, moves);
				}

			}
			//en passant
			else if (x + 1 == currentBoardState->getEnPassantX() && y - 1 == currentBoardState->getEnPassantY()) {
				//we can only en passant if it doesn't put our king in check.

				if (canEnPassant({ x,y }, { x + 1,y - 1 }, currentBoardState)) {
					attemptAddMove({ {x,y},{x + 1,y - 1},false,false,true,false,' ' }, currentBoardState, moves);
				}



			}
		}
	}
}


bool Board::canEnPassant(Box pawnBox, Box toBox, BoardState* currentBoardState) {
	char currentTurn = currentBoardState->getCurrentTurn();
	Box kingBox;
	uint8_t enemyPiece;
	uint8_t** board = currentBoardState->getBoard();
	if (currentTurn == 'w') {
		kingBox = whiteLocations.getKingLocation();
		enemyPiece = Piece::black;
	}
	else {
		kingBox = blackLocations.getKingLocation();
		enemyPiece = Piece::white;
	}

	if (inSameRow(kingBox, { pawnBox.x,pawnBox.y })) {
		//if the pawn is on the right of the king
		if (pawnBox.x > kingBox.x) {
			//enPassanting to the right, therefore taking a pawn away from pawnBox.x + 1
			if (toBox.x > pawnBox.x) {
				for (int i = pawnBox.x+2; i < boardXBoxes; ++i) {
					if (board[i][pawnBox.y] == 0) {
						continue;
					}
					else if ((board[i][pawnBox.y] == (enemyPiece | Piece::rook)) || (board[i][pawnBox.y] == (enemyPiece | Piece::queen))) {
						for (int j = pawnBox.x - 1; j > kingBox.x; --j) {
							if (board[j][pawnBox.y] != 0) {
								return true;
							}
						}
						return false;
					}
					else {	//if there's some other piece there.
						break;
					}
				}
			}
			//enPassanting to the left, therefore taking a pawn away from pawnBox.x -1 but we still search from the right side
			else {
				for (int i = pawnBox.x +1; i < boardXBoxes; ++i) {
					if (board[i][pawnBox.y] == 0) {
						continue;
					}
					else if ((board[i][pawnBox.y] == (enemyPiece | Piece::rook)) || (board[i][pawnBox.y] == (enemyPiece | Piece::queen))) {
						for (int j = pawnBox.x - 2; j > kingBox.x; --j) {
							if (board[j][pawnBox.y] != 0) {
								return true;
							}
						}
						return false;
					}
					else {	//if there's some other piece there.
						break;
					}
				}
			}

		}
		else {
			//en passanting to right on the left side of the king
			if (toBox.x > pawnBox.x) {
				for (int i = pawnBox.x - 1; i >=0; --i) {
					if (board[i][pawnBox.y] == 0) {
						continue;
					}
					else if ((board[i][pawnBox.y] == (enemyPiece | Piece::rook)) || (board[i][pawnBox.y] == (enemyPiece | Piece::queen))) {
						for (int j = pawnBox.x + 2; j < kingBox.x; j++) {
							if (board[j][pawnBox.y] != 0) {
								return true;
							}
						}
						return false;
					}
					else {	//if there's some other piece there.
						break;
					}
				}
			}
			//en passanting to the left on the left side of the king.
			else {
				for (int i = pawnBox.x - 2; i >=0; --i) {
					if (board[i][pawnBox.y] == 0) {
						continue;
					}
					else if ((board[i][pawnBox.y] == (enemyPiece | Piece::rook)) || (board[i][pawnBox.y] == (enemyPiece | Piece::queen))) {
						for (int j = pawnBox.x + 1; j < kingBox.x; j++) {
							if (board[j][pawnBox.y] != 0) {
								return true;
							}
						}
						return false;
					}
					else {	//if there's some other piece there.
						break;
					}
				}
			}

		}

	}
	return true;
}

void Board::addUpLeftPawnMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	//if we're not at the edge of the board
	int x = box.x;
	int y = box.y;
	if (y - 1 >= 0) {
		if (x - 1 >= 0) {
			if (currentBoardState->getBoard()[x - 1][y - 1] != 0 && !pieceIsCurrentPlayersPiece(x - 1, y - 1, currentBoardState)) {
				if (y - 1 == 0) {
					attemptAddMove({ {x,y},{x - 1,y - 1}, false, false, false,true,'q' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x - 1,y - 1}, false, false, false,true,'r' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x - 1,y - 1}, false, false, false,true,'n' }, currentBoardState, moves);
					attemptAddMove({ {x,y},{x - 1,y - 1}, false, false, false,true,'b' }, currentBoardState, moves);
				}
				else {
					attemptAddMove({ {x,y},{x - 1,y - 1}, false, false, false,false,' ' }, currentBoardState, moves);
				}

			}
			//en passant
			else if (x - 1 == currentBoardState->getEnPassantX() && y - 1 == currentBoardState->getEnPassantY()) {
				if (canEnPassant({ x,y }, { x - 1,y - 1 }, currentBoardState)) {
					attemptAddMove({ {x,y},{x - 1,y - 1},false,false,true,false,' ' }, currentBoardState, moves);
				}

			}
		}
	}

}

void Board::calculatePawnLegalMoves(Box pawnBox, Box kingBox, BoardState* currentBoardState, std::vector<Move>& currentLegalMoves, KingThreatenedInfo currentKingInfo) {
	char currentTurn = currentBoardState->getCurrentTurn();
	bool addAll = true;

	if (currentTurn == 'w') {
		if (inSameCol(pawnBox, kingBox)) {

			//if the pawn is above the king.
			if (pawnBox.y < kingBox.y) {
				//if the king is threatened from above we only add the pawn moves that are straight up and down.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightUpThreatened) == KingThreatenedInfo::straightUpThreatened) {
					if (currentKingInfo.straightUpBox.y < pawnBox.y) {
						addStraightUpPawnMoves(pawnBox, currentBoardState, currentLegalMoves);
						addAll = false;
					}

				}


			}
			//the pawn is below the king.
			else if (pawnBox.y > kingBox.y) {
				//if the king is threatened from below we only add moves that add the pawn up and down.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightDownThreatened) == KingThreatenedInfo::straightDownThreatened) {
					if (currentKingInfo.straightDownBox.y > pawnBox.y) {
						addStraightUpPawnMoves(pawnBox, currentBoardState, currentLegalMoves);
						addAll = false;
					}
				}
			}
		}
		//if it's in the same row and i's attacked we reallllly can't move it. 
		else if (inSameRow(pawnBox, kingBox)) {
			//pawn is to the right of the king.
			if (pawnBox.x > kingBox.x) {
				//if the king is threatened from right we only add the pawn moves that are straight up and down.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightRightThreatened) == KingThreatenedInfo::straightRightThreatened) {
					if (currentKingInfo.straightRightBox.x > pawnBox.x) {
						addAll = false;
					}

				}

			}
			else if (pawnBox.x < kingBox.x) {
				//if the king is threatened from left we only add the pawn moves that are straight up and down.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightLeftThreatened) == KingThreatenedInfo::straightLeftThreatened) {
					if (currentKingInfo.straightLeftBox.x < pawnBox.x) {
						addAll = false;
					}

				}
			}

		}
		else if (inSameDiagonal(pawnBox, kingBox)) {
			//if the pawn is above the king.
			if (pawnBox.y < kingBox.y) {

				//if the king is attacked diagonally, we can only move the pawn to the top right.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upRightThreatened) == KingThreatenedInfo::upRightThreatened) {
					if (currentKingInfo.upRightBox.x > pawnBox.x) {
						addUpRightPawnMoves(pawnBox, currentBoardState, currentLegalMoves);
						addAll = false;
					}

				}
			}
			//if the pawn is below the king
			if (pawnBox.y > kingBox.y) {
				//if the king is attacked diagonally from below we can't move the pawn.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downLeftThreatened) == KingThreatenedInfo::downLeftThreatened) {
					if (currentKingInfo.downLeftBox.x < pawnBox.x) {
						addAll = false;
					}

				}
			}
		}
		else if (inSameReverseDiagonal(pawnBox, kingBox)) {
			//	std::cout << "Pawn is in same reverse diagonal: " << pawnBox.x << " " << pawnBox.y << std::endl;

				//if the pawn is above the king.
			if (pawnBox.y < kingBox.y) {
				//std::cout << "Pawn is above king" << std::endl;
				//if the king is attacked diagonally, we can't move the pawn at all.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upLeftThreatened) == KingThreatenedInfo::upLeftThreatened) {
					//	std::cout << "up left threatened matches" << std::endl;
					if (currentKingInfo.upLeftBox.x < pawnBox.x) {
						//	std::cout << "adding pawn: " << pawnBox.x << " " << pawnBox.y << " Current turn is: " << currentTurn << std::endl;
						addUpLeftPawnMoves(pawnBox, currentBoardState, currentLegalMoves);
						addAll = false;
					}
					//	std::cout << "King is threatened!!!" << std::endl;
				}
				else {
					//std::cout << "up left doesn't match" << std::endl;
				}
			}
			//if the pawn is below the king
			if (pawnBox.y > kingBox.y) {
				//if the king is attacked diagonally from below, we can't move the pawn.` 
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downRightThreatened) == KingThreatenedInfo::downRightThreatened) {
					if (currentKingInfo.downRightBox.x > pawnBox.x) {
						addAll = false;
					}

				}
			}
		}
		else {
			//std::cout << "Pawn si good: " << pawnBox.x << " " << pawnBox.y << std::endl;
		}

		//if it's not in any of the things, we can just add on all the possible pawn moves. 
		if (addAll) {
			calculatePawnUpMoves(pawnBox, currentBoardState, currentLegalMoves);
		}
	}
	//if it's blacks turn.
	else {
		if (inSameCol(pawnBox, kingBox)) {

			//if the pawn is above the king.
			if (pawnBox.y < kingBox.y) {
				//if the king is threatened from above we only add the pawn moves that are straight up and down.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightUpThreatened) == KingThreatenedInfo::straightUpThreatened) {
					if (currentKingInfo.straightUpBox.y < pawnBox.y) {
						addStraightDownPawnMoves(pawnBox, currentBoardState, currentLegalMoves);
						addAll = false;
					}

				}


			}
			//the pawn is below the king.
			else if (pawnBox.y > kingBox.y) {
				//if the king is threatened from below we only add moves that add the pawn up and down.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightDownThreatened) == KingThreatenedInfo::straightDownThreatened) {
					if (currentKingInfo.straightDownBox.y > pawnBox.y) {
						addStraightDownPawnMoves(pawnBox, currentBoardState, currentLegalMoves);
						addAll = false;
					}

				}
			}
		}
		//if it's in the same row and i's attacked we reallllly can't move it. 
		else if (inSameRow(pawnBox, kingBox)) {
			//pawn is to the right of the king.
			if (pawnBox.x > kingBox.x) {
				//if the king is threatened from right we only add the pawn moves that are straight up and down.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightRightThreatened) == KingThreatenedInfo::straightRightThreatened) {
					if (currentKingInfo.straightRightBox.x > pawnBox.x) {
						addAll = false;
					}

				}

			}
			else if (pawnBox.x < kingBox.x) {
				//if the king is threatened from left we only add the pawn moves that are straight up and down.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightLeftThreatened) == KingThreatenedInfo::straightLeftThreatened) {
					if (currentKingInfo.straightLeftBox.x < pawnBox.x) {
						addAll = false;
					}

				}
			}

		}
		else if (inSameDiagonal(pawnBox, kingBox)) {
			//if the pawn is above the king.
			if (pawnBox.y < kingBox.y) {

				//if the king is attacked diagonally, we can only move the pawn to the bottom left.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upRightThreatened) == KingThreatenedInfo::upRightThreatened) {
					if (currentKingInfo.upRightBox.x > pawnBox.x) {
						addAll = false;
					}

				}
			}
			//if the pawn is below the king
			if (pawnBox.y > kingBox.y) {
				//if the king is attacked diagonally from below we can't move the pawn.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downLeftThreatened) == KingThreatenedInfo::downLeftThreatened) {
					if (currentKingInfo.downLeftBox.x < pawnBox.x) {
						addDownLeftPawnMoves(pawnBox, currentBoardState, currentLegalMoves);
						addAll = false;
					}

				}
			}
		}
		else if (inSameReverseDiagonal(pawnBox, kingBox)) {
			//if the pawn is above the king.
			if (pawnBox.y < kingBox.y) {

				//if the king is attacked diagonally, we can't move the pawn at all.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upLeftThreatened) == KingThreatenedInfo::upLeftThreatened) {
					if (currentKingInfo.upLeftBox.x < pawnBox.x) {
						addAll = false;
					}

				}
			}
			//if the pawn is below the king
			if (pawnBox.y > kingBox.y) {
				//if the king is attacked diagonally from below, we can't move the pawn.
				if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downRightThreatened) == KingThreatenedInfo::downRightThreatened) {
					if (currentKingInfo.downRightBox.x > pawnBox.x) {
						addDownRightPawnMoves(pawnBox, currentBoardState, currentLegalMoves);
						addAll = false;
					}

				}
			}
		}

		//if it's not in any of the things, we can just add on all the possible pawn moves. 
		if (addAll) {
			calculatePawnDownMoves(pawnBox, currentBoardState, currentLegalMoves);
		}
	}


}


void Board::calculatePawnDownMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	addStraightDownPawnMoves(box, currentBoardState, moves);
	addDownLeftPawnMoves(box, currentBoardState, moves);
	addDownRightPawnMoves(box, currentBoardState, moves);
}
void Board::calculatePawnUpMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	addStraightUpPawnMoves(box, currentBoardState, moves);
	addUpLeftPawnMoves(box, currentBoardState, moves);
	addUpRightPawnMoves(box, currentBoardState, moves);
}




//directly adds rook moves to the legal moves. 
void Board::calculateRookLegalMoves(Box rookBox, Box kingBox, BoardState* currentBoardState, std::vector<Move>& currentLegalMoves, KingThreatenedInfo currentKingInfo) {
	//programming this assuming the king is not in check.
	bool addAll = true;
	if (inSameCol(rookBox, kingBox)) {

		//if the rook is above the king.
		if (rookBox.y < kingBox.y) {
			//if the king is threatened from above we only add the rook moves that are straight up and down.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightUpThreatened) == KingThreatenedInfo::straightUpThreatened) {
				if (currentKingInfo.straightUpBox.y < rookBox.y) {
					addStraightDownMoves(rookBox, currentBoardState, currentLegalMoves);
					addStraightUpMoves(rookBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}


		}
		//the rook is below the king.
		else if (rookBox.y > kingBox.y) {
			//if the king is threatened from below we only add moves that add the rook up and down.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightDownThreatened) == KingThreatenedInfo::straightDownThreatened) {
				if (currentKingInfo.straightDownBox.y > rookBox.y) {
					addStraightDownMoves(rookBox, currentBoardState, currentLegalMoves);
					addStraightUpMoves(rookBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
	}
	else if (inSameRow(rookBox, kingBox)) {
		//rook is to the right of the king.
		if (rookBox.x > kingBox.x) {
			//if the king is threatened from right we only add the rook moves that are straight up and down.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightRightThreatened) == KingThreatenedInfo::straightRightThreatened) {
				if (currentKingInfo.straightRightBox.x > rookBox.x) {
					addStraightRightMoves(rookBox, currentBoardState, currentLegalMoves);
					addStraightLeftMoves(rookBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}
			}

		}
		else if (rookBox.x < kingBox.x) {
			//if the king is threatened from left we only add the rook moves that are straight up and down.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightLeftThreatened) == KingThreatenedInfo::straightLeftThreatened) {
				if (currentKingInfo.straightLeftBox.x < rookBox.x) {
					addStraightRightMoves(rookBox, currentBoardState, currentLegalMoves);
					addStraightLeftMoves(rookBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}

	}
	else if (inSameDiagonal(rookBox, kingBox)) {
		//if the rook is above the king.
		if (rookBox.y < kingBox.y) {

			//if the king is attacked diagonally, we can't move the rook at all.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upRightThreatened) == KingThreatenedInfo::upRightThreatened) {
				if (currentKingInfo.upRightBox.x > rookBox.x) {
					addAll = false;
				}

			}
		}
		//if the rook is below the king
		if (rookBox.y > kingBox.y) {
			//if the king is attacked diagonally, we can't move the rook at all.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downLeftThreatened) == KingThreatenedInfo::downLeftThreatened) {
				if (currentKingInfo.downLeftBox.x < rookBox.x) {
					addAll = false;
				}

			}
		}
	}
	else if (inSameReverseDiagonal(rookBox, kingBox)) {
		//if the rook is above the king.
		if (rookBox.y < kingBox.y) {

			//if the king is attacked diagonally, we can't move the rook at all.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upLeftThreatened) == KingThreatenedInfo::upLeftThreatened) {
				if (currentKingInfo.upLeftBox.x < rookBox.x) {
					addAll = false;
				}

			}
		}
		//if the rook is below the king
		if (rookBox.y > kingBox.y) {
			//if the king is attacked diagonally, we can't move the rook at all.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downRightThreatened) == KingThreatenedInfo::downRightThreatened) {
				if (currentKingInfo.downRightBox.x > rookBox.x) {
					addAll = false;
				}

			}
		}
	}

	//if it's not in any of the things, we can just add on all the possible rook moves. 
	if (addAll) {
		calculateRookMoves(rookBox, currentBoardState, currentLegalMoves);
	}




}

void Board::calculateBishopLegalMoves(Box bishopBox, Box kingBox, BoardState* currentBoardState, std::vector<Move>& currentLegalMoves, KingThreatenedInfo currentKingInfo) {
	//programming this assuming the king is not in check.
	bool addAll = true;
	if (inSameCol(bishopBox, kingBox)) {

		//if the bishop is above the king.
		if (bishopBox.y < kingBox.y) {
			//if the king is threatened from above we don't add bishop moves.
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightUpThreatened) == KingThreatenedInfo::straightUpThreatened) {
				if (currentKingInfo.straightUpBox.y < bishopBox.y) {
					addAll = false;
				}

			}


		}
		//the bishop is below the king.
		else if (bishopBox.y > kingBox.y) {
			//no bishop moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightDownThreatened) == KingThreatenedInfo::straightDownThreatened) {
				if (currentKingInfo.straightDownBox.y > bishopBox.y) {
					addAll = false;
				}

			}
		}
	}
	else if (inSameRow(bishopBox, kingBox)) {
		//bishop is to the right
		if (bishopBox.x > kingBox.x) {
			//no bishop moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightRightThreatened) == KingThreatenedInfo::straightRightThreatened) {
				if (currentKingInfo.straightRightBox.x > bishopBox.x) {
					addAll = false;
				}

			}

		}
		else if (bishopBox.x < kingBox.x) {
			//add no bishop moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightLeftThreatened) == KingThreatenedInfo::straightLeftThreatened) {
				if (currentKingInfo.straightLeftBox.x < bishopBox.x) {
					addAll = false;
				}

			}
		}

	}
	else if (inSameDiagonal(bishopBox, kingBox)) {
		//if the rook is above the king.
		if (bishopBox.y < kingBox.y) {


			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upRightThreatened) == KingThreatenedInfo::upRightThreatened) {

				if (currentKingInfo.upRightBox.x > bishopBox.x) {
					addUpRightMoves(bishopBox, currentBoardState, currentLegalMoves);
					addDownLeftMoves(bishopBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
		//if the rook is below the king
		if (bishopBox.y > kingBox.y) {

			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downLeftThreatened) == KingThreatenedInfo::downLeftThreatened) {
				if (currentKingInfo.downLeftBox.x < bishopBox.x) {
					addUpRightMoves(bishopBox, currentBoardState, currentLegalMoves);
					addDownLeftMoves(bishopBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
	}
	else if (inSameReverseDiagonal(bishopBox, kingBox)) {
		//if the bishop is above the king
		if (bishopBox.y < kingBox.y) {


			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upLeftThreatened) == KingThreatenedInfo::upLeftThreatened) {
				if (currentKingInfo.upLeftBox.x < bishopBox.x) {
					addUpLeftMoves(bishopBox, currentBoardState, currentLegalMoves);
					addDownRightMoves(bishopBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
		//if the rook is below the king
		if (bishopBox.y > kingBox.y) {

			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downRightThreatened) == KingThreatenedInfo::downRightThreatened) {
				if (currentKingInfo.downRightBox.x > bishopBox.x) {
					addUpLeftMoves(bishopBox, currentBoardState, currentLegalMoves);
					addDownRightMoves(bishopBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
	}

	//if it's not in any of the things, we can just add on all the possible rook moves. 
	if (addAll) {
		calculateBishopMoves(bishopBox, currentBoardState, currentLegalMoves);
	}
}

void Board::calculateQueenLegalMoves(Box queenBox, Box kingBox, BoardState* currentBoardState, std::vector<Move>& currentLegalMoves, KingThreatenedInfo currentKingInfo) {

	//programming this assuming the king is not in check.
	bool addAll = true;
	if (inSameCol(queenBox, kingBox)) {

		//if the queen is above the king.
		if (queenBox.y < kingBox.y) {

			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightUpThreatened) == KingThreatenedInfo::straightUpThreatened) {
				if (currentKingInfo.straightUpBox.y < queenBox.y) {
					addStraightUpMoves(queenBox, currentBoardState, currentLegalMoves);
					addStraightDownMoves(queenBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}


		}
		//the queen is below the king.
		else if (queenBox.y > kingBox.y) {
			//no queen moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightDownThreatened) == KingThreatenedInfo::straightDownThreatened) {
				if (currentKingInfo.straightDownBox.y > queenBox.y) {
					addStraightUpMoves(queenBox, currentBoardState, currentLegalMoves);
					addStraightDownMoves(queenBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
	}
	else if (inSameRow(queenBox, kingBox)) {
		//queen is to the right
		if (queenBox.x > kingBox.x) {
			//no queen moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightRightThreatened) == KingThreatenedInfo::straightRightThreatened) {
				if (currentKingInfo.straightRightBox.x > queenBox.x) {
					addStraightLeftMoves(queenBox, currentBoardState, currentLegalMoves);
					addStraightRightMoves(queenBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}

		}
		else if (queenBox.x < kingBox.x) {
			//add no queen moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightLeftThreatened) == KingThreatenedInfo::straightLeftThreatened) {
				if (currentKingInfo.straightLeftBox.x < queenBox.x) {
					addStraightLeftMoves(queenBox, currentBoardState, currentLegalMoves);
					addStraightRightMoves(queenBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}

	}
	else if (inSameDiagonal(queenBox, kingBox)) {
		//queen above king
		if (queenBox.y < kingBox.y) {


			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upRightThreatened) == KingThreatenedInfo::upRightThreatened) {
				if (currentKingInfo.upRightBox.x > queenBox.x) {
					addUpRightMoves(queenBox, currentBoardState, currentLegalMoves);
					addDownLeftMoves(queenBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
		//king below king
		if (queenBox.y > kingBox.y) {

			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downLeftThreatened) == KingThreatenedInfo::downLeftThreatened) {
				if (currentKingInfo.downLeftBox.x < queenBox.x) {
					addUpRightMoves(queenBox, currentBoardState, currentLegalMoves);
					addDownLeftMoves(queenBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
	}
	else if (inSameReverseDiagonal(queenBox, kingBox)) {
		//if the queen is above the king
		if (queenBox.y < kingBox.y) {


			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upLeftThreatened) == KingThreatenedInfo::upLeftThreatened) {
				if (currentKingInfo.upLeftBox.x < queenBox.x) {
					addUpLeftMoves(queenBox, currentBoardState, currentLegalMoves);
					addDownRightMoves(queenBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}

			}
		}
		//if the queen is below the king
		if (queenBox.y > kingBox.y) {

			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downRightThreatened) == KingThreatenedInfo::downRightThreatened) {
				if (currentKingInfo.downRightBox.x > queenBox.x) {
					addUpLeftMoves(queenBox, currentBoardState, currentLegalMoves);
					addDownRightMoves(queenBox, currentBoardState, currentLegalMoves);
					addAll = false;
				}
			}
		}
	}

	//if it's not in any of the things, we can just add on all the possible rook moves. 
	if (addAll) {

		calculateQueenMoves(queenBox, currentBoardState, currentLegalMoves);
	}
}



void Board::calculateKingLegalMoves(Box kingBox, BoardState* currentBoardState, std::vector<Move>& currentLegalMoves, KingThreatenedInfo currentKingInfo) {
	int x = kingBox.x;
	int y = kingBox.y;
	uint8_t** board = currentBoardState->getBoard();
	uint8_t currentPlayerPieceType = (currentBoardState->getCurrentTurn() == 'w') ? Piece::white : Piece::black;

	for (int change = -1; change <= 1; ++change) {
		if (y + 1 < boardYBoxes) {
			if (x + change < boardXBoxes && x + change >= 0) {
				if ((board[x + change][y + 1] & currentPlayerPieceType) != currentPlayerPieceType) {
					if (!squareAttacked({ x + change,y + 1 }, currentBoardState)) {
						currentLegalMoves.push_back({ { x,y }, { x + change,y + 1 }, false, false, false, false, ' ' });
					}
				}
			}
		}
		if (y - 1 >= 0) {
			if (x + change < boardXBoxes && x + change >= 0) {
				if ((board[x + change][y - 1] & currentPlayerPieceType) != currentPlayerPieceType) {
					if (!squareAttacked({ x + change,y - 1 }, currentBoardState)) {
						currentLegalMoves.push_back({ { x,y }, { x + change,y - 1 }, false, false, false, false, ' ' });
					}
				}
			}
		}

	}
	if (x + 1 < boardXBoxes) {
		if (!squareAttacked({ x + 1,y }, currentBoardState)) {
			if ((board[x + 1][y] & currentPlayerPieceType) != currentPlayerPieceType) {
				currentLegalMoves.push_back({ { x,y }, { x + 1,y}, false, false, false, false, ' ' });
			}
		}
	}
	if (x - 1 >= 0) {
		if ((board[x - 1][y] & currentPlayerPieceType) != currentPlayerPieceType) {
			if (!squareAttacked({ x - 1,y }, currentBoardState)) {
				currentLegalMoves.push_back({ { x,y }, { x - 1, y}, false, false, false, false, ' ' });
			}
		}
	}
	calculateCastlingLegalMoves(kingBox, currentBoardState, currentLegalMoves);


}


void Board::calculateCastlingLegalMoves(Box kingBox, BoardState* currentBoardState, std::vector<Move>& currentLegalMoves) {
	int x = kingBox.x;
	int y = kingBox.y;
	uint8_t** board = currentBoardState->getBoard();
	int attackedAmount;
	char currentTurn = currentBoardState->getCurrentTurn();
	if (currentTurn == 'w') {
		attackedAmount = whiteThreatened.amountAttacked;
	}
	else {
		attackedAmount = blackThreatened.amountAttacked;
	}
	//if the king is attacked you can't castle. 
	if (attackedAmount != 0) {
		return;
	}

	if ((board[x][y] & Piece::white) == Piece::white) {
		if (currentBoardState->getWhiteCanKingsideCastle()) {
			if (board[x + 1][y] == 0 && board[x + 2][y] == 0) {
				if (!squareAttacked({ x + 1,y }, currentBoardState) && !squareAttacked({ x + 2,y }, currentBoardState)) {
					currentLegalMoves.push_back({ {x,y},{x + 2,y},true,false,false });
				}
			}
		}
		if (currentBoardState->getWhiteCanQueensideCastle()) {
			if (board[x - 1][y] == 0 && board[x - 2][y] == 0 && board[x-3][y] == 0) {
				if (!squareAttacked({ x - 1,y }, currentBoardState) && !squareAttacked({ x - 2,y }, currentBoardState)) {
					currentLegalMoves.push_back({ {x,y},{x - 2,y},false,true,false });
				}
			}
		}
	}
	else {
		if (currentBoardState->getBlackCanKingsideCastle()) {
			if (board[x + 1][y] == 0 && board[x + 2][y] == 0) {
				if (!squareAttacked({ x + 1,y }, currentBoardState) && !squareAttacked({ x + 2,y }, currentBoardState)) {
					currentLegalMoves.push_back({ {x,y},{x + 2,y},true,false,false });
				}
			}
		}
		if (currentBoardState->getBlackCanQueensideCastle()) {
			if (board[x - 1][y] == 0 && board[x - 2][y] == 0 && board[x - 3][y] == 0) {
				if (!squareAttacked({ x - 1,y }, currentBoardState) && !squareAttacked({ x - 2,y }, currentBoardState)) {
					currentLegalMoves.push_back({ {x,y},{x - 2,y},false,true,false });
				}
			}
		}
	}
}

void Board::calculateKnightLegalMoves(Box knightBox, Box kingBox, BoardState* currentBoardState, std::vector<Move>& currentLegalMoves, KingThreatenedInfo currentKingInfo) {
	//programming this assuming the king is not in check.
	bool addAll = true;
	if (inSameCol(knightBox, kingBox)) {

		//if the knight is above the king.
		if (knightBox.y < kingBox.y) {

			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightUpThreatened) == KingThreatenedInfo::straightUpThreatened) {
				if (currentKingInfo.straightUpBox.y < knightBox.y) {
					addAll = false;
				}

			}


		}
		//the knight is below the king.
		else if (knightBox.y > kingBox.y) {
			//no knight moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightDownThreatened) == KingThreatenedInfo::straightDownThreatened) {
				if (currentKingInfo.straightDownBox.y > knightBox.y) {
					addAll = false;
				}

			}
		}
	}
	else if (inSameRow(knightBox, kingBox)) {
		//knight is to the right
		if (knightBox.x > kingBox.x) {
			//no knight moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightRightThreatened) == KingThreatenedInfo::straightRightThreatened) {
				if (currentKingInfo.straightRightBox.x > knightBox.x) {
					addAll = false;
				}

			}

		}
		else if (knightBox.x < kingBox.x) {
			//add no knight moves
			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::straightLeftThreatened) == KingThreatenedInfo::straightLeftThreatened) {
				if (currentKingInfo.straightLeftBox.x < knightBox.x) {
					addAll = false;
				}
			}
		}

	}
	else if (inSameDiagonal(knightBox, kingBox)) {
		//knight above king
		if (knightBox.y < kingBox.y) {


			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upRightThreatened) == KingThreatenedInfo::upRightThreatened) {
				if (currentKingInfo.upRightBox.x > knightBox.x) {
					addAll = false;
				}

			}
		}
		//knight below king
		if (knightBox.y > kingBox.y) {

			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downLeftThreatened) == KingThreatenedInfo::downLeftThreatened) {
				if (currentKingInfo.downLeftBox.x < knightBox.x) {
					addAll = false;
				}

			}
		}
	}
	else if (inSameReverseDiagonal(knightBox, kingBox)) {
		//if the knight is above the king
		if (knightBox.y < kingBox.y) {


			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::upLeftThreatened) == KingThreatenedInfo::upLeftThreatened) {
				if (currentKingInfo.upLeftBox.x < knightBox.x) {
					addAll = false;
				}

			}
		}
		//if the knight is below the king
		if (knightBox.y > kingBox.y) {

			if ((currentKingInfo.threatenedInfo & KingThreatenedInfo::downRightThreatened) == KingThreatenedInfo::downRightThreatened) {
				if (currentKingInfo.downRightBox.x > knightBox.x) {
					addAll = false;
				}

			}
		}
	}

	//if it's not in any of the things, we can just add on all the possible knight moves. 
	if (addAll) {
		calculateKnightMoves(knightBox, currentBoardState, currentLegalMoves);
	}
}





void Board::calculateRookMoves(Box box, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {

	addStraightRightMoves(box, currentBoardState, currentPseudo);
	addStraightLeftMoves(box, currentBoardState, currentPseudo);

	//going up on the board
	addStraightUpMoves(box, currentBoardState, currentPseudo);
	addStraightDownMoves(box, currentBoardState, currentPseudo);
	//going down the board



}

void Board::calculateBishopMoves(Box box, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {

	addDownRightMoves(box, currentBoardState, currentPseudo);
	addDownLeftMoves(box, currentBoardState, currentPseudo);
	addUpLeftMoves(box, currentBoardState, currentPseudo);

	addUpRightMoves(box, currentBoardState, currentPseudo);


}
void Board::calculateQueenMoves(Box box, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {


	addDownRightMoves(box, currentBoardState, currentPseudo);
	addDownLeftMoves(box, currentBoardState, currentPseudo);
	addUpLeftMoves(box, currentBoardState, currentPseudo);

	addUpRightMoves(box, currentBoardState, currentPseudo);
	addStraightRightMoves(box, currentBoardState, currentPseudo);
	addStraightLeftMoves(box, currentBoardState, currentPseudo);

	//going up on the board
	addStraightUpMoves(box, currentBoardState, currentPseudo);
	addStraightDownMoves(box, currentBoardState, currentPseudo);
	//going down the board

}
void Board::calculateKingMoves(Box box, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {
	int x = box.x;
	int y = box.y;
	uint8_t** board = currentBoardState->getBoard();
	//king moving down
	if (y + 1 < boardYBoxes) {
		for (int xChange = -1; xChange <= 1; xChange++) {
			if (x + xChange < 0 || x + xChange >= boardXBoxes) {
				continue;
			}
			if (board[x + xChange][y + 1] == 0 || !pieceIsCurrentPlayersPiece(x + xChange, y + 1, currentBoardState)) {
				currentPseudo.push_back({ {x,y},{x + xChange,y + 1}, false, false, false });
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
				currentPseudo.push_back({ {x,y},{x + xChange,y - 1}, false, false, false });
			}
		}
	}

	//king moving to the left
	if (x - 1 >= 0) {
		if (board[x - 1][y] == 0 || !pieceIsCurrentPlayersPiece(x - 1, y, currentBoardState)) {
			currentPseudo.push_back({ {x,y},{x - 1,y}, false, false, false });
		}
	}
	//king moving to the right
	if (x + 1 < boardXBoxes) {
		if (board[x + 1][y] == 0 || !pieceIsCurrentPlayersPiece(x + 1, y, currentBoardState)) {
			currentPseudo.push_back({ {x,y},{x + 1,y}, false, false, false });
		}
	}

	//add castling NOW
	calculateCastlingMoves(box, currentBoardState, currentPseudo);

}

//just to separate this a bit from the king moves, because it's kinda busy.
void Board::calculateCastlingMoves(Box box, BoardState* currentBoardState, std::vector<Move>& currentPseudo) {


}

//update castling status.
void Board::updateCastling(Box fromBox, Box toBox, BoardState* currentBoardState) {
	//if the king moved.
	int queenSideX = 0;
	int kingSideX = boardXBoxes - 1;
	int whiteY = boardYBoxes - 1;
	int blackY = 0;

	uint8_t** board = boardState->getBoard();
	if (currentBoardState->getCurrentTurn() == 'w') {
		if ((board[fromBox.x][fromBox.y] & Piece::king) == Piece::king) {
			currentBoardState->setWhiteCanKingsideCastle(false);
			currentBoardState->setWhiteCanQueensideCastle(false);
			currentBoardState->setWhiteCanQueensideCastle(false);


		}
		if (fromBox.x == queenSideX && fromBox.y == whiteY) {
			currentBoardState->setWhiteCanQueensideCastle(false);

		}
		else if (fromBox.x == kingSideX && fromBox.y == whiteY) {
			currentBoardState->setWhiteCanKingsideCastle(false);

		}
		if (toBox.x == kingSideX && toBox.y == blackY) {
			currentBoardState->setBlackCanKingsideCastle(false);

		}
		else if (toBox.x == queenSideX && toBox.y == blackY) {
			currentBoardState->setBlackCanQueensideCastle(false);

		}
	}
	else {
		if ((board[fromBox.x][fromBox.y] & Piece::king) == Piece::king) {
			currentBoardState->setBlackCanKingsideCastle(false);
			currentBoardState->setBlackCanQueensideCastle(false);

		}
		if (fromBox.x == queenSideX && fromBox.y == blackY) {
			currentBoardState->setBlackCanQueensideCastle(false);

		}
		else if (fromBox.x == kingSideX && fromBox.y == blackY) {
			currentBoardState->setBlackCanKingsideCastle(false);

		}
		//if the move takes a rook.
		if (toBox.x == kingSideX && toBox.y == whiteY) {
			currentBoardState->setWhiteCanKingsideCastle(false);

		}
		else if (toBox.x == queenSideX && toBox.y == whiteY) {
			currentBoardState->setWhiteCanQueensideCastle(false);

		}
	}


}


void Board::calculateKnightMoves(Box box, BoardState* currentBoardState, std::vector<Move>& moves) {
	int x = box.x;
	int y = box.y;

	//knight moving up.
	uint8_t** board = currentBoardState->getBoard();
	if (y - 2 >= 0) {
		if (x + 1 < boardXBoxes) {
			if (board[x + 1][y - 2] == 0 || !pieceIsCurrentPlayersPiece(x + 1, y - 2, currentBoardState)) {
				attemptAddMove({ {x, y}, {x + 1, y - 2}, false, false, false,false,' ' }, currentBoardState, moves);
			}
		}
		if (x - 1 >= 0) {
			if (board[x - 1][y - 2] == 0 || !pieceIsCurrentPlayersPiece(x - 1, y - 2, currentBoardState)) {
				attemptAddMove({ {x, y}, {x - 1, y - 2}, false, false, false,false,' ' }, currentBoardState, moves);
			}
		}
	}
	//knight moving down
	if (y + 2 < boardYBoxes) {
		if (x + 1 < boardXBoxes) {
			if (board[x + 1][y + 2] == 0 || !pieceIsCurrentPlayersPiece(x + 1, y + 2, currentBoardState)) {
				attemptAddMove({ {x, y}, {x + 1, y + 2}, false, false, false,false,' ' }, currentBoardState, moves);
			}
		}
		if (x - 1 >= 0) {
			if (board[x - 1][y + 2] == 0 || !pieceIsCurrentPlayersPiece(x - 1, y + 2, currentBoardState)) {
				attemptAddMove({ {x, y}, {x - 1, y + 2}, false, false, false,false,' ' }, currentBoardState, moves);
			}
		}
	}
	//knight moving left
	if (x - 2 >= 0) {
		if (y + 1 < boardYBoxes) {
			if (board[x - 2][y + 1] == 0 || !pieceIsCurrentPlayersPiece(x - 2, y + 1, currentBoardState)) {
				attemptAddMove({ {x, y}, {x - 2, y + 1}, false, false, false,false,' ' }, currentBoardState, moves);
			}
		}
		if (y - 1 >= 0) {
			if (board[x - 2][y - 1] == 0 || !pieceIsCurrentPlayersPiece(x - 2, y - 1, currentBoardState)) {
				attemptAddMove({ {x, y}, {x - 2, y - 1}, false, false, false,false,' ' }, currentBoardState, moves);
			}
		}
	}
	//knight moving right
	if (x + 2 < boardXBoxes) {
		if (y + 1 < boardYBoxes) {
			if (board[x + 2][y + 1] == 0 || !pieceIsCurrentPlayersPiece(x + 2, y + 1, currentBoardState)) {
				attemptAddMove({ {x, y}, {x + 2, y + 1}, false, false, false,false,' ' }, currentBoardState, moves);
			}
		}
		if (y - 1 >= 0) {
			if (board[x + 2][y - 1] == 0 || !pieceIsCurrentPlayersPiece(x + 2, y - 1, currentBoardState)) {
				attemptAddMove({ {x, y}, {x + 2, y - 1}, false, false, false,false,' ' }, currentBoardState, moves);
			}
		}
	}



}


void Board::renderHighlightMoves() {

	for (int i = 0; i < highlightBoxes.size(); i++) {
		renderBox(highlightBoxes.at(i), HIGHLIGHT_COLOR);

	}
}

void Board::createHighlightMoves(int x, int y) {
	highlightBoxes.clear();
	for (int i = 0; i < legalMoves.size(); i++) {
		if (legalMoves.at(i).fromBox.x == x &&
			legalMoves.at(i).fromBox.y == y) {
			highlightBoxes.push_back(

				{ legalMoves.at(i).toBox.x,
				legalMoves.at(i).toBox.y }
			);
		}
	}
}


void Board::updateEnPassant(Box fromBox, Box toBox, BoardState* currentBoardState) {

	if ((currentBoardState->getBoard()[fromBox.x][fromBox.y] & Piece::pawn) == Piece::pawn) {

		if (abs(fromBox.y - toBox.y) == 2) {

			currentBoardState->setEnPassantX(fromBox.x);
			currentBoardState->setEnPassantY((fromBox.y + toBox.y) / 2);

		}
		else {
			currentBoardState->setEnPassantX(-1);
			currentBoardState->setEnPassantY(-1);

		}
	}
	else {
		currentBoardState->setEnPassantX(-1);
		currentBoardState->setEnPassantY(-1);
		currentBoardState->setEnPassantY(-1);
	}
}

void Board::findKingLocation(int* xPos, int* yPos, BoardState* currentBoardState) {
	char currentPlayer = currentBoardState->getCurrentTurn();
	Box kingBox;
	if (currentPlayer == 'w') {
		kingBox = whiteLocations.getKingLocation();
	}
	else {
		kingBox = blackLocations.getKingLocation();
	}
	*xPos = kingBox.x;
	*yPos = kingBox.y;
}

bool Board::isEnPassant(Box fromBox, Box toBox, BoardState* currentBoardState) {

	if ((currentBoardState->getBoard()[fromBox.x][fromBox.y] & Piece::pawn) == Piece::pawn) {
		if (toBox.x == currentBoardState->getEnPassantX() && toBox.y == currentBoardState->getEnPassantY()) {

			return true;
		}
	}

	return false;
}

bool Board::squareAttacked(Box box, BoardState* currentBoardState) {
	int x = box.x;
	int y = box.y;
	char currentTurn = currentBoardState->getCurrentTurn();
	uint8_t** board = currentBoardState->getBoard();
	//pawns attack differently than they move.
	if (currentTurn == 'b') {
		if (y + 1 < boardYBoxes) {
			if (x - 1 >= 0) {
				if ((board[x - 1][y + 1]) == (Piece::pawn | Piece::white)) {

					return true;
				}
			}
			if (x + 1 < boardXBoxes) {
				if (board[x + 1][y + 1] == (Piece::pawn | Piece::white)) {

					return true;
				}
			}
		}
	}
	else {
		if (y - 1 >= 0) {
			if (x - 1 >= 0) {
				if (board[x - 1][y - 1] == (Piece::pawn | Piece::black)) {

					return true;
				}
			}
			if (x + 1 < boardXBoxes) {
				if (board[x + 1][y - 1] == (Piece::pawn | Piece::black)) {

					return true;
				}
			}
		}
	}

	uint8_t yourColor;
	uint8_t enemyColor;
	if (currentTurn == 'w') {
		enemyColor = Piece::black;
		yourColor = Piece::white;
	}
	else {
		enemyColor = Piece::white;
		yourColor = Piece::black;
	}

	//going to right
	if (x + 1 < boardXBoxes) {
		if (board[x + 1][y] == (enemyColor | Piece::king)) {
			return true;
		}
	}

	for (int currX = x + 1; currX < boardXBoxes; ++currX) {
		if (board[currX][y] == 0) {
			continue;
		}//if it's an enemy piece.
		else if (board[currX][y] == (enemyColor | Piece::queen) || board[currX][y] == (enemyColor | Piece::rook)) {
			return true;
		}
		else { //if it's your own piece.
			if ((board[currX][y] ) != (Piece::king | yourColor)) {
				break;
			}
			
		}
	}


	if (x - 1 >= 0) {
		if (board[x - 1][y] == (enemyColor | Piece::king)) {
			return true;
		}
	}

	//going to left
	for (int currX = x - 1; currX >= 0; --currX) {
		if (board[currX][y] == 0) {
			continue;
		}//if it's an enemy piece.
		else if (board[currX][y] == (enemyColor | Piece::queen) || board[currX][y] == (enemyColor | Piece::rook)) {
			return true;
		}
		else { //if it's your own piece.
			if (board[currX][y] != (Piece::king | yourColor)) {
				break;
			}
			
		}
	}

	if (y - 1 >= 0) {
		if (board[x][y - 1] == (enemyColor | Piece::king)) {
			return true;
		}
	}

	//going  up
	for (int currY = y - 1; currY >= 0; --currY) {
		if (board[x][currY] == 0) {
			continue;
		}//if it's an enemy piece.
		else if (board[x][currY] == (enemyColor | Piece::queen) || board[x][currY] == (enemyColor | Piece::rook)) {
			return true;
		}
		else { //if it's your own piece.
			if (board[x][currY] != (Piece::king | yourColor)) {
				break;
			}
		}
	}
	//going down
	if (y + 1 < boardYBoxes) {
		if (board[x][y + 1] == (enemyColor | Piece::king)) {
			return true;
		}
	}

	for (int currY = y + 1; currY < boardYBoxes; ++currY) {
		if (board[x][currY] == 0) {
			continue;
		}//if it's an enemy piece.
		else if (board[x][currY] == (enemyColor | Piece::queen) || board[x][currY] == (enemyColor | Piece::rook)) {
			return true;
		}
		else { //if it's your own piece.
			if (board[x][currY] != (Piece::king | yourColor)) {
				break;
			}
		}
	}


	int increase = 1;
	//going down to right
	if (x + increase < boardXBoxes && y + increase < boardYBoxes) {
		if (board[x + increase][y + increase] == (enemyColor | Piece::king)) {
			return true;
		}
	}

	while (x + increase < boardXBoxes && y + increase < boardYBoxes) {
		if (board[x + increase][y + increase] == 0) {
			++increase;
			continue;
		}//if it's an enemy piece.
		else if (board[x + increase][y + increase] == (enemyColor | Piece::queen) || board[x + increase][y + increase] == (enemyColor | Piece::bishop)) {
			return true;
		}
		else { //if it's your own piece.
			if (board[x+increase][y+increase] != (Piece::king | yourColor)) {
				break;
			}
		}
		++increase;
	}
	increase = 1;

	if (x - increase >= 0 && y + increase < boardYBoxes) {
		if (board[x - increase][y + increase] == (enemyColor | Piece::king)) {
			return true;
		}
	}

	//going down to left
	while (x - increase >= 0 && y + increase < boardYBoxes) {
		if (board[x - increase][y + increase] == 0) {
			++increase;
			continue;
		}//if it's an enemy piece.
		else if (board[x - increase][y + increase] == (enemyColor | Piece::queen) || board[x - increase][y + increase] == (enemyColor | Piece::bishop)) {
			return true;
		}
		else { //if it's your own piece.
			if (board[x - increase][y + increase] != (Piece::king | yourColor)) {
				break;
			}
		}
		++increase;
	}
	increase = 1;
	//going up to the left
	if (x - increase >= 0 && y - increase >= 0) {
		if (board[x - increase][y - increase] == (enemyColor | Piece::king)) {
			return true;
		}
	}
	while (x - increase >= 0 && y - increase >= 0) {
		if (board[x - increase][y - increase] == 0) {
			++increase;
			continue;
		}//if it's an enemy piece.
		else if (board[x - increase][y - increase] == (enemyColor | Piece::queen) || board[x - increase][y - increase] == (enemyColor | Piece::bishop)) {
			return true;
		}
		else { //if it's your own piece.
			if (board[x - increase][y - increase] != (Piece::king | yourColor)) {
				break;
			}
		}
		++increase;
	}
	increase = 1;
	//going up to the right

	if (x + increase < boardXBoxes && y - increase >= 0) {
		if (board[x + increase][y - increase] == (enemyColor | Piece::king)) {
			return true;
		}
	}
	while (x + increase < boardXBoxes && y - increase >= 0) {
		if (board[x + increase][y - increase] == 0) {
			++increase;
			continue;
		}//if it's an enemy piece.
		else if (board[x + increase][y - increase] == (enemyColor | Piece::queen) || board[x + increase][y - increase] == (enemyColor | Piece::bishop)) {
			return true;
		}
		else { //if it's your own piece.
			if (board[x + increase][y - increase] != (Piece::king | yourColor)) {
				break;
			}
		}
		++increase;
	}




	//check if there are any knights attacking the square.
	if (currentTurn == 'w') {
		for (int i = 0; i < blackLocations.getKnightLocations().size(); i++) {
			int xDiff = abs(blackLocations.getKnightLocations().at(i).x - box.x);
			int yDiff = abs(blackLocations.getKnightLocations().at(i).y - box.y);
			int sumOfLeftAndRight = xDiff + yDiff;
			//that means there's a square thats 2-1 between them.
			if (sumOfLeftAndRight == 3 && xDiff != 0 && yDiff != 0) {
				return true;
			}

		}
	}
	else {
		for (int i = 0; i < whiteLocations.getKnightLocations().size(); i++) {
			int xDiff = abs(whiteLocations.getKnightLocations().at(i).x - box.x);
			int yDiff = abs(whiteLocations.getKnightLocations().at(i).y - box.y);
			int sumOfLeftAndRight = xDiff + yDiff;
			//that means there's a square thats 2-1 between them.
			if (sumOfLeftAndRight == 3 && xDiff != 0 && yDiff != 0) {
				return true;
			}

		}
	}

	return false;
}

//figure out... is the king in check?
bool Board::kingInCheck(BoardState* currentBoardState) {
	//if the current players king is under attack then the king is in check.
	int kingX, kingY;
	findKingLocation(&kingX, &kingY, currentBoardState);

	bool result = squareAttacked({ kingX,kingY }, currentBoardState);


	return result;

}


void Board::updateHighlightKingBox() {
	//optimize this later, probably fine though.
	if (kingInCheck(boardState)) {

		findKingLocation(&highlightKingBox.x, &highlightKingBox.y, boardState);
	}
	else {
		highlightKingBox.x = highlightKingBox.y = -1;
	}
	if (gameOver) {

		switchTurns(boardState);
		findKingLocation(&winnerKing.x, &winnerKing.y, boardState);

	}


}

void Board::renderKingBox() {

	if (highlightKingBox.x != -1) {
		renderBox(highlightKingBox, ATTACK_COLOR);

	}
	if (winnerKing.x != -1) {
		renderBox(winnerKing, WIN_COLOR);

	}




}

int Board::isGameOver(BoardState* currentBoardState) {
	std::vector<Move> currentLegalMoves = calculateLegalMoves(currentBoardState);
	if (currentLegalMoves.size() == 0) {
		winner = (currentBoardState->getCurrentTurn() == 'w' ? 'b' : 'w');
		return 1;
	}
	else {
		return 0;
	}
}



void Board::calculateBoardStates() {
	for (int i = 1; i <= totalTests; i++) {
		int initialTime = SDL_GetTicks();
		std::cout << "Total Board states in "<< i <<" moves: " << totalPossibleFutureBoardPositions(boardState, i) <<std::endl;
		std::cout << "Took : " << SDL_GetTicks() - initialTime << " Milliseconds" << std::endl;
		initialTime = SDL_GetTicks();
	}
	
	
}

int Board::totalPossibleFutureBoardPositions(BoardState* currentBoardState, int depth) {
	int totalAmount = 0;
	if (depth == 0) {
		return 1;		//if we're at the end then this is a board state
	}
	std::vector<Move> legalMoves = calculateLegalMoves(currentBoardState);
	for (int i = 0; i < legalMoves.size(); i++) {


		makeMove(legalMoves.at(i), currentBoardState);

		int amountOfMoves = totalPossibleFutureBoardPositions(currentBoardState, depth - 1);
	 if (depth == totalTests){
			std::cout <<  char('a' + legalMoves.at(i).fromBox.x) << 8 - legalMoves.at(i).fromBox.y  << char('a' + legalMoves.at(i).toBox.x) << 8 - legalMoves.at(i).toBox.y << ": " << amountOfMoves << std::endl;
		}
		totalAmount += amountOfMoves;
		unMakeMove(currentBoardState);

	}
	return totalAmount;
}


void Board::makeRandomMove(BoardState* currentBoardState) {
	legalMoves = calculateLegalMoves(currentBoardState);
	int choice = rand() % legalMoves.size();
	makeMove(legalMoves.at(choice), currentBoardState);

	nextTurn(currentBoardState);
}

void Board::initializePieceLocations(BoardState* currentBoardState) {
	blackLocations.clear();
	whiteLocations.clear();
	for (int x = 0; x < boardXBoxes; x++) {
		for (int y = 0; y < boardYBoxes; y++) {
			if (currentBoardState->getBoard()[x][y] != 0) {
				uint8_t currentPiece = currentBoardState->getBoard()[x][y];

				if (currentPiece == (Piece::black | Piece::king)) {
					blackLocations.setKingLocation({ x,y });
				}
				else if (currentPiece == (Piece::black | Piece::queen)) {
					blackLocations.getQueenLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::black | Piece::bishop)) {
					blackLocations.getBishopLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::black | Piece::knight)) {
					blackLocations.getKnightLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::black | Piece::pawn)) {
					blackLocations.getPawnLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::black | Piece::rook)) {
					blackLocations.getRookLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::white | Piece::king)) {
					whiteLocations.setKingLocation({ x,y });
				}
				else if (currentPiece == (Piece::white | Piece::queen)) {
					whiteLocations.getQueenLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::white | Piece::bishop)) {
					whiteLocations.getBishopLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::white | Piece::knight)) {
					whiteLocations.getKnightLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::white | Piece::pawn)) {
					whiteLocations.getPawnLocations().push_back({ x,y });
				}
				else if (currentPiece == (Piece::white | Piece::rook)) {
					whiteLocations.getRookLocations().push_back({ x,y });
				}


			}
		}
	}


}

//don't worry about the length of this function.
//seriously don't. Just leave it alone please god.
bool Board::inSameRow(Box box1, Box box2) {
	return box1.y == box2.y;
}

bool Board::inSameCol(Box box1, Box box2) {
	return box1.x == box2.x;
}

bool Board::inSameDiagonal(Box box1, Box box2) {
	int increase = 1;
	if (box1.x < box2.x) {
		while (box1.x + increase <= box2.x) {
			if (box1.x + increase == box2.x && box1.y - increase == box2.y) {
				return true;
			}
			++increase;
		}
		return false;
	}
	else {
		while (box2.x + increase <= box1.x) {
			if (box2.x + increase == box1.x && box2.y - increase == box1.y) {
				return true;
			}
			++increase;
		}
		return false;
	}


}

bool Board::inSameReverseDiagonal(Box box1, Box box2) {
	int increase = 1;
	if (box1.x < box2.x) {
		while (box1.x + increase <= box2.x) {
			if (box1.x + increase == box2.x && box1.y + increase == box2.y) {
				return true;
			}
			++increase;
		}
		return false;
	}
	else {
		while (box2.x + increase <= box1.x) {
			if (box2.x + increase == box1.x && box2.y + increase == box1.y) {
				return true;
			}
			++increase;
		}
		return false;
	}
}