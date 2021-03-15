#pragma once
class BoardState
{
public:
	
	unsigned int** getBoard();
	void setBoard(unsigned int**);


	void setCurrentTurn(char);
	char getCurrentTurn();
	bool getWhiteCanKingsideCastle();
	void setWhiteCanKingsideCastle(bool);
	bool getWhiteCanQueensideCastle();
	void setWhiteCanQueensideCastle(bool);
	bool getBlackCanKingsideCastle();
	void setBlackCanKingsideCastle(bool);
	bool getBlackCanQueensideCastle();
	void setBlackCanQueensideCastle(bool);

	int getEnPassantX();
	void setEnPassantX(int);

	int getEnPassantY();
	void setEnPassantY(int);

	int getHalfMoveClock();
	void setHalfMoveClock(int);

	int getFullMoveClock();
	void setFullMoveCLock(int);

	static BoardState* copyBoardState(BoardState*);
	
private:
	unsigned int** board;
	char currentTurn;
	bool whiteCanKingsideCastle;
	bool whiteCanQueensideCastle;
	bool blackCanKingsideCastle;
	bool blackCanQueensideCastle;
	int enPassantX;
	int enPassantY;
	int halfMoveClock;
	int fullMoveClock;


};

