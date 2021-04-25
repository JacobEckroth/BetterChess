#pragma once
#include "Move.h"
#include <stdint.h>
class StoreMove
{
public:
	void init(StoreMove*, Move, bool, bool, Box, bool, uint8_t);

	void setNext(StoreMove*);
	StoreMove* getNext();
	StoreMove();

	Move getMove();
	bool tookPiece();
	Box getPreviousEnPassant();
	bool getPreviousWhiteKingSide();
	bool getPreviousWhiteQueenSide();
	bool getPreviousBlackKingSide();
	bool getPreviousBlackQueenSide();
	uint8_t getTakenPiece();


	void setMove(Move);
	void setPreviousCastles(bool, bool, bool, bool);
	void setPreviousEnPassant(Box);
	void setTookPiece(bool);
	void setPieceTaken(uint8_t);
	void setThreatInfo(uint8_t, uint8_t, uint8_t, uint8_t, int,bool,Box previousAttackedFrom);

	void setThreatBoxes(Box, Box, Box, Box, Box, Box, Box, Box
						,Box, Box, Box, Box, Box, Box, Box, Box);
	Box getPreviousWhiteStraightLeftBox();
	Box getPreviousWhiteStraightRightBox();
	Box getPreviousWhiteStraightUpBox();
	Box getPreviousWhiteStraightDownBox();

	Box getPreviousWhiteUpLeftBox();
	Box getPreviousWhiteUpRightBox();
	Box getPreviousWhiteDownRightBox();
	Box getPreviousWhiteDownLeftBox();

	Box getPreviousBlackStraightLeftBox();
	Box getPreviousBlackStraightRightBox();
	Box getPreviousBlackStraightUpBox();
	Box getPreviousBlackStraightDownBox();

	Box getPreviousBlackUpLeftBox();
	Box getPreviousBlackUpRightBox();
	Box getPreviousBlackDownRightBox();
	Box getPreviousBlackDownLeftBox();

	uint8_t getPreviousWhiteThreatenedInfo();
	uint8_t getPreviousBlackThreatenedInfo();
	uint8_t getPreviousWhiteAttackedInfo();
	uint8_t getPreviousBlackAttackedInfo();
	int getPreviousAmountAttacked();

	bool getPreviousAttackedByKnight();

	Box getPreviousAttackedFromBox();

private:
	StoreMove* next;
	Move move;
	bool previousWhiteKingSide;
	bool previousBlackKingSide;
	bool previousWhiteQueenSide;
	bool previousBlackQueenSide;

	Box previousEnPassant;
	bool tookAPiece;
	uint8_t pieceTaken;
	bool previousAttackedByKnight;
	uint8_t previousWhiteThreatenedInfo;
	uint8_t previousBlackThreatenedInfo;
	uint8_t previousWhiteAttackedInfo;
	uint8_t previousBlackAttackedInfo;
	Box previousAttackedFromBox;
	int previousAmountAttacked;
	Box previousWhiteStraightLeftBox, previousWhiteStraightRightBox, previousWhiteStraightUpBox,
		previousWhiteStraightDownBox, previousWhiteUpLeftBox, previousWhiteUpRightBox,
		previousWhiteDownRightBox, previousWhiteDownLeftBox;
	Box previousBlackStraightLeftBox, previousBlackStraightRightBox, previousBlackStraightUpBox,
		previousBlackStraightDownBox, previousBlackUpLeftBox, previousBlackUpRightBox,
		previousBlackDownRightBox, previousBlackDownLeftBox;
};

