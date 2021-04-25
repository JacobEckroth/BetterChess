#pragma once
#include "StoreMove.h"
class MoveStack
{
private:

	int amountOfMovesInStack;
	StoreMove* top;


public:
	void init();
	StoreMove* pop();
	void push(StoreMove*);
	int getSize();
	void clear();
	StoreMove* getTop();
};

