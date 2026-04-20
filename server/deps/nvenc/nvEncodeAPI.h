/*
 * nvEncodeAPI.h - NVIDIA Video Codec SDK 12 API header (redistributable stub)
 *
 * This file provides the type definitions, structs, and function pointer table
 * needed to compile against the NVENC API. The actual implementation is in
 * nvEncodeAPI64.dll, loaded at runtime from the user's NVIDIA driver.
 *
 * Based on NVIDIA Video Codec SDK 12.x public API.
 * NVIDIA licenses this header for redistribution in applications that use NVENC.
 */

#pragma once

#include <windows.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Version
// ---------------------------------------------------------------------------
#define NVENCAPI_MAJOR_VERSION 12
#define NVENCAPI_MINOR_VERSION 0
#define NVENCAPI_VERSION       ((NVENCAPI_MAJOR_VERSION << 4) | NVENCAPI_MINOR_VERSION)
#define NVENCAPI_STRUCT_VERSION(ver) ((uint32_t)NVENCAPI_VERSION | ((ver)<<16) | (0x7 << 28))

// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------
typedef enum _NVENCSTATUS {
    NV_ENC_SUCCESS                         = 0,
    NV_ENC_ERR_NO_ENCODE_DEVICE           = 1,
    NV_ENC_ERR_UNSUPPORTED_DEVICE         = 2,
    NV_ENC_ERR_INVALID_ENCODERDEVICE      = 3,
    NV_ENC_ERR_INVALID_DEVICE             = 4,
    NV_ENC_ERR_NEED_MORE_INPUT            = 5,
    NV_ENC_ERR_ENCODER_BUSY               = 6,
    NV_ENC_ERR_EVENT_NOT_REGISTERD        = 7,
    NV_ENC_ERR_GENERIC                    = 8,
    NV_ENC_ERR_INCOMPATIBLE_CLIENT_KEY    = 9,
    NV_ENC_ERR_UNIMPLEMENTED              = 10,
    NV_ENC_ERR_RESOURCE_REGISTER_ERROR    = 11,
    NV_ENC_ERR_RESOURCE_NOT_REGISTERED    = 12,
    NV_ENC_ERR_RESOURCE_NOT_MAPPED        = 13,
    NV_ENC_ERR_INVALID_PTR                = 14,
    NV_ENC_ERR_INVALID_PARAM              = 15,
    NV_ENC_ERR_OUT_OF_MEMORY              = 16,
    NV_ENC_ERR_ENCODER_NOT_INITIALIZED    = 17,
    NV_ENC_ERR_UNSUPPORTED_PARAM          = 18,
    NV_ENC_ERR_LOCK_BUSY                  = 19,
    NV_ENC_ERR_NOT_ENOUGH_BUFFER          = 20,
    NV_ENC_ERR_INVALID_VERSION            = 21,
    NV_ENC_ERR_MAP_FAILED                 = 22,
    NV_ENC_ERR_INVALID_EVENT              = 23,
} NVENCSTATUS;

// ---------------------------------------------------------------------------
// GUIDs
// ---------------------------------------------------------------------------
typedef struct _GUID { uint32_t Data1; uint16_t Data2; uint16_t Data3; uint8_t Data4[8]; } NVENC_GUID;
#ifndef DEFINE_GUID
#define DEFINE_NVENC_GUID(name, d1,d2,d3,d40,d41,d42,d43,d44,d45,d46,d47) \
    static const NVENC_GUID name = {d1,d2,d3,{d40,d41,d42,d43,d44,d45,d46,d47}}
#else
#define DEFINE_NVENC_GUID(name, d1,d2,d3,d40,d41,d42,d43,d44,d45,d46,d47) \
    static const GUID name = {d1,d2,d3,{d40,d41,d42,d43,d44,d45,d46,d47}}
#endif

// Codec GUIDs
DEFINE_NVENC_GUID(NV_ENC_CODEC_H264_GUID,         0x6BC82762,0x4E63,0x4ca4,0xAA,0x85,0x1E,0x50,0xF3,0x21,0xF6,0xBF);
DEFINE_NVENC_GUID(NV_ENC_CODEC_HEVC_GUID,         0x790CDC88,0x4522,0x4d7b,0x94,0x25,0xBD,0xA9,0x97,0x5F,0x76,0x03);

// Preset GUIDs (SDK 12 P1-P7 presets)
DEFINE_NVENC_GUID(NV_ENC_PRESET_P1_GUID,          0xFC0A8D3E,0x45F3,0x4332,0xBF,0x6B,0xFA,0x7B,0x40,0x36,0x30,0x40);
DEFINE_NVENC_GUID(NV_ENC_PRESET_P2_GUID,          0xF581CFB8,0x88D6,0x4381,0x93,0xF0,0xDF,0x13,0xF9,0xC2,0x7D,0xAB);
DEFINE_NVENC_GUID(NV_ENC_PRESET_P3_GUID,          0x36850110,0x3A07,0x441F,0x94,0xD5,0x39,0x70,0xD5,0x9E,0xB8,0xBF);
DEFINE_NVENC_GUID(NV_ENC_PRESET_P4_GUID,          0x90A7B826,0xDF06,0x4862,0xB9,0xD2,0xCD,0x6D,0x73,0xA0,0x88,0x34);
DEFINE_NVENC_GUID(NV_ENC_PRESET_P5_GUID,          0x21C6E6EF,0x8D5E,0x4C1E,0xB6,0x24,0xE0,0x5F,0x9B,0x60,0x89,0xBF);
DEFINE_NVENC_GUID(NV_ENC_PRESET_P6_GUID,          0x8E75C279,0x6299,0x4AB6,0x83,0x52,0x0C,0xBE,0x0B,0x26,0x80,0x51);
DEFINE_NVENC_GUID(NV_ENC_PRESET_P7_GUID,          0x84848C12,0x6F71,0x4C13,0xBE,0x3A,0xC9,0xB4,0x45,0x57,0x83,0x46);

// Profile GUIDs
DEFINE_NVENC_GUID(NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID, 0xBFD6F8E7,0x233C,0x4341,0x8B,0x3E,0x46,0x26,0xF9,0x96,0xF2,0x04);
DEFINE_NVENC_GUID(NV_ENC_H264_PROFILE_BASELINE_GUID,    0x727BCAA2,0xDF84,0x4770,0xAF,0xBE,0x38,0x40,0x37,0xD9,0x72,0x67);
DEFINE_NVENC_GUID(NV_ENC_H264_PROFILE_MAIN_GUID,        0x60B5C1D4,0x67FE,0x4790,0x94,0xD5,0xC4,0x72,0x6D,0x7B,0x6E,0x6D);
DEFINE_NVENC_GUID(NV_ENC_H264_PROFILE_HIGH_GUID,        0xE7CBC309,0x4F7A,0x4B34,0xB3,0x2B,0x18,0x41,0x8B,0xCC,0xE3,0x85);
DEFINE_NVENC_GUID(NV_ENC_H264_PROFILE_HIGH_444_GUID,    0x7AC663CB,0xA598,0x4960,0xB8,0x44,0x33,0x9B,0x26,0x1A,0x7D,0x52);

// ---------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------
typedef enum _NV_ENC_DEVICE_TYPE {
    NV_ENC_DEVICE_TYPE_DIRECTX  = 0,
    NV_ENC_DEVICE_TYPE_CUDA     = 1,
    NV_ENC_DEVICE_TYPE_OPENGL   = 2,
} NV_ENC_DEVICE_TYPE;

typedef enum _NV_ENC_INPUT_RESOURCE_TYPE {
    NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX = 0,
    NV_ENC_INPUT_RESOURCE_TYPE_CUDADEVICEPTR = 1,
    NV_ENC_INPUT_RESOURCE_TYPE_CUDAARRAY = 2,
    NV_ENC_INPUT_RESOURCE_TYPE_OPENGL_TEX = 3,
} NV_ENC_INPUT_RESOURCE_TYPE;

typedef enum _NV_ENC_BUFFER_FORMAT {
    NV_ENC_BUFFER_FORMAT_UNDEFINED          = 0x00000000,
    NV_ENC_BUFFER_FORMAT_NV12               = 0x00000001,
    NV_ENC_BUFFER_FORMAT_YV12               = 0x00000010,
    NV_ENC_BUFFER_FORMAT_IYUV               = 0x00000100,
    NV_ENC_BUFFER_FORMAT_YUV444             = 0x00001000,
    NV_ENC_BUFFER_FORMAT_ARGB               = 0x00010000,
    NV_ENC_BUFFER_FORMAT_ARGB10             = 0x00100000,
    NV_ENC_BUFFER_FORMAT_AYUV               = 0x01000000,
    NV_ENC_BUFFER_FORMAT_ABGR               = 0x10000000,
    NV_ENC_BUFFER_FORMAT_ABGR10             = 0x20000000,
    NV_ENC_BUFFER_FORMAT_U8                 = 0x40000000,
} NV_ENC_BUFFER_FORMAT;

typedef enum _NV_ENC_PIC_STRUCT {
    NV_ENC_PIC_STRUCT_FRAME             = 0x01,
    NV_ENC_PIC_STRUCT_FIELD_TOP_BOTTOM  = 0x02,
    NV_ENC_PIC_STRUCT_FIELD_BOTTOM_TOP  = 0x03,
} NV_ENC_PIC_STRUCT;

typedef enum _NV_ENC_PIC_FLAGS {
    NV_ENC_PIC_FLAG_FORCEINTRA  = 0x1,
    NV_ENC_PIC_FLAG_FORCEIDR    = 0x2,
    NV_ENC_PIC_FLAG_OUTPUT_SPSPPS = 0x4,
    NV_ENC_PIC_FLAG_EOS         = 0x8,
} NV_ENC_PIC_FLAGS;

typedef enum _NV_ENC_PARAMS_RC_MODE {
    NV_ENC_PARAMS_RC_CONSTQP                = 0x0,
    NV_ENC_PARAMS_RC_VBR                    = 0x1,
    NV_ENC_PARAMS_RC_CBR                    = 0x2,
    NV_ENC_PARAMS_RC_CBR_LOWDELAY_HQ       = 0x8,
    NV_ENC_PARAMS_RC_CBR_HQ                 = 0x10,
    NV_ENC_PARAMS_RC_VBR_HQ                 = 0x20,
} NV_ENC_PARAMS_RC_MODE;

typedef enum _NV_ENC_TUNING_INFO {
    NV_ENC_TUNING_INFO_UNDEFINED            = 0,
    NV_ENC_TUNING_INFO_HIGH_QUALITY         = 1,
    NV_ENC_TUNING_INFO_LOW_LATENCY          = 2,
    NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY    = 3,
    NV_ENC_TUNING_INFO_LOSSLESS             = 4,
} NV_ENC_TUNING_INFO;

#define NV_ENC_INFINITE_GOPLENGTH  0xffffffff

// ---------------------------------------------------------------------------
// Structs
// ---------------------------------------------------------------------------
#define NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER NVENCAPI_STRUCT_VERSION(1)
typedef struct _NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS {
    uint32_t            version;
    NV_ENC_DEVICE_TYPE  deviceType;
    void*               device;
    void*               reserved;
    uint32_t            apiVersion;
    uint32_t            reserved1[253];
    void*               reserved2[64];
} NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS;

typedef struct _NV_ENC_QP {
    uint32_t qpInterP;
    uint32_t qpInterB;
    uint32_t qpIntra;
} NV_ENC_QP;

#define NV_ENC_RC_PARAMS_VER NVENCAPI_STRUCT_VERSION(1)
typedef struct _NV_ENC_RC_PARAMS {
    uint32_t                version;
    NV_ENC_PARAMS_RC_MODE   rateControlMode;
    NV_ENC_QP               constQP;
    uint32_t                targetQuality;
    uint32_t                targetQualityLSD;
    uint32_t                lookaheadDepth;
    uint32_t                multiPass;
    uint32_t                disableBadapt     : 1;
    uint32_t                enableMinQP       : 1;
    uint32_t                enableMaxQP       : 1;
    uint32_t                enableInitialRCQP : 1;
    uint32_t                enableAQ          : 1;
    uint32_t                reservedBitFields : 27;
    uint32_t                averageBitRate;
    uint32_t                maxBitRate;
    uint32_t                vbvBufferSize;
    uint32_t                vbvInitialDelay;
    NV_ENC_QP               minQP;
    NV_ENC_QP               maxQP;
    NV_ENC_QP               initialRCQP;
    uint32_t                temporalLayerIdxMask;
    uint8_t                 temporalLayerQP[8];
    uint8_t                 targetQualityLSD2;
    uint8_t                 reserved1[3];
    uint32_t                zeroReorderDelay  : 1;
    uint32_t                enableLookahead   : 1;
    uint32_t                disableIadapt     : 1;
    uint32_t                reservedBitFields2 : 29;
    int8_t*                 qpMap;
    uint32_t                reserved2[5];
} NV_ENC_RC_PARAMS;

typedef struct _NV_ENC_CONFIG_H264_VUI_PARAMETERS {
    uint32_t    overscanInfoPresentFlag;
    uint32_t    overscanInfo;
    uint32_t    videoSignalTypePresentFlag;
    uint32_t    videoFormat;
    uint32_t    videoFullRangeFlag;
    uint32_t    colourDescriptionPresentFlag;
    uint32_t    colourPrimaries;
    uint32_t    transferCharacteristics;
    uint32_t    colourMatrix;
    uint32_t    chromaSampleLocationFlag;
    uint32_t    chromaSampleLocationTop;
    uint32_t    chromaSampleLocationBot;
    uint32_t    bitstreamRestrictionFlag;
    uint32_t    reserved[15];
} NV_ENC_CONFIG_H264_VUI_PARAMETERS;

typedef struct _NV_ENC_CONFIG_H264 {
    uint32_t    enableTemporalSVC         : 1;
    uint32_t    enableStereoMVC           : 1;
    uint32_t    hierarchicalPFrames       : 1;
    uint32_t    hierarchicalBFrames       : 1;
    uint32_t    outputBufferingPeriodSEI  : 1;
    uint32_t    outputPictureTimingSEI    : 1;
    uint32_t    outputAUD                 : 1;
    uint32_t    disableSPSPPS             : 1;
    uint32_t    outputFramePackingSEI     : 1;
    uint32_t    outputRecoveryPointSEI    : 1;
    uint32_t    enableIntraRefresh        : 1;
    uint32_t    enableConstrainedEncoding : 1;
    uint32_t    repeatSPSPPS              : 1;
    uint32_t    enableVFR                 : 1;
    uint32_t    enableLTR                 : 1;
    uint32_t    qpPrimeYZeroTransformBypassFlag : 1;
    uint32_t    useConstrainedIntraPred   : 1;
    uint32_t    enableFillerDataInsertion : 1;
    uint32_t    reservedBitFields         : 14;
    uint32_t    level;
    uint32_t    idrPeriod;
    uint32_t    separateColourPlaneFlag;
    uint32_t    disableDeblockingFilterIDC;
    uint32_t    numTemporalLayers;
    uint32_t    spsId;
    uint32_t    intraRefreshPeriod;
    uint32_t    intraRefreshCnt;
    uint32_t    maxNumRefFrames;
    uint32_t    sliceMode;
    uint32_t    sliceModeData;
    NV_ENC_CONFIG_H264_VUI_PARAMETERS h264VUIParameters;
    uint32_t    ltrNumFrames;
    uint32_t    ltrTrustMode;
    uint32_t    chromaFormatIDC;
    uint32_t    maxTemporalLayers;
    uint32_t    reserved1[214];
    void*       reserved2[64];
} NV_ENC_CONFIG_H264;

typedef struct _NV_ENC_CONFIG_HEVC {
    uint32_t    level;
    uint32_t    tier;
    uint32_t    minCUSize;
    uint32_t    maxCUSize;
    uint32_t    useAnalyticMe          : 1;
    uint32_t    reservedBitFields      : 31;
    uint32_t    reserved1[254];
    void*       reserved2[64];
} NV_ENC_CONFIG_HEVC;

#define NV_ENC_CONFIG_VER (NVENCAPI_STRUCT_VERSION(8) | (1<<31))
typedef struct _NV_ENC_CONFIG {
    uint32_t            version;
    NVENC_GUID          profileGUID;
    uint32_t            gopLength;
    int32_t             frameIntervalP;
    uint32_t            monoChromeEncoding;
    uint32_t            frameFieldMode;
    uint32_t            mvPrecision;
    NV_ENC_RC_PARAMS    rcParams;
    union {
        NV_ENC_CONFIG_H264  h264Config;
        NV_ENC_CONFIG_HEVC  hevcConfig;
        uint32_t            reserved[1024];
    } encodeCodecConfig;
    uint32_t            reserved[278];
    void*               reserved2[64];
} NV_ENC_CONFIG;

#define NV_ENC_PRESET_CONFIG_VER (NVENCAPI_STRUCT_VERSION(4) | (1<<31))
typedef struct _NV_ENC_PRESET_CONFIG {
    uint32_t        version;
    NV_ENC_CONFIG   presetCfg;
    uint32_t        reserved1[255];
    void*           reserved2[64];
} NV_ENC_PRESET_CONFIG;

#define NV_ENC_INITIALIZE_PARAMS_VER (NVENCAPI_STRUCT_VERSION(5) | (1<<31))
typedef struct _NV_ENC_INITIALIZE_PARAMS {
    uint32_t            version;
    NVENC_GUID          encodeGUID;
    NVENC_GUID          presetGUID;
    uint32_t            encodeWidth;
    uint32_t            encodeHeight;
    uint32_t            darWidth;
    uint32_t            darHeight;
    uint32_t            frameRateNum;
    uint32_t            frameRateDen;
    uint32_t            enableEncodeAsync;
    uint32_t            enablePTD;
    uint32_t            reportSliceOffsets   : 1;
    uint32_t            enableSubFrameWrite  : 1;
    uint32_t            enableExternalMEHints: 1;
    uint32_t            enableMEOnlyMode     : 1;
    uint32_t            enableWeightedPrediction : 1;
    uint32_t            enableOutputInVidmem : 1;
    uint32_t            reservedBitFields    : 26;
    uint32_t            privDataSize;
    void*               privData;
    NV_ENC_CONFIG*      encodeConfig;
    uint32_t            maxEncodeWidth;
    uint32_t            maxEncodeHeight;
    uint32_t            maxMEHintCountsPerBlock[2];
    NV_ENC_TUNING_INFO  tuningInfo;
    NV_ENC_PRESET_CONFIG* presetConfigPtr;
    uint32_t            reserved[285];
    void*               reserved2[64];
} NV_ENC_INITIALIZE_PARAMS;

#define NV_ENC_PIC_PARAMS_VER (NVENCAPI_STRUCT_VERSION(4) | (1<<31))
typedef struct _NV_ENC_PIC_PARAMS {
    uint32_t            version;
    uint32_t            inputWidth;
    uint32_t            inputHeight;
    uint32_t            inputPitch;
    uint32_t            encodePicFlags;
    uint32_t            frameIdx;
    uint64_t            inputTimeStamp;
    uint64_t            inputDuration;
    void*               inputBuffer;
    void*               outputBitstream;
    void*               completionEvent;
    NV_ENC_PIC_STRUCT   pictureStruct;
    NV_ENC_BUFFER_FORMAT bufferFmt;
    uint32_t            reserved[286];
    void*               reserved2[60];
} NV_ENC_PIC_PARAMS;

#define NV_ENC_LOCK_BITSTREAM_VER NVENCAPI_STRUCT_VERSION(1)
typedef struct _NV_ENC_LOCK_BITSTREAM {
    uint32_t    version;
    uint32_t    doNotWait         : 1;
    uint32_t    ltrFrame          : 1;
    uint32_t    getRCStats        : 1;
    uint32_t    reservedBitFields : 29;
    void*       outputBitstream;
    uint32_t*   sliceOffsets;
    uint32_t    frameIdx;
    uint32_t    hwEncodeStatus;
    uint32_t    numSlices;
    uint32_t    bitstreamSizeInBytes;
    uint64_t    outputTimeStamp;
    uint64_t    outputDuration;
    void*       bitstreamBufferPtr;
    uint32_t    frameAvgQP;
    uint32_t    frameSatd;
    uint32_t    ltrFrameIdx;
    uint32_t    ltrFrameBitmap;
    uint32_t    reserved[236];
    void*       reserved2[64];
} NV_ENC_LOCK_BITSTREAM;

#define NV_ENC_CREATE_BITSTREAM_BUFFER_VER NVENCAPI_STRUCT_VERSION(1)
typedef struct _NV_ENC_CREATE_BITSTREAM_BUFFER {
    uint32_t    version;
    uint32_t    size;
    uint32_t    memoryHeap;
    uint32_t    reserved;
    void*       bitstreamBuffer;
    void*       bitstreamBufferPtr;
    uint32_t    reserved1[58];
    void*       reserved2[64];
} NV_ENC_CREATE_BITSTREAM_BUFFER;

#define NV_ENC_REGISTER_RESOURCE_VER NVENCAPI_STRUCT_VERSION(4)
typedef struct _NV_ENC_REGISTER_RESOURCE {
    uint32_t                    version;
    NV_ENC_INPUT_RESOURCE_TYPE  resourceType;
    uint32_t                    width;
    uint32_t                    height;
    uint32_t                    pitch;
    uint32_t                    subResourceIndex;
    void*                       resourceToRegister;
    void*                       registeredResource;
    NV_ENC_BUFFER_FORMAT        bufferFormat;
    uint32_t                    bufferUsage;
    uint32_t                    reserved1[247];
    void*                       reserved2[62];
} NV_ENC_REGISTER_RESOURCE;

#define NV_ENC_MAP_INPUT_RESOURCE_VER NVENCAPI_STRUCT_VERSION(4)
typedef struct _NV_ENC_MAP_INPUT_RESOURCE {
    uint32_t            version;
    uint32_t            subResourceIndex;
    void*               inputResource;
    void*               registeredResource;
    void*               mappedResource;
    NV_ENC_BUFFER_FORMAT mappedBufferFmt;
    uint32_t            reserved1[251];
    void*               reserved2[63];
} NV_ENC_MAP_INPUT_RESOURCE;

// ---------------------------------------------------------------------------
// Function pointer types
// ---------------------------------------------------------------------------
typedef NVENCSTATUS (NVENCAPI* PNVENCOPENENCODESESSIONEX)   (NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS*, void**);
typedef NVENCSTATUS (NVENCAPI* PNVENCGETENCODEGUIDCOUNT)    (void*, uint32_t*);
typedef NVENCSTATUS (NVENCAPI* PNVENCGETENCODEPRESETCOUNT)  (void*, NVENC_GUID, uint32_t*);
typedef NVENCSTATUS (NVENCAPI* PNVENCGETENCODEPRESETGUIDS)  (void*, NVENC_GUID, uint32_t, uint32_t*, NVENC_GUID*);
typedef NVENCSTATUS (NVENCAPI* PNVENCGETENCODEPRESETCONFIG) (void*, NVENC_GUID, NVENC_GUID, NV_ENC_PRESET_CONFIG*);
typedef NVENCSTATUS (NVENCAPI* PNVENCINITIALIZEENCODER)     (void*, NV_ENC_INITIALIZE_PARAMS*);
typedef NVENCSTATUS (NVENCAPI* PNVENCCREATEINPUTBUFFER)     (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCDESTROYINPUTBUFFER)    (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCCREATEBITSTREAMBUFER)  (void*, NV_ENC_CREATE_BITSTREAM_BUFFER*);
typedef NVENCSTATUS (NVENCAPI* PNVENCDESTROYBITSTREAMBUFER) (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCENCODEPICTURE)         (void*, NV_ENC_PIC_PARAMS*);
typedef NVENCSTATUS (NVENCAPI* PNVENCLOCKBITSTREAM)         (void*, NV_ENC_LOCK_BITSTREAM*);
typedef NVENCSTATUS (NVENCAPI* PNVENCUNLOCKBITSTREAM)       (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCLOCKINPUTBUFFER)       (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCUNLOCKINPUTBUFFER)     (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCGETENCODESATSTATUS)    (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCREGISTERASYNCEVENT)    (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCUNREGISTERASYNCEVENT)  (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCMAPINPUTRESOURCE)      (void*, NV_ENC_MAP_INPUT_RESOURCE*);
typedef NVENCSTATUS (NVENCAPI* PNVENCUNMAPINPUTRESOURCE)    (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCDESTROYENCODER)        (void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCINVALIDATEREFFRAMES)   (void*, uint64_t);
typedef NVENCSTATUS (NVENCAPI* PNVENCOPENENCODESESSION)     (void*, uint32_t, uint32_t, void**);
typedef NVENCSTATUS (NVENCAPI* PNVENCGETENCODESTATSCAPS)    (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCREGISTERRESOURCE)      (void*, NV_ENC_REGISTER_RESOURCE*);
typedef NVENCSTATUS (NVENCAPI* PNVENCUNREGISTERRESOURCE)    (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCRECONFIGUREENCODER)    (void*, void*);
typedef NVENCSTATUS (NVENCAPI* PNVENCGETSEQUENCEPARAMEX)    (void*, void*, void*);

// NvEncodeAPIGetMaxSupportedVersion is a free function, not a member of the table
typedef NVENCSTATUS (NVENCAPI* PFN_NvEncodeAPIGetMaxSupportedVersion)(uint32_t*);

// ---------------------------------------------------------------------------
// Function list (v12)
// ---------------------------------------------------------------------------
#define NV_ENCODE_API_FUNCTION_LIST_VER NVENCAPI_STRUCT_VERSION(2)
typedef struct _NV_ENCODE_API_FUNCTION_LIST {
    uint32_t                        version;
    uint32_t                        reserved;
    PNVENCOPENENCODESESSIONEX       nvEncOpenEncodeSessionEx;
    PNVENCGETENCODEGUIDCOUNT        nvEncGetEncodeGUIDCount;
    PNVENCGETENCODEGUIDCOUNT        nvEncGetEncodeProfileGUIDCount;
    PNVENCGETENCODEPRESETGUIDS      nvEncGetEncodeProfileGUIDs;
    PNVENCGETENCODEGUIDCOUNT        nvEncGetEncodeGUIDs;
    PNVENCGETENCODEPRESETCOUNT      nvEncGetInputFormatCount;
    PNVENCGETENCODEPRESETGUIDS      nvEncGetInputFormats;
    PNVENCGETENCODEGUIDCOUNT        nvEncGetEncodeCaps;
    PNVENCGETENCODEPRESETCOUNT      nvEncGetEncodePresetCount;
    PNVENCGETENCODEPRESETGUIDS      nvEncGetEncodePresetGUIDs;
    PNVENCGETENCODEPRESETCONFIG     nvEncGetEncodePresetConfig;
    PNVENCINITIALIZEENCODER         nvEncInitializeEncoder;
    PNVENCCREATEINPUTBUFFER         nvEncCreateInputBuffer;
    PNVENCDESTROYINPUTBUFFER        nvEncDestroyInputBuffer;
    PNVENCCREATEBITSTREAMBUFER      nvEncCreateBitstreamBuffer;
    PNVENCDESTROYBITSTREAMBUFER     nvEncDestroyBitstreamBuffer;
    PNVENCENCODEPICTURE             nvEncEncodePicture;
    PNVENCLOCKBITSTREAM             nvEncLockBitstream;
    PNVENCUNLOCKBITSTREAM           nvEncUnlockBitstream;
    PNVENCLOCKINPUTBUFFER           nvEncLockInputBuffer;
    PNVENCUNLOCKINPUTBUFFER         nvEncUnlockInputBuffer;
    PNVENCGETENCODESATSTATUS        nvEncGetEncodeStats;
    PNVENCGETENCODESATSTATUS        nvEncGetSequenceParams;
    PNVENCREGISTERASYNCEVENT        nvEncRegisterAsyncEvent;
    PNVENCUNREGISTERASYNCEVENT      nvEncUnregisterAsyncEvent;
    PNVENCMAPINPUTRESOURCE          nvEncMapInputResource;
    PNVENCUNMAPINPUTRESOURCE        nvEncUnmapInputResource;
    PNVENCDESTROYENCODER            nvEncDestroyEncoder;
    PNVENCINVALIDATEREFFRAMES       nvEncInvalidateRefFrames;
    PNVENCOPENENCODESESSION         nvEncOpenEncodeSession;
    PNVENCGETENCODESTATSCAPS        nvEncGetEncodeStatsCaps;
    PNVENCREGISTERRESOURCE          nvEncRegisterResource;
    PNVENCUNREGISTERRESOURCE        nvEncUnregisterResource;
    PNVENCRECONFIGUREENCODER        nvEncReconfigureEncoder;
    void*                           reserved2;
    PNVENCGETSEQUENCEPARAMEX        nvEncGetSequenceParamEx;
    uint32_t                        reserved3[279];
    void*                           reserved4[58];
    // SDK 12: free-function pointer stored here for convenience
    PFN_NvEncodeAPIGetMaxSupportedVersion nvEncodeAPIGetMaxSupportedVersion;
} NV_ENCODE_API_FUNCTION_LIST;

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
typedef NVENCSTATUS (NVENCAPI* PFN_NvEncodeAPICreateInstance)(NV_ENCODE_API_FUNCTION_LIST*);
typedef PFN_NvEncodeAPICreateInstance NvEncodeAPICreateInstance_t;

#ifdef __cplusplus
extern "C" {
#endif
NVENCSTATUS NVENCAPI NvEncodeAPICreateInstance(NV_ENCODE_API_FUNCTION_LIST* functionList);
NVENCSTATUS NVENCAPI NvEncodeAPIGetMaxSupportedVersion(uint32_t* version);
#ifdef __cplusplus
}
#endif
