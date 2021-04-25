#pragma once
#include "Box.h"
struct Move {
	Box fromBox;
	Box toBox;
	bool kingSideCastle;
	bool queenSideCastle;
	bool enPassant;
	bool isPromotion;
	char promotionType;
};