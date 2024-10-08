#ifndef SUPERVISION_HEADER
#define SUPERVISION_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/// This runs all save state functions for each chip.
int packState(void *statePtr);

/// This runs all load state functions for each chip.
void unpackState(const void *statePtr);

/// Gets the total state size in bytes.
int getStateSize(void);

/// Setup Supervision background for emulator screen.
void setupEmuBackground(void);

void setupEmuBorderPalette(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SUPERVISION_HEADER
