#pragma once
#include <stdint.h>
#include "Box.h"
struct KingThreatenedInfo {
	uint8_t threatenedInfo;
	uint8_t attackedInfo;
	Box straightLeftBox;
	Box upLeftBox;
	Box straightUpBox;
	Box upRightBox;

	Box straightRightBox;
	Box downRightBox;
	Box straightDownBox;
	Box downLeftBox;
	int amountAttacked;

	Box attackedFromBox;
	bool attackedByKnight;

	static const uint8_t straightLeftThreatened;
	static  const uint8_t upLeftThreatened;
	static  const uint8_t straightUpThreatened;
	static  const uint8_t upRightThreatened;
	static  const uint8_t straightRightThreatened;
	static  const uint8_t downRightThreatened;
	static const  uint8_t straightDownThreatened;
	static  const uint8_t downLeftThreatened;

};

/*
const uint8_t KingThreatenedInfo::straightLeftThreatened = 0b00000001;
const uint8_t KingThreatenedInfo::upLeftThreatened = 0b00000010;
const uint8_t KingThreatenedInfo::straightUpThreatened = 0b00000100;
const uint8_t KingThreatenedInfo::upRightThreatened = 0b00001000;
const uint8_t KingThreatenedInfo::straightRightThreatened = 0b00010000;
const uint8_t KingThreatenedInfo::downRightThreatened = 0b00100000;
const uint8_t KingThreatenedInfo::straightDownThreatened = 0b01000000;
const uint8_t KingThreatenedInfo::downLeftThreatened = 0b10000000;
*/