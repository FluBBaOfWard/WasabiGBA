#ifdef __arm__

#include "Shared/gba_asm.h"
#include "KS5360/KS5360.i"

	.global gfxInit
	.global gfxReset
	.global monoPalInit
	.global paletteInit
	.global paletteTxAll
	.global refreshGfx
	.global endFrameGfx
	.global wsvReadIO
	.global wsvWriteIO
	.global updateLCDRefresh
	.global setScreenRefresh

	.global gfxState
	.global gGammaValue
	.global gFlicker
	.global gTwitch
	.global gGfxMask
	.global vblIrqHandler
	.global GFX_DISPCNT
	.global GFX_BG0CNT
	.global GFX_BG1CNT
	.global EMUPALBUFF
	.global tmpOamBuffer


	.global ks5360_0


	.syntax unified
	.arm

#if GBA
	.section .ewram, "ax", %progbits	;@ For the GBA
#else
	.section .text						;@ For anything else
#endif
	.align 2
;@----------------------------------------------------------------------------
gfxInit:					;@ Called from machineInit
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	bl svVideoInit
	bl gfxWinInit

	ldmfd sp!,{pc}

;@----------------------------------------------------------------------------
gfxReset:					;@ Called with CPU reset
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}

	ldr r0,=gfxState
	mov r1,#5					;@ 5*4
	bl memclr_					;@ Clear GFX regs

	bl gfxWinInit

	ldr r0,=m6502SetNMIPin
	ldr r1,=m6502SetIRQPin
	ldr r2,=svVRAM
	ldr r3,=gSOC
	ldrb r3,[r3]
	bl svVideoReset0
	bl monoPalInit

	ldr r0,=gGammaValue
	ldrb r0,[r0]
	bl paletteInit				;@ Do palette mapping

	ldr svvptr,=ks5360_0

	ldmfd sp!,{pc}

;@----------------------------------------------------------------------------
gfxWinInit:
;@----------------------------------------------------------------------------
	mov r1,#REG_BASE
	;@ Horizontal start-end
	ldr r0,=(((SCREEN_WIDTH-GAME_WIDTH)/2)<<8)+(SCREEN_WIDTH+GAME_WIDTH)/2
	strh r0,[r1,#REG_WIN0H]
	strh r0,[r1,#REG_WIN1H]
	;@ Vertical start-end
	ldr r0,=(((SCREEN_HEIGHT-GAME_HEIGHT)/2)<<8)+(SCREEN_HEIGHT+GAME_HEIGHT)/2
	strh r0,[r1,#REG_WIN0V]
	strh r0,[r1,#REG_WIN1V]

	ldr r0,=0x3B3B				;@ WinIN0/1, BG0, BG1, BG3, SPR & COL inside Win0
	strh r0,[r1,#REG_WININ]
	mov r0,#0x002C				;@ WinOUT, Only BG2, BG3 & COL enabled outside Windows.
	strh r0,[r1,#REG_WINOUT]
	bx lr
;@----------------------------------------------------------------------------
monoPalInit:
	.type monoPalInit STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4,lr}
	ldr r0,=gPaletteBank
	ldrb r0,[r0]
	adr r1,monoPalette
	add r1,r1,r0,lsl#3
	ldr r0,=MAPPED_RGB

	ldmia r1,{r2-r3}
	stmia r0!,{r2-r3}

	ldmfd sp!,{r4,lr}
	bx lr
;@----------------------------------------------------------------------------
monoPalette:

;@ Green
	.long 0x7FFF7F, 0x102010
;@ Black & White
	.long 0xFFFFFF, 0x000000
;@ Red
	.long 0xFF7F7F, 0x201010
;@ Blue
	.long 0x8080FF, 0x000020
;@ Classic
	.long 0xFFDDAA, 0x221100
;@----------------------------------------------------------------------------
paletteInit:		;@ r0-r3 modified.
	.type paletteInit STT_FUNC
;@ Called by ui.c:  void paletteInit(gammaVal);
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r9,lr}
	mov r8,r0					;@ Gamma value = 0 -> 4
	ldr r9,=gContrastValue
	ldrb r9,[r9]
	ldr r6,=EMUPALBUFF
	ldr r7,=MAPPED_RGB
	mov r4,#4
noMap:							;@ Map rrrrrrrr_gggggggg_bbbbbbbb  ->  0bbbbbgggggrrrrr
	ldrb r0,[r7,#2]				;@ Red high
	ldrb r1,[r7,#6]				;@ Red low
	mov r2,r9
	bl contrastConvert
	sub r2,r4,#1
	bl liConvR2
	mov r1,r8
	bl gammaConvert
	mov r5,r0,lsr#3

	ldrb r0,[r7,#1]				;@ Green high
	ldrb r1,[r7,#5]				;@ Green low
	mov r2,r9
	bl contrastConvert
	sub r2,r4,#1
	bl liConvR2
	mov r1,r8
	bl gammaConvert
	mov r0,r0,lsr#3
	orr r5,r5,r0,lsl#5

	ldrb r0,[r7,#0]				;@ Blue high
	ldrb r1,[r7,#4]				;@ Blue low
	mov r2,r9
	bl contrastConvert
	sub r2,r4,#1
	bl liConvR2
	mov r1,r8
	bl gammaConvert
	mov r0,r0,lsr#3
	orr r5,r5,r0,lsl#10

	strh r5,[r6],#2
	subs r4,r4,#1
	bne noMap

	ldmfd sp!,{r4-r9,lr}
	bx lr

;@----------------------------------------------------------------------------
gammaConvert:	;@ Takes value in r0(0-0xFF), gamma in r1(0-4)
				;@ returns new value in r0=0xFF
;@----------------------------------------------------------------------------
	rsb r2,r0,#0x100
	mul r3,r2,r2
	rsbs r2,r3,#0x10000
	rsb r3,r1,#4
	orr r0,r0,r0,lsl#8
	mul r2,r1,r2
	mla r0,r3,r0,r2
	mov r0,r0,lsr#10
	bx lr
;@----------------------------------------------------------------------------
contrastConvert:	;@ Takes values in r0 & r1(0-0xFF), contrast in r2(0-4)
					;@ returns new values in r0&r1=0xFF
;@----------------------------------------------------------------------------
	movs r2,r2,lsl#8
	addeq r2,r2,#0x80
	add r3,r0,r1
	sub r0,r0,r3,lsr#1
	sub r1,r1,r3,lsr#1
	mul r0,r2,r0
	mul r1,r2,r1
	mov r0,r0,asr#8+2
	mov r1,r1,asr#8+2
	add r0,r0,r3,lsr#1
	add r1,r1,r3,lsr#1
	bx lr

;@----------------------------------------------------------------------------
liConvR2:
	orr r2,r2,r2,lsl#2
	orr r2,r2,r2,lsl#4
;@----------------------------------------------------------------------------
lightConvert:	;@ Takes values in r0 & r1(0-0xFF), light in r2(0-0xFF)
				;@ returns new values in r0=0xFF
;@----------------------------------------------------------------------------
	sub r3,r0,r1
	mul r3,r2,r3
	add r0,r1,r3,lsr#8
	bx lr

;@----------------------------------------------------------------------------
paletteTxAll:				;@ Called from ui.c
	.type paletteTxAll STT_FUNC
;@----------------------------------------------------------------------------
	bx lr
;@----------------------------------------------------------------------------
updateLCDRefresh:
	.type updateLCDRefresh STT_FUNC
;@----------------------------------------------------------------------------
	ldr svvptr,=ks5360_0
	ldrb r1,[svvptr,#svvLCDVSize]
	b svRefW
;@----------------------------------------------------------------------------
setScreenRefresh:			;@ r0 in = WS scan line count.
	.type setScreenRefresh STT_FUNC
;@----------------------------------------------------------------------------
	bx lr

;@----------------------------------------------------------------------------
	.section .iwram, "ax", %progbits	;@ For the GBA
;@----------------------------------------------------------------------------
vblIrqHandler:
	.type vblIrqHandler STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r8,lr}
	bl vblSound1
	bl calculateFPS

	mov r6,#REG_BASE
	strh r6,[r6,#REG_DMA0CNT_H]	;@ DMA0 stop

	add r1,r6,#REG_DMA0SAD
	ldr r2,dmaScroll			;@ Setup DMA buffer for scrolling:
	ldmia r2!,{r4}				;@ Read
	add r3,r6,#REG_BG0HOFS		;@ DMA0 always goes here
	stmia r3,{r4}				;@ Set 1st value manually, HBL is AFTER 1st line
	ldr r4,=0xA6600001			;@ noIRQ hblank 32bit repeat incsrc inc_reloaddst, 1 word
	stmia r1,{r2-r4}			;@ DMA0 go

	add r1,r6,#REG_DMA3SAD

	ldr r2,=EMUPALBUFF			;@ DMA3 src, Palette transfer:
	mov r3,#BG_PALETTE			;@ DMA3 dst
	mov r4,#0x84000000			;@ noIRQ 32bit incsrc incdst
	orr r4,r4,#0x100			;@ 256 words (1024 bytes)
	stmia r1,{r2-r4}			;@ DMA3 go

	adr svvptr,ks5360_0
	ldr r0,GFX_BG0CNT
	str r0,[r6,#REG_BG0CNT]
	ldr r0,GFX_DISPCNT
	ldrb r1,[svvptr,#wsvLatchedDispCtrl]
//	tst r1,#0x01
//	biceq r0,r0,#0x0100			;@ Turn off Bg
	ldrb r2,gGfxMask
	bic r0,r0,r2,lsl#8
	strh r0,[r6,#REG_DISPCNT]

	ldr r0,[svvptr,#windowData]
	strh r0,[r6,#REG_WIN0H]
	mov r0,r0,lsr#16
	strh r0,[r6,#REG_WIN0V]
	ldr r0,=0x3B3B				;@ WinIN0/1, BG0, BG1, BG3, SPR & COL inside Win0
	and r2,r1,#0x30
	cmp r2,#0x20
	biceq r0,r0,#0x0200
	cmp r2,#0x30
	biceq r0,r0,#0x0002
	strh r0,[r6,#REG_WININ]


	ldrb r0,frameDone
	cmp r0,#0
	beq nothingNew
	ldr r0,=BG_GFX+0x8000
	bl svConvertScreen
	mov r0,#0
	strb r0,frameDone
nothingNew:

	bl scanKeys
	bl vblSound2
	ldmfd sp!,{r4-r8,lr}
	bx lr


;@----------------------------------------------------------------------------
refreshGfx:					;@ Called from C when changing scaling.
	.type refreshGfx STT_FUNC
;@----------------------------------------------------------------------------
	adr svvptr,ks5360_0
;@----------------------------------------------------------------------------
endFrameGfx:				;@ Called just before screen end (~line 159)	(r0-r3 safe to use)
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,lr}

	ldr r0,tmpScroll			;@ Destination
	bl copyScrollValues
;@--------------------------

	ldr r0,tmpScroll
	ldr r1,dmaScroll
	str r0,dmaScroll
	str r1,tmpScroll

	mov r0,#1
	strb r0,frameDone

	ldmfd sp!,{r3,lr}
	bx lr

;@----------------------------------------------------------------------------
tmpScroll:		.long SCROLLBUFF1
dmaScroll:		.long SCROLLBUFF2


gFlicker:		.byte 1
				.space 2
gTwitch:		.byte 0

gGfxMask:		.byte 0
frameDone:		.byte 0
				.byte 0,0
;@----------------------------------------------------------------------------
svVideoReset0:		;@ r0=NmiFunc, r1=IrqFunc, r2=ram+LUTs, r3=model
;@----------------------------------------------------------------------------
	adr svvptr,ks5360_0
	b svVideoReset
;@----------------------------------------------------------------------------
wsvReadIO:
	.type wsvReadIO STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,r12,lr}
	mov r0,r12
	adr svvptr,ks5360_0
	bl svRead
	ldmfd sp!,{r3,r12,lr}
	bx lr
;@----------------------------------------------------------------------------
wsvWriteIO:
	.type wsvWriteIO STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r3,r12,lr}
	mov r1,r0
	mov r0,r12
	adr svvptr,ks5360_0
	bl svWrite
	ldmfd sp!,{r3,r12,lr}
	bx lr
;@----------------------------------------------------------------------------
ks5360_0:
	.space ks5360Size
;@----------------------------------------------------------------------------

gfxState:
	.long 0
	.long 0
	.long 0,0
lcdSkip:
	.long 0

GFX_DISPCNT:
	.long 0
GFX_BG0CNT:
	.short 0
GFX_BG1CNT:
	.short 0

#ifdef GBA
	.section .sbss				;@ For the GBA
#else
	.section .bss
#endif
	.align 2
SCROLLBUFF1:
	.space 0x100*8				;@ Scrollbuffer.
SCROLLBUFF2:
	.space 0x100*8				;@ Scrollbuffer.
MAPPED_RGB:
	.space 0x2000				;@ 4096*2
EMUPALBUFF:
	.space 0x400

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
