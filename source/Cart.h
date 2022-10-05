#ifndef CART_HEADER
#define CART_HEADER

#ifdef __cplusplus
extern "C" {
#endif

extern u32 gRomSize;
extern u32 maxRomSize;
extern u8 gConfig;
extern u8 gMachineSet;
extern u8 gMachine;
extern u8 gSOC;
extern u8 gLang;
extern u8 gPaletteBank;
extern u8 gGameID;
extern int sramSize;

extern u8 svRAM[0x2000];
extern u8 svVRAM[0x2000];
extern const u8 *romSpacePtr;

void machineInit(void);
void loadCart(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CART_HEADER
