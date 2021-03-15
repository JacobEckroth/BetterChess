#pragma once
#include "SDL.h"
#include "Move.h"
#include "BoardState.h"
#include <vector>
class Board
{
private:

	SDL_Color boardColor1;
	SDL_Color boardColor2;
	

	BoardState* boardState;
	

	//moving piece stuff.
	bool draggingPiece;
	int draggingPieceX;
	int draggingPieceY;
	SDL_Texture* draggingPieceTexture;

	//Move stuff
	std::vector<Move> pseudoLegalMoves;
	std::vector<Move> legalMoves;

	std::vector<Move> highlightMoves;

public:
	static int boxXWidth;
	static int boxYHeight;
	static int boardXBoxes;
	static int boardYBoxes;

	~Board();
	void update();
	void render(BoardState*);
	void resize();
	void init();
	void renderBoard();
	void renderDraggedPiece();

	void handleMouseButtonDown(SDL_MouseButtonEvent&, BoardState*);
	void attemptPickupPiece(int x, int y,BoardState*);
	void attemptPlacePiece(int x, int y,BoardState*);
	void stopDraggingPiece();

	int getWidth();
	int getHeight();
	BoardState* getBoardState();

	void printBoardState(BoardState*);

	void renderPieces(BoardState*);
	void renderPiece(int x, int y,BoardState*);
	void renderPieceTexture(SDL_Texture*, int x, int y);
	
	SDL_Texture* getTextureAtLocation(int x, int y,BoardState*);

	void loadBoardFromFen(const char* fen,BoardState*);

	bool pieceIsCurrentPlayersPiece(int x, int y,BoardState*);

	


	void switchTurns(BoardState*);

	//updates the en passant squares.
	void updateEnPassant(int, int, int, int,BoardState*);
	bool isEnPassant(int fromX, int fromY, int toX, int toY, BoardState*);

	//updates castling stuff.
	void updateCastling(int fromX, int fromY, int toX, int toY, BoardState*);

	//calculating legal move stuff
	void clearMoves();
	std::vector<Move> calculatePseudoLegalMoves(BoardState*);
	void calculateLegalMoves(BoardState*);
	void calculateMovesAt(int x, int y, BoardState*, std::vector<Move>&);
	

	void calculateRookMoves(int x, int y,BoardState*, std::vector<Move>&);
	void calculateBishopMoves(int x, int y,BoardState*, std::vector<Move>&);
	void calculateKnightMoves(int x, int y,BoardState*, std::vector<Move>&);
	void calculateKingMoves(int x, int y,BoardState*, std::vector<Move>&);
	void calculateQueenMoves(int x, int y,BoardState*, std::vector<Move>&);
	void calculatePawnMoves(int x, int y,BoardState*, std::vector<Move>&);
	void calculateCastlingMoves(int x, int y,BoardState*, std::vector<Move>&);

	bool inLegalMoves(struct Move&);
	bool inPseudoMoves(struct Move&);

	void makeMove(struct Move, BoardState*);
	

	//what moves should we highlight?
	void renderHighlightMoves();
	void createHighlightMoves(int x, int y);

	//check and checkmate stuff
	bool kingInCheck(BoardState* currentBoardState);
	void findKingLocation(int*, int*, BoardState*);
	bool squareAttacked(int x, int y, BoardState* currentBoardState);
};

