#pragma once
#include "SDL.h"
#include "SDL_image.h"
class Piece
{

public:
	static void init();
	static void loadImages();
	static void loadTypes();
	static void destroyImages();
	static SDL_Texture* blackKingTexture;
	static SDL_Texture* whiteKingTexture;
	static SDL_Texture* blackKnightTexture;
	static SDL_Texture* whiteKnightTexture;
	static SDL_Texture* blackRookTexture;
	static SDL_Texture* whiteRookTexture;
	static SDL_Texture* blackBishopTexture;
	static SDL_Texture* whiteBishopTexture;
	static SDL_Texture* whiteQueenTexture;
	static SDL_Texture* blackQueenTexture;
	static SDL_Texture* whitePawnTexture;
	static SDL_Texture* blackPawnTexture;


	static unsigned int white;
	static unsigned int black;
	static unsigned int queen;
	static unsigned int king;
	static unsigned int bishop;
	static unsigned int knight;
	static unsigned int rook;
	static unsigned int pawn;
};

