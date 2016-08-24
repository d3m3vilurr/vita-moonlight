// https://gitlab.slkdev.net/RPCS3/rpcs3/blob/5b225ad59f/rpcs3/Emu/PSP2/Modules/sceVideodec.h
#ifndef _PSP2_VIDEODEC_H_
#define _PSP2_VIDEODEC_H_

#include <psp2/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SceVideodecQueryInitInfoHwAvcdec {
	SceUInt32 size;
	SceUInt32 horizontal;
	SceUInt32 vertical;
	SceUInt32 numOfRefFrames;
	SceUInt32 numOfStreams;
} SceVideodecQueryInitInfoHwAvcdec;

//typedef union SceVideodecQueryInitInfo {
//	SceUInt8 reserved[32];
//	SceVideodecQueryInitInfoHwAvcdec hwAvc;
//} SceVideodecQueryInitInfo;

typedef struct SceVideodecTimeStamp {
	SceUInt32 upper;
	SceUInt32 lower;
} SceVideodecTimeStamp;

typedef struct SceAvcdecQueryDecoderInfo {
	SceUInt32 horizontal;
	SceUInt32 vertical;
	SceUInt32 numOfRefFrames;
} SceAvcdecQueryDecoderInfo;

typedef struct SceAvcdecDecoderInfo {
	SceUInt32 frameMemSize;
} SceAvcdecDecoderInfo;

typedef struct SceAvcdecBuf {
	void *pBuf;
	SceUInt32 size;
} SceAvcdecBuf;

typedef struct SceAvcdecCtrl {
	SceUInt32 handle;
	SceAvcdecBuf frameBuf;
} SceAvcdecCtrl;

typedef struct SceAvcdecAu {
	SceVideodecTimeStamp pts;
	SceVideodecTimeStamp dts;
	SceAvcdecBuf es;
} SceAvcdecAu;

typedef struct SceAvcdecInfo {
	SceUInt32 numUnitsInTick;
	SceUInt32 timeScale;
	SceUInt8 fixedFrameRateFlag;

	SceUInt8 aspectRatioIdc;
	SceUInt16 sarWidth;
	SceUInt16 sarHeight;

	SceUInt8 colourPrimaries;
	SceUInt8 transferCharacteristics;
	SceUInt8 matrixCoefficients;

	SceUInt8 videoFullRangeFlag;

	SceUInt8 picStruct[2];
	SceUInt8 ctType;

	SceUInt8 padding[3];
} SceAvcdecInfo;

typedef struct SceAvcdecFrameOptionRGBA {
	SceUInt8 alpha;
	SceUInt8 cscCoefficient;
	SceUInt8 reserved[14];
} SceAvcdecFrameOptionRGBA;

//typedef union SceAvcdecFrameOption {
//	SceUInt8 reserved[16];
//	SceAvcdecFrameOptionRGBA rgba;
//} SceAvcdecFrameOption;

typedef struct SceAvcdecFrame {
	SceUInt32 pixelType;
	SceUInt32 framePitch;
	SceUInt32 frameWidth;
	SceUInt32 frameHeight;

	SceUInt32 horizontalSize;
	SceUInt32 verticalSize;

	SceUInt32 frameCropLeftOffset;
	SceUInt32 frameCropRightOffset;
	SceUInt32 frameCropTopOffset;
	SceUInt32 frameCropBottomOffset;

	SceAvcdecFrameOptionRGBA opt;

	void *pPicture[2];
} SceAvcdecFrame;

typedef struct SceAvcdecPicture {
	SceUInt32 size;
	SceAvcdecFrame frame;
	SceAvcdecInfo info;
} SceAvcdecPicture;

typedef struct SceAvcdecArrayPicture {
	SceUInt32 numOfOutput;
	SceUInt32 numOfElm;
	SceAvcdecPicture **pPicture;
} SceAvcdecArrayPicture;

extern SceInt32 sceVideodecInitLibrary(SceUInt32 codecType, const SceVideodecQueryInitInfoHwAvcdec *pInitInfo);
extern SceInt32 sceVideodecTermLibrary(SceUInt32 codecType);
extern SceInt32 sceAvcdecQueryDecoderMemSize(SceUInt32 codecType, const SceAvcdecQueryDecoderInfo *pDecoderInfo, SceAvcdecDecoderInfo *pMemInfo);
extern SceInt32 sceAvcdecCreateDecoder(SceUInt32 codecType, SceAvcdecCtrl *pCtrl, const SceAvcdecQueryDecoderInfo *pDecoderInfo);
extern SceInt32 sceAvcdecDeleteDecoder(SceAvcdecCtrl *pCtrl);
extern SceInt32 sceAvcdecDecodeAvailableSize(SceAvcdecCtrl *pCtrl);
extern SceInt32 sceAvcdecDecode(SceAvcdecCtrl *pCtrl, const SceAvcdecAu *pAu, SceAvcdecArrayPicture *pArrayPicture);
extern SceInt32 sceAvcdecDecodeStop(SceAvcdecCtrl *pCtrl, SceAvcdecArrayPicture *pArrayPicture);
extern SceInt32 sceAvcdecDecodeFlush(SceAvcdecCtrl *pCtrl);

#ifdef __cplusplus
}
#endif

#endif
