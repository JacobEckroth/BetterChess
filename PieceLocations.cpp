#include "PieceLocations.h"
#include "Piece.h"

Box& PieceLocations::getKingLocation() {
	return kingLocation;
}

std::vector<Box>& PieceLocations::getPawnLocations() {
	return pawnLocations;
}

std::vector<Box>& PieceLocations::getKnightLocations() {
	return knightLocations;
}

std::vector<Box>& PieceLocations::getRookLocations() {
	return rookLocations;
}

std::vector<Box>& PieceLocations::getBishopLocations() {
	return bishopLocations;
}

std::vector<Box>& PieceLocations::getQueenLocations() {
	return queenLocations;
}


void PieceLocations::removePawn(Box removeBox) {
	for (int i = 0; i < pawnLocations.size(); i++) {
		if (pawnLocations.at(i).x == removeBox.x && pawnLocations.at(i).y == removeBox.y) {
			pawnLocations.erase(pawnLocations.begin() + i);
			return;
		}
	}
}

void PieceLocations::removeKnight(Box removeBox) {
	for (int i = 0; i < knightLocations.size(); i++) {
		if (knightLocations.at(i).x == removeBox.x && knightLocations.at(i).y == removeBox.y) {
			knightLocations.erase(knightLocations.begin() + i);
			return;
		}
	}
}

void PieceLocations::removeRook(Box removeBox) {
	for (int i = 0; i < rookLocations.size(); i++) {
		if (rookLocations.at(i).x == removeBox.x && rookLocations.at(i).y == removeBox.y) {
			rookLocations.erase(rookLocations.begin() + i);
			return;
		}
	}
}

void PieceLocations::removeBishop(Box removeBox) {
	for (int i = 0; i < bishopLocations.size(); i++) {
		if (bishopLocations.at(i).x == removeBox.x && bishopLocations.at(i).y == removeBox.y) {
			bishopLocations.erase(bishopLocations.begin() + i);
			return;
		}
	}
}

void PieceLocations::removeQueen(Box removeBox) {
	for (int i = 0; i < queenLocations.size(); i++) {
		if (queenLocations.at(i).x == removeBox.x && queenLocations.at(i).y == removeBox.y) {
			queenLocations.erase(queenLocations.begin() + i);
			return;
		}
	}
}


void PieceLocations::updatePawn(Box oldBox, Box newBox) {
	for (int i = 0; i < pawnLocations.size(); i++) {
		if (pawnLocations.at(i).x == oldBox.x && pawnLocations.at(i).y == oldBox.y) {
			pawnLocations.at(i).x = newBox.x;
			pawnLocations.at(i).y = newBox.y;
			return;
		}
	}
}

void PieceLocations::updateQueen(Box oldBox, Box newBox) {
	for (int i = 0; i < queenLocations.size(); i++) {
		if (queenLocations.at(i).x == oldBox.x && queenLocations.at(i).y == oldBox.y) {
			queenLocations.at(i).x = newBox.x;
			queenLocations.at(i).y = newBox.y;
			return;
		}
	}
}

void PieceLocations::updateBishop(Box oldBox, Box newBox) {
	for (int i = 0; i < bishopLocations.size(); i++) {
		if (bishopLocations.at(i).x == oldBox.x && bishopLocations.at(i).y == oldBox.y) {
			bishopLocations.at(i).x = newBox.x;
			bishopLocations.at(i).y = newBox.y;
			return;
		}
	}
}

void PieceLocations::updateRook(Box oldBox, Box newBox) {
	for (int i = 0; i < rookLocations.size(); i++) {
		if (rookLocations.at(i).x == oldBox.x && rookLocations.at(i).y == oldBox.y) {
			rookLocations.at(i).x = newBox.x;
			rookLocations.at(i).y = newBox.y;
			return;
		}
	}
}

void PieceLocations::updateKnight(Box oldBox, Box newBox) {
	for (int i = 0; i < knightLocations.size(); i++) {
		if (knightLocations.at(i).x == oldBox.x && knightLocations.at(i).y == oldBox.y) {
			knightLocations.at(i).x = newBox.x;
			knightLocations.at(i).y = newBox.y;
			return;
		}
	}
}


void PieceLocations::removePiece(uint8_t piece, Box box) {
	if ((piece & Piece::knight) == Piece::knight) {
		removeKnight(box);
	}else if ((piece & Piece::pawn) == Piece::pawn) {
		removePawn(box);
	}
	else if ((piece & Piece::queen) == Piece::queen) {
		removeQueen(box);
	}
	else if ((piece & Piece::bishop) == Piece::bishop) {
		removeBishop(box);
	}
	else if ((piece & Piece::rook) == Piece::rook) {
		removeRook(box);
	}
}

void PieceLocations::updatePiece(uint8_t piece, Box fromBox, Box toBox) {
	if ((piece & Piece::knight) == Piece::knight) {
		updateKnight(fromBox, toBox);
	}
	else if ((piece & Piece::pawn) == Piece::pawn) {
		updatePawn(fromBox, toBox);
	}
	else if ((piece & Piece::queen) == Piece::queen) {
		updateQueen(fromBox, toBox);
	}
	else if ((piece & Piece::bishop) == Piece::bishop) {
		updateBishop(fromBox, toBox);
	}
	else if ((piece & Piece::rook) == Piece::rook) {
		updateRook(fromBox, toBox);
	}
	else if ((piece & Piece::king) == Piece::king) {
		setKingLocation(toBox);
	}
}

void PieceLocations::addPiece(uint8_t piece, Box newBox) {
	if ((piece & Piece::knight) == Piece::knight) {
		knightLocations.push_back({ newBox.x,newBox.y });
	}
	else if ((piece & Piece::pawn) == Piece::pawn) {
		pawnLocations.push_back({ newBox.x,newBox.y });
	}
	else if ((piece & Piece::queen) == Piece::queen) {
		queenLocations.push_back({ newBox.x,newBox.y });
	}
	else if ((piece & Piece::bishop) == Piece::bishop) {
		bishopLocations.push_back({ newBox.x,newBox.y });
	}
	else if ((piece & Piece::rook) == Piece::rook) {
		rookLocations.push_back({ newBox.x,newBox.y });
	}
	else if ((piece & Piece::king) == Piece::king) {
		setKingLocation(newBox);
	}
}

void PieceLocations::setKingLocation(Box newLoc) {
	kingLocation.x = newLoc.x;
	kingLocation.y = newLoc.y;
}

void PieceLocations::clear() {
	pawnLocations.clear();
	knightLocations.clear();
	rookLocations.clear();
	bishopLocations.clear();
	queenLocations.clear();
}