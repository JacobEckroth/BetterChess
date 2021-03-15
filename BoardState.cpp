#include "BoardState.h"
#include "Board.h"

unsigned int** BoardState::getBoard() {
	return board;
}

void BoardState::setBoard(unsigned int** newBoard) {
	board = newBoard;
}

void BoardState::setCurrentTurn(char newTurn) {
	currentTurn = newTurn;
}

char BoardState::getCurrentTurn() {
	return currentTurn;
}

bool BoardState::getWhiteCanKingsideCastle() {
	return whiteCanKingsideCastle;
}

void BoardState::setWhiteCanKingsideCastle(bool newSet) {
	whiteCanKingsideCastle = newSet;
}

void BoardState::setWhiteCanQueensideCastle(bool newSet) {
	whiteCanQueensideCastle = newSet;
}
bool BoardState::getWhiteCanQueensideCastle() {
	return whiteCanQueensideCastle;
}
bool BoardState::getBlackCanKingsideCastle() {
	return blackCanKingsideCastle;
}

void BoardState::setBlackCanKingsideCastle(bool newSet) {
	blackCanKingsideCastle = newSet;
}

void BoardState::setBlackCanQueensideCastle(bool newSet) {
	blackCanQueensideCastle = newSet;
}
bool BoardState::getBlackCanQueensideCastle() {
	return blackCanQueensideCastle;
}

int BoardState::getEnPassantX() {
	return enPassantX;
}

void BoardState::setEnPassantX(int newSet) {
	enPassantX = newSet;
}

int BoardState::getEnPassantY() {
	return enPassantY;
}

void BoardState::setEnPassantY(int newSet) {
	enPassantY = newSet;
}

int BoardState::getHalfMoveClock() {
	return halfMoveClock;
}
void BoardState::setHalfMoveClock(int newSet) {
	halfMoveClock = newSet;
}

int BoardState::getFullMoveClock() {
	return fullMoveClock;
}

void BoardState::setFullMoveCLock(int newSet) {
	fullMoveClock = newSet;
}

BoardState* BoardState::copyBoardState(BoardState* currentBoardState) {
	BoardState* newState = new BoardState();
	newState->currentTurn = currentBoardState->currentTurn;

	newState->whiteCanKingsideCastle = currentBoardState->whiteCanKingsideCastle;
	newState->whiteCanQueensideCastle = currentBoardState->whiteCanQueensideCastle;
	newState->blackCanKingsideCastle = currentBoardState->blackCanKingsideCastle;
	newState->blackCanQueensideCastle = currentBoardState->blackCanQueensideCastle;
	newState->enPassantX = currentBoardState->enPassantX;
	newState->enPassantY = currentBoardState->enPassantY;
	newState->halfMoveClock = currentBoardState->halfMoveClock;
	newState->fullMoveClock = currentBoardState->fullMoveClock;
	newState->board = new unsigned int* [Board::boardXBoxes];
	for (int x = 0; x < Board::boardXBoxes; x++) {
		(newState->board)[x]= new unsigned int[Board::boardYBoxes];
	}
	for (int x = 0; x < Board::boardXBoxes; x++) {
		for (int y = 0; y < Board::boardYBoxes; y++) {
			(newState->board)[x][y] = (currentBoardState->board)[x][y];
		}
	}
	return newState;
}





