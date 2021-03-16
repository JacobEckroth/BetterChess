#pragma once

struct Move {
	int fromX;
	int fromY;
	int toX;
	int toY;
	bool kingSideCastle;
	bool queenSideCastle;
	bool enPassant;
	bool isPromotion;
	char promotionType;
};