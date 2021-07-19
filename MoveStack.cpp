#include "MoveStack.h"
#include <assert.h>
#include <cstddef> //for using NULL

void MoveStack::init() {
	top = NULL;
	amountOfMovesInStack = 0;
}

void MoveStack::push(StoreMove* newMove) {
	newMove->setNext(top);
	top = newMove; 
	amountOfMovesInStack++;
}

StoreMove* MoveStack::pop() {
	assert(amountOfMovesInStack != 0);
	StoreMove* temp = top;
	top = top->getNext();
	--amountOfMovesInStack;
	return temp;
}

int MoveStack::getSize() {
	return amountOfMovesInStack;
}

void MoveStack::clear() {
	while (top != NULL) {
		StoreMove* temp = top->getNext();
		delete top;
		top = temp;
	}
	amountOfMovesInStack = 0;
}
StoreMove* MoveStack::getTop() {
	return top;
}



