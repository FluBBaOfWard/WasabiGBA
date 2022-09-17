# WasabiGBA V0.2.1

<img align="right" width="220" src="./logo.png" />

This is a Watara/QuickShot Supervision emulator for the Nintendo GBA.

## How to use:

When the emulator starts, you press L+R to open up the menu.
Now you can use the cross to navigate the menus, A to select an option,
B to go back a step.

## Menu:

### File:
	Load Game: Select a game to load.
	Load State: Load a previously saved state of the currently running game.
	Save State: Save a state of the currently running game.
	Save Settings: Save the current settings.
	Reset Game: Reset the currently running game.

### Options:
	Controller:
		Autofire: Select if you want autofire.
		Swap A/B: Swap which GBA button is mapped to which SV button.
	Display:
		Gamma: Lets you change the gamma ("brightness").
		Contrast: Change palette contrast.
		Palette: Here you can select between different palettes.
	Machine Settings:
		Machine: Select the emulated machine.
	Settings:
		Speed: Switch between speed modes.
			Normal: Game runs at it's normal speed.
			200%: Game runs at double speed.
			Max: Games can run up to 4 times normal speed (might change).
			50%: Game runs at half speed.
		Autoload State: Toggle Savestate autoloading.
			Automagically load the savestate associated with the selected game.
		Autosave Settings: This will save settings when
			leaving menu if any changes are made.
		Autopause Game: Toggle if the game should pause when opening the menu.
		Debug Output: Show an FPS meter for now.
		Autosleep: Doesn't work.

### About:
	Some info about the emulator and game...

## Controls:
	A & B buttons are mapped to SV A & B.
	Start is mapped to SV Start.
	Select is mapped to SV Select.
	The d-pad is mapped to SV d-pad.

## Games:
	All games should "work".

## Credits:

Huge thanks to Loopy for the incredible PocketNES, without it this emu would
probably never have been made.
Thanks to:
	Peter Trauner & Kevin Horton for docs about the Supervision.
	Osman Celimli for docs, tests & help about the Supervision.


Fredrik Ahlström

Twitter @TheRealFluBBa

http://www.github.com/FluBBaOfWard
