#include <gba.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Shared/SRAMHandler.h"
#include "Shared/AsmExtra.h"
#include "Main.h"
#include "Gui.h"
#include "Cart.h"
#include "cpu.h"
#include "Gfx.h"
#include "io.h"

/// Used for emulators or flashcarts to choose save type.
const char *const sramTag = "SRAM_Vnnn";

EWRAM_BSS int selectedGame = 0;
EWRAM_BSS ConfigData cfg;

//---------------------------------------------------------------------------------
void applyConfigData(void) {
	emuSettings    = cfg.emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	gGammaValue    = cfg.gammaValue & 0xF;
	gContrastValue = (cfg.gammaValue >> 4) & 0xF;
	joyCfg         = (joyCfg & ~0x400) | ((cfg.controller & 1) << 10);
	setSleepValue(emuSettings & AUTOSLEEP_MASK);
}

void updateConfigData(void) {
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	cfg.gammaValue  = (gGammaValue & 0xF) | (gContrastValue<<4);
	cfg.controller  = (joyCfg >> 10) & 1;
}

void initSettings() {
	memset(&cfg, 0, sizeof(ConfigData));
	cfg.emuSettings = AUTOPAUSE_EMULATION | AUTOSLEEP_OFF | ALLOW_SPEED_HACKS;
	cfg.gammaValue  = 0x10;

	applyConfigData();
}

int loadSettings() {
	if (readFile((u8 *)&cfg, sizeof(cfg), WSVID)) {
		applyConfigData();
		settingsChanged = false;
		infoOutput("Settings loaded.");
		return 0;
	}
	else {
		updateConfigData();
		infoOutput("No settings file found.");
	}
	return 1;
}
void saveSettings() {
	updateConfigData();

	if (writeFile((u8 *)&cfg, sizeof(cfg), WSVID, "Config")) {
		settingsChanged = false;
		infoOutput("Settings saved.");
	}
	else {
		infoOutput("Could not save settings file.");
	}
}

int loadNVRAM() {
	return 0;
}

void saveNVRAM() {
}

void loadState(void) {
	if (getStateSize() < 0x10000
		&& quickLoad()) {
		infoOutput("Loaded State.");
	}
}
void saveState(void) {
	if (getStateSize() < 0x10000
		&& quickSave()) {
		infoOutput("Saved State.");
	}
}

//---------------------------------------------------------------------------------
bool loadGame(const RomHeader *rh) {
	if (rh) {
		return loadROM(rh->romData, rh->filesize);
	}
	return true;
}

//---------------------------------------------------------------------------------
bool loadROM(const u8 *rom, int size) {
	selectedGame = selected;
	gRomSize = size;
	romSpacePtr = rom;
	checkMachine();
	setEmuSpeed(0);
	loadCart();
	gameInserted = true;
	if (emuSettings & AUTOLOAD_STATE) {
		loadState();
	}
	powerIsOn = true;
	closeMenu();
	return false;
}

void selectGame() {
	pauseEmulation = true;
	ui9();
	const RomHeader *rh = browseForFile();
	if (loadGame(rh)) {
		backOutOfMenu();
	}
}

void viewSStates() {
	pauseEmulation = true;
	ui13();
	skipScroll();
	loadStateMenu();
	backOutOfMenu();
}

void checkMachine() {
	if (gMachineSet == HW_AUTO) {
		if (romSpacePtr[gRomSize - 9] != 0) {
			gMachine = HW_SUPERVISION;
		}
//		else if (strstr(fileExt, ".pc2")) {
//			gMachine = HW_SUPERVISION_TVLINK;
//		}
	}
	else {
		gMachine = gMachineSet;
	}
	setupEmuBackground();
}
