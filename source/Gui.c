#include <gba.h>
#include <string.h>

#include "Gui.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Main.h"
#include "FileHandling.h"
#include "Cart.h"
#include "Gfx.h"
#include "Sound.h"
#include "io.h"
#include "cpu.h"
#include "ARM6502/Version.h"
#include "KS5360/Version.h"

#define EMUVERSION "V0.2.4 2024-09-22"

static void gammaChange(void);
static void paletteChange(void);
static const char *getPaletteText(void);
static void machineSet(void);
static const char *getMachineText(void);
static void borderSet(void);
static const char *getBorderText(void);
static void soundSet(void);
static void swapABSet(void);
static const char *getSwapABText(void);
static void contrastSet(void);
static const char *getContrastText(void);


const MItem dummyItems[] = {
	{"", uiDummy},
};
const MItem mainItems[] = {
	{"File->", ui2},
	{"Controller->", ui3},
	{"Display->", ui4},
	{"Settings->", ui5},
	{"Machine->", ui6},
	{"Debug->", ui7},
	{"About->", ui8},
	{"Sleep", gbaSleep},
	{"Reset Console", resetGame},
	{"Quit Emulator", ui10},
};
const MItem fileItems[] = {
	{"Load Game->", selectGame},
	{"Load State", loadState},
	{"Save State", saveState},
	{"Save Settings", saveSettings},
	{"Reset Game", resetGame},
};
const MItem ctrlItems[] = {
	{"B Autofire: ", autoBSet, getAutoBText},
	{"A Autofire: ", autoASet, getAutoAText},
	{"Swap A-B:   ", swapABSet, getSwapABText},
};
const MItem displayItems[] = {
	{"Gamma: ", gammaChange, getGammaText},
	{"Contrast: ", contrastSet, getContrastText},
	{"Palette: ", paletteChange, getPaletteText},
	{"Border: ", borderSet, getBorderText},
};
const MItem setItems[] = {
	{"Speed: ", speedSet, getSpeedText},
	{"Sound: ", soundSet, getSoundEnableText},
	{"Autoload State: ", autoStateSet, getAutoStateText},
	{"Autosave Settings: ", autoSettingsSet, getAutoSettingsText},
	{"Autopause Game: ", autoPauseGameSet, getAutoPauseGameText},
	{"EWRAM Overclock: ", ewramSet, getEWRAMText},
	{"Autosleep: ", sleepSet, getSleepText},
};
const MItem machineItems[] = {
	{"Machine: ", machineSet, getMachineText},
};
const MItem debugItems[] = {
	{"Debug Output:", debugTextSet, getDebugText},
	{"Step Frame", stepFrame},
};
const MItem fnList9[] = {
	{"", quickSelectGame},
};
const MItem quitItems[] = {
	{"Yes", exitEmulator},
	{"No", backOutOfMenu},
};

const Menu menu0 = MENU_M("", uiNullNormal, dummyItems);
Menu menu1 = MENU_M("Main Menu", uiAuto, mainItems);
const Menu menu2 = MENU_M("File Handling", uiAuto, fileItems);
const Menu menu3 = MENU_M("Controller Settings", uiAuto, ctrlItems);
const Menu menu4 = MENU_M("Display Settings", uiAuto, displayItems);
const Menu menu5 = MENU_M("Other Settings", uiAuto, setItems);
const Menu menu6 = MENU_M("Machine Settings", uiAuto, machineItems);
const Menu menu7 = MENU_M("Debug", uiAuto, debugItems);
const Menu menu8 = MENU_M("About", uiAbout, dummyItems);
const Menu menu9 = MENU_M("Load Game", uiLoadGame, fnList9);
const Menu menu10 = MENU_M("Quit Emulator?", uiAuto, quitItems);

const Menu *const menus[] = {&menu0, &menu1, &menu2, &menu3, &menu4, &menu5, &menu6, &menu7, &menu8, &menu9, &menu10 };

u8 gGammaValue = 0;
u8 gContrastValue = 0;
u8 gBorderEnable = 1;

const char *const machTxt[]  = {"Auto", "Supervision", "Supervision TV-Link"};
const char *const bordTxt[]  = {"Black", "Frame", "BG Color", "None"};
const char *const palTxt[]   = {"Green", "Black & White", "Red", "Blue", "Classic"};

/// This is called at the start of the emulator
void setupGUI() {
	emuSettings = AUTOPAUSE_EMULATION;
//	keysSetRepeat(25, 4);	// Delay, repeat.
	menu1.itemCount = ARRSIZE(mainItems) - (enableExit?0:1);
	closeMenu();
}

/// This is called when going from emu to ui.
void enterGUI() {
}

/// This is called going from ui to emu.
void exitGUI() {
	setupEmuBorderPalette();
}

void quickSelectGame() {
	openMenu();
	selectGame();
	closeMenu();
}

void uiNullNormal() {
	uiNullDefault();
}

void uiAbout() {
	setupSubMenu("About");
	drawText("A:        SV A Button", 3);
	drawText("B:        SV B Button", 4);
	drawText("Start:    SV Start Button", 5);
	drawText("Select:   Sv Select Button", 6);
	drawText("DPad:     SV DPad", 7);

	drawText("WasabiGBA  " EMUVERSION, 17);
	drawText("KS5360     " KS5360VERSION, 18);
	drawText("ARM6502    " ARM6502VERSION, 19);
}

void uiLoadGame() {
	setupSubMenuText();
}

void nullUINormal(int key) {
}

void nullUIDebug(int key) {
}

void resetGame() {
	checkMachine();
	loadCart();
	setupEmuBackground();
	setupMenuPalette();
	powerIsOn = true;
}

//---------------------------------------------------------------------------------
void debugIO(u8 port, u8 val, const char *message) {
	char debugString[32];

	debugString[0] = 0;
	strlcat(debugString, message, sizeof(debugString));
	char2HexStr(&debugString[strlen(debugString)], port);
	strlcat(debugString, " val:", sizeof(debugString));
	char2HexStr(&debugString[strlen(debugString)], val);
	debugOutput(debugString);
}
//---------------------------------------------------------------------------------
void debugIOUnimplR(u8 port) {
	debugIO(port, 0, "Unimpl R port:");
}
void debugIOUnimplW(u8 port, u8 val) {
	debugIO(port, val, "Unimpl W port:");
}
void debugIOUnmappedR(u8 port) {
	debugIO(port, 0, "Unmapped R port:");
}
void debugIOUnmappedW(u8 port, u8 val) {
	debugIO(port, val, "Unmapped W port:");
}
void debugUndefinedInstruction() {
	debugOutput("Undefined Instruction.");
}
void debugCrashInstruction() {
	debugOutput("CPU Crash!");
}
//---------------------------------------------------------------------------------

/// Swap A & B buttons
void swapABSet() {
	joyCfg ^= 0x400;
}
const char *getSwapABText() {
	return autoTxt[(joyCfg>>10)&1];
}

/// Change gamma (brightness)
void gammaChange() {
	gammaSet();
	paletteInit(gGammaValue);
//	setupEmuBorderPalette();
	setupMenuPalette();
}

/// Change contrast
void contrastSet() {
	gContrastValue++;
	if (gContrastValue > 4) gContrastValue = 0;
	paletteInit(gGammaValue);
//	setupEmuBorderPalette();
	settingsChanged = true;
}
const char *getContrastText() {
	return brighTxt[gContrastValue];
}

void paletteChange() {
	gPaletteBank++;
	if (gPaletteBank > 4) {
		gPaletteBank = 0;
	}
	monoPalInit();
	paletteInit(gGammaValue);
	settingsChanged = true;
}
const char *getPaletteText() {
	return palTxt[gPaletteBank];
}

void borderSet() {
	gBorderEnable ^= 0x01;
	setupEmuBorderPalette();
	setupMenuPalette();
}
const char *getBorderText() {
	return bordTxt[gBorderEnable];
}

void machineSet() {
	gMachineSet++;
	if (gMachineSet >= HW_SELECT_END) {
		gMachineSet = 0;
	}
}
const char *getMachineText() {
	return machTxt[gMachineSet];
}

void soundSet() {
	soundEnableSet();
	soundMode = (emuSettings & SOUND_ENABLE)>>10;
	soundInit();
}

void speedHackSet() {
//	emuSettings ^= ALLOW_SPEED_HACKS;
//	emuSettings &= ~HALF_CPU_SPEED;
//	hacksInit();
}
