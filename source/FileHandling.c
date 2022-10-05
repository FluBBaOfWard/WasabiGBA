#include <gba.h>
#include <string.h>

#include "FileHandling.h"
#include "Emubase.h"
#include "Main.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/FileHelper.h"
#include "Gui.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"

static int selectedGame = 0;
ConfigData cfg;

//---------------------------------------------------------------------------------
int loadSettings() {
	gGammaValue = cfg.gammaValue;
	emuSettings  = cfg.emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	sleepTime    = cfg.sleepTime;
	joyCfg       = (joyCfg&~0x400)|((cfg.controller&1)<<10);
//	strlcpy(currentDir, cfg.currentPath, sizeof(currentDir));

	infoOutput("Settings loaded.");
	return 0;
}
void saveSettings() {
	strcpy(cfg.magic,"cfg");
	cfg.gammaValue  = gGammaValue;
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK;	// Clear speed setting.
	cfg.sleepTime   = sleepTime;
	cfg.controller  = (joyCfg>>10)&1;
//	strlcpy(cfg.currentPath, currentDir, sizeof(currentDir));
	infoOutput("Settings saved.");
}

int loadNVRAM() {
	return 0;
}

void saveNVRAM() {
}

void loadState(void) {
//	unpackState(testState);
	infoOutput("Loaded state.");
}
void saveState(void) {
//	packState(testState);
	infoOutput("Saved state.");
}

//---------------------------------------------------------------------------------
bool loadGame(const RomHeader *rh) {
	if (rh) {
		gRomSize = rh->filesize;
		romSpacePtr = (const u8 *)rh + sizeof(RomHeader);
		selectedGame = selected;
		checkMachine();
		setEmuSpeed(0);
		loadCart();
		gameInserted = true;
		if (emuSettings & AUTOLOAD_NVRAM) {
			loadNVRAM();
		}
		if (emuSettings & AUTOLOAD_STATE) {
			loadState();
		}
		closeMenu();
		return false;
	}
	return true;
}

void selectGame() {
	pauseEmulation = true;
	ui9();
	const RomHeader *rh = browseForFile();
	if (loadGame(rh)) {
		backOutOfMenu();
	}
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
