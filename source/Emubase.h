#ifndef EMUBASE
#define EMUBASE

#ifdef __cplusplus
extern "C" {
#endif

#define WSVID 0x1A565357			// "WSV",0x1A - WataraSuperVision

#define ALLOW_SPEED_HACKS	(1<<17)
#define ENABLE_HEADPHONES	(1<<18)

typedef struct {
	const u32 identifier;		// WSVID
	const u32 filesize;
	const u32 flags;
	const u32 undefined;
	const u32 bios;				// bit 0 = bios file.
	const u32 reserved[3];
	const char name[32];
	const u8 romData[];
} RomHeader;

typedef struct {				//(config struct)
	int emuSettings;
	int unused;					// unused
	u8 gammaValue;				// From gfx.s
	u8 config;					// From cart.s
	u8 controller;				// From io.s
	u8 contrastValue;			// From gfx.s
	u8 language;
	u8 palette;
	u8 padding[2];
	char currentPath[256];
	char monoBiosPath[256];
	char colorBiosPath[256];
} ConfigData;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !EMUBASE
