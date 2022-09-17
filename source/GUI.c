#include <gba.h>
#include <string.h>

#include "GUI.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Main.h"
#include "FileHandling.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"
#include "cpu.h"
#include "ARM6502/Version.h"
#include "KS5360/Version.h"

#define EMUVERSION "V0.2.0 2022-09-17"

#define ALLOW_SPEED_HACKS	(1<<17)
#define ENABLE_HEADPHONES	(1<<18)
#define ALLOW_REFRESH_CHG	(1<<19)

static void paletteChange(void);
static void machineSet(void);
static void refreshChgSet(void);

static void setupWSVBackground(void);

static void uiMachine(void);
static void uiDebug(void);
static void updateGameInfo(void);

const fptr fnMain[] = {nullUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI, subUI};

const fptr fnList0[] = {uiDummy};
const fptr fnList1[] = {ui2, ui3, ui4, ui5, ui6, ui7, gbaSleep, resetGame};
const fptr fnList2[] = {ui8, loadState, saveState, saveSettings, resetGame};
const fptr fnList3[] = {autoBSet, autoASet, swapABSet};
const fptr fnList4[] = {gammaSet, contrastSet, paletteChange};
const fptr fnList5[] = {speedSet, autoStateSet, autoSettingsSet, autoPauseGameSet, debugTextSet, sleepSet};
const fptr fnList6[] = {machineSet};
const fptr fnList7[] = {uiDummy};
const fptr fnList8[] = {quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame, quickSelectGame};
const fptr *const fnListX[] = {fnList0, fnList1, fnList2, fnList3, fnList4, fnList5, fnList6, fnList7, fnList8};
const u8 menuXItems[] = {ARRSIZE(fnList0), ARRSIZE(fnList1), ARRSIZE(fnList2), ARRSIZE(fnList3), ARRSIZE(fnList4), ARRSIZE(fnList5), ARRSIZE(fnList6), ARRSIZE(fnList7), ARRSIZE(fnList8)};
const fptr drawUIX[] = {uiNullNormal, uiMainMenu, uiFile, uiController, uiDisplay, uiSettings, uiMachine, uiAbout, uiLoadGame};
const u8 menuXBack[] = {0,0,1,1,1,1,1,1,2};

u8 gGammaValue = 0;
u8 gContrastValue = 1;
char gameInfoString[32];

const char *const autoTxt[]  = {"Off", "On", "With R"};
const char *const speedTxt[] = {"Normal", "200%", "Max", "50%"};
const char *const sleepTxt[] = {"5min", "10min", "30min", "Off"};
const char *const brighTxt[] = {"I", "II", "III", "IIII", "IIIII"};
const char *const ctrlTxt[]  = {"1P", "2P"};
const char *const dispTxt[]  = {"Unscaled", "Scaled"};
const char *const flickTxt[] = {"No Flicker", "Flicker"};
const char *const bordTxt[]  = {"Black", "Border Color", "None"};
const char *const palTxt[]   = {"Green", "Black & White", "Red", "Blue", "Classic"};
const char *const langTxt[]  = {"Japanese", "English"};
const char *const machTxt[]  = {"Auto", "SuperVision", "SuperVision TV-Link"};

/// This is called at the start of the emulator
void setupGUI() {
	emuSettings = AUTOPAUSE_EMULATION;
//	keysSetRepeat(25, 4);	// Delay, repeat.
	closeMenu();
}

/// This is called when going from emu to ui.
void enterGUI() {
}

/// This is called going from ui to emu.
void exitGUI() {
	setupBorderPalette();
}

void quickSelectGame() {
	while (loadGame()) {
		redrawUI();
		return;
	}
	closeMenu();
}

void uiNullNormal() {
	uiNullDefault();
}

void uiFile() {
	setupSubMenu("File Handling");
	drawMenuItem("Load Game->");
	drawMenuItem("Load State");
	drawMenuItem("Save State");
	drawMenuItem("Save Settings");
	drawMenuItem("Reset Game");
}

void uiMainMenu() {
	setupSubMenu("Main Menu");
	drawMenuItem("File->");
	drawMenuItem("Controller->");
	drawMenuItem("Display->");
	drawMenuItem("Settings->");
	drawMenuItem("Machine->");
	drawMenuItem("Help->");
	drawMenuItem("Sleep");
	drawMenuItem("Restart");
	if (enableExit) {
		drawMenuItem("Exit");
	}
}

void uiAbout() {
	setupSubMenu("Help");
	updateGameInfo();
	drawText("A:        SV A Button", 3);
	drawText("B:        SV B Button", 4);
	drawText("Start:    SV Start Button", 5);
	drawText("Select:   Sv Select Button", 6);
	drawText("DPad:     SV DPad", 7);

//	drawText(gameInfoString, 9);

	drawText("WasabiGBA   " EMUVERSION, 17);
	drawText("KS5360      " KS5360VERSION, 18);
	drawText("ARM6502     " ARM6502VERSION, 19);
}

void uiController() {
	setupSubMenu("Controller Settings");
	drawSubItem("B Autofire: ", autoTxt[autoB]);
	drawSubItem("A Autofire: ", autoTxt[autoA]);
	drawSubItem("Swap A-B:   ", autoTxt[(joyCfg>>10)&1]);
}

void uiDisplay() {
	setupSubMenu("Display Settings");
	drawSubItem("Gamma: ", brighTxt[gGammaValue]);
	drawSubItem("Contrast: ", brighTxt[gContrastValue]);
	drawSubItem("Palette: ", palTxt[gPaletteBank]);
}

static void uiMachine() {
	setupSubMenu("Machine Settings");
	drawSubItem("Machine: ",machTxt[gMachineSet]);
}

void uiSettings() {
	setupSubMenu("Other Settings");
	drawSubItem("Speed: ", speedTxt[(emuSettings>>6)&3]);
	drawSubItem("Autoload State: ", autoTxt[(emuSettings>>1)&1]);
	drawSubItem("Autosave Settings: ", autoTxt[(emuSettings>>4)&1]);
	drawSubItem("Autopause Game: ", autoTxt[emuSettings&1]);
	drawSubItem("Debug Output: ", autoTxt[gDebugSet&1]);
	drawSubItem("Autosleep: ", sleepTxt[(emuSettings>>8)&3]);
}

void uiDebug() {
	setupSubMenu("Debug");
	drawSubItem("Debug Output: ", autoTxt[gDebugSet&1]);
}

void uiLoadGame() {
	setupSubMenu("Load game");
}

void nullUINormal(int key) {
}

void nullUIDebug(int key) {
}

void resetGame() {
	gMachine = gMachineSet;
	loadCart();
}

void updateGameInfo() {
	char catalog[8];
//	char2HexStr(catalog, gGameID);
	strlMerge(gameInfoString, " Game #: 0x", catalog, sizeof(gameInfoString));
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
	debugOutput("CPU Crash! (0xF1)");
}
//---------------------------------------------------------------------------------

/// Swap A & B buttons
void swapABSet() {
	joyCfg ^= 0x400;
}

/// Change gamma (brightness)
void gammaSet() {
	gGammaValue++;
	if (gGammaValue > 4) gGammaValue=0;
	paletteInit(gGammaValue);
	setupMenuPalette();
	settingsChanged = true;
}

/// Change contrast
void contrastSet() {
	gContrastValue++;
	if (gContrastValue > 4) gContrastValue = 0;
	paletteInit(gGammaValue);
	settingsChanged = true;
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

void machineSet() {
	gMachineSet++;
	if (gMachineSet >= HW_SELECT_END) {
		gMachineSet = 0;
	}
}

void speedHackSet() {
//	emuSettings ^= ALLOW_SPEED_HACKS;
//	emuSettings &= ~HALF_CPU_SPEED;
//	hacksInit();
}
