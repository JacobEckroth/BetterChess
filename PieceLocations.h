#pragma once
#include "Box.h"
#include <vector>
#include <stdint.h>
class PieceLocations
{
public:
	Box& getKingLocation();
	
	std::vector<Box>& getPawnLocations();
	std::vector<Box>& getKnightLocations();
	std::vector<Box>& getRookLocations();
	std::vector<Box>& getBishopLocations();
	std::vector<Box>& getQueenLocations();

	void clear();

	void setKingLocation(Box);
	void removePawn(Box);
	void removeKnight(Box);
	void removeRook(Box);
	void removeBishop(Box);
	void removeQueen(Box);

	void removePiece(uint8_t, Box);
	void updatePiece(uint8_t, Box, Box);
	void addPiece(uint8_t, Box);
	void updatePawn(Box,Box);
	void updateRook(Box,Box);
	void updateKnight(Box,Box);
	void updateBishop(Box,Box);
	void updateQueen(Box,Box);

private:

	Box kingLocation;
	std::vector<Box> pawnLocations;
	std::vector<Box> knightLocations;
	std::vector<Box> rookLocations;
	std::vector<Box> bishopLocations;
	std::vector<Box> queenLocations;



};

