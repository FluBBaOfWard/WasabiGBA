#include <gba.h>
#include <string.h>

#include "Supervision.h"
#include "SVBorder.h"
#include "Cart.h"
#include "Gfx.h"
#include "cpu.h"


int packState(void *statePtr) {
	int size = 0;
	memcpy(statePtr+size, svRAM, sizeof(svRAM));
	size += sizeof(svRAM);
	memcpy(statePtr+size, svVRAM, sizeof(svVRAM));
	size += sizeof(svVRAM);
	size += svVideoSaveState(statePtr+size, &ks5360_0);
	size += m6502SaveState(statePtr+size, &m6502_0);
	return size;
}

void unpackState(const void *statePtr) {
	int size = 0;
	memcpy(svRAM, statePtr+size, sizeof(svRAM));
	size += sizeof(svRAM);
	memcpy(svVRAM, statePtr+size, sizeof(svVRAM));
	size += sizeof(svVRAM);
	size += svVideoLoadState(&ks5360_0, statePtr+size);
	size += m6502LoadState(&m6502_0, statePtr+size);
}

int getStateSize() {
	int size = 0;
	size += sizeof(svRAM);
	size += sizeof(svVRAM);
	size += svVideoGetStateSize();
	size += m6502GetStateSize();
	return size;
}

void setupSVBackground() {
	LZ77UnCompVram(SVBorderTiles, TILE_BASE_ADR(1));
	LZ77UnCompVram(SVBorderMap, MAP_BASE_ADR(2));
	memcpy(&EMUPALBUFF[0x10], SVBorderPal+16, 32);
}


void setupBorderPalette() {
	memcpy(&EMUPALBUFF[0x10], SVBorderPal+16, 32); // SVBorderPalLen
}

void setupEmuBackground() {
//	if (gMachine == HW_SUPERVISION) {
		setupSVBackground();
//	}
}
