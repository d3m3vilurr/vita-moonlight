// https://gitlab.slkdev.net/RPCS3/rpcs3/blob/5b225ad59f/rpcs3/Emu/PSP2/Modules/sceVideodec.h
#ifndef _PSP2_VIDEODEC_H_
#define _PSP2_VIDEODEC_H_

#include <psp2/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    SCE_VIDEODEC_ERROR_INVALID_PARAM            = 0x80620802,
    SCE_VIDEODEC_ERROR_OUT_OF_MEMORY            = 0x80620803,
    SCE_VIDEODEC_ERROR_INVALID_STATE            = 0x80620804,
    SCE_VIDEODEC_ERROR_UNSUPPORT_IMAGE_SIZE     = 0x80620805,
    SCE_VIDEODEC_ERROR_INVALID_COLOR_FORMAT     = 0x80620806,
    SCE_VIDEODEC_ERROR_NOT_PHY_CONTINUOUS_MEMORY    = 0x80620807,
    SCE_VIDEODEC_ERROR_ALREADY_USED             = 0x80620808,
    SCE_VIDEODEC_ERROR_INVALID_POINTER          = 0x80620809,
    SCE_VIDEODEC_ERROR_ES_BUFFER_FULL           = 0x8062080A,
    SCE_VIDEODEC_ERROR_INITIALIZE               = 0x8062080B,
    SCE_VIDEODEC_ERROR_NOT_INITIALIZE           = 0x8062080C,
    SCE_VIDEODEC_ERROR_INVALID_STREAM           = 0x8062080D,
    SCE_VIDEODEC_ERROR_INVALID_ARGUMENT_SIZE    = 0x8062080E,
    SCE_AVCDEC_ERROR_INVALID_PARAM              = 0x80620002,
    SCE_AVCDEC_ERROR_OUT_OF_MEMORY              = 0x80620003,
    SCE_AVCDEC_ERROR_INVALID_STATE              = 0x80620004,
    SCE_AVCDEC_ERROR_UNSUPPORT_IMAGE_SIZE       = 0x80620005,
    SCE_AVCDEC_ERROR_INVALID_COLOR_FORMAT       = 0x80620006,
    SCE_AVCDEC_ERROR_NOT_PHY_CONTINUOUS_MEMORY  = 0x80620007,
    SCE_AVCDEC_ERROR_ALREADY_USED               = 0x80620008,
    SCE_AVCDEC_ERROR_INVALID_POINTER            = 0x80620009,
    SCE_AVCDEC_ERROR_ES_BUFFER_FULL             = 0x8062000A,
    SCE_AVCDEC_ERROR_INITIALIZE                 = 0x8062000B,
    SCE_AVCDEC_ERROR_NOT_INITIALIZE             = 0x8062000C,
    SCE_AVCDEC_ERROR_INVALID_STREAM             = 0x8062000D,
    SCE_AVCDEC_ERROR_INVALID_ARGUMENT_SIZE      = 0x8062000E
};

#define SCE_VIDEODEC_TYPE_HW_AVCDEC     0x1001U

#define SCE_AVCDEC_PIXEL_RGBA8888               0x0U
#define SCE_AVCDEC_PIXEL_RGB565                 0x1U
#define SCE_AVCDEC_PIXEL_RGBA5551               0x2U
#define SCE_AVCDEC_PIXEL_YUV420_RASTER          0x10U
#define SCE_AVCDEC_PIXEL_YUV420_PACKED_RASTER   0x20U
#define SCE_AVCDEC_OPTION_ENABLE                (1<<31)

#define SCE_AVCDEC_CSC_COEFFICIENT_DEFAULT      0
#define SCE_AVCDEC_CSC_COEFFICIENT_ITU601       1
#define SCE_AVCDEC_CSC_COEFFICIENT_ITU709       2
#define SCE_AVCDEC_CSC_COEFFICIENT_ITU601_FULL  3
#define SCE_AVCDEC_CSC_COEFFICIENT_ITU709_FULL  4

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
