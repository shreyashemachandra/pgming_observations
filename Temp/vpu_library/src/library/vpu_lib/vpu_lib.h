/*******************************************************************************
 *
 * (C) 2015 ALLGO EMBEDDED SYSTEMS PVT LTD.
 *
 *  REVISION HISTORY
 *
 *  dd/mm/yy  Version   Description         Author
 *  --------  -------   -----------         ------ 
 *  17/11/15    01      Copied as it is     Shreyas
 *                      from linux vpu
 *                      Changed prototype
 *                      of vpu_Init
 *  24/11/15    02      Commented all the   Shreyas
 *                      Macros not related 
 *                      to imx6
 *  
 ******************************************************************************/ 

#ifndef __VPU__LIB__H
#define __VPU__LIB__H

typedef unsigned char Uint8;
typedef unsigned long Uint32;
typedef unsigned short Uint16;
typedef unsigned long long Uint64;
typedef long long Int64;
typedef Uint32 PhysicalAddress;
typedef Uint32 VirtualAddress;

#define STREAM_FULL_EMPTY_CHECK_DISABLE 0
#define BUF_PIC_FLUSH			1
#define BUF_PIC_RESET			0

#define BIT_REG_MARGIN			0x4000

#define PRJ_TRISTAN     		0xF000
#define PRJ_TRISTAN_REV			0xF001
#define PRJ_PRISM_CX			0xF002
#define PRJ_SHIVA       		0xF003
#define PRJ_PRISM_EX			0xF004
#define PRJ_BODA_CX_4			0xF005
#define PRJ_CODA_DX_6M			0xF100
#define PRJ_CODA_DX_8			0xF306
#define PRJ_BODA_DX_4V			0xF405
#define PRJ_BODADX7X			0xF009
#define	PRJ_CODAHX_14			0xF00A
#define PRJ_CODA7541			0xF012
#define PRJ_CODA_960			0xF020

#define MAX_NUM_INSTANCE		32

#define DC_TABLE_INDEX0		    0
#define AC_TABLE_INDEX0		    1
#define DC_TABLE_INDEX1		    2
#define AC_TABLE_INDEX1		    3

typedef enum {
    STD_MPEG4 = 0,
    STD_H263 = 1,
    STD_AVC = 2,
    STD_VC1 = 3,
    STD_MPEG2 = 4,
    STD_DIV3 =5,
    STD_RV = 6,
    STD_MJPG = 7,
    STD_AVS = 8,
    STD_VP8 = 9
} CodStd;

typedef enum {
    RETCODE_SUCCESS = 0,
    RETCODE_FAILURE = -1,
    RETCODE_INVALID_HANDLE = -2,
    RETCODE_INVALID_PARAM = -3,
    RETCODE_INVALID_COMMAND = -4,
    RETCODE_ROTATOR_OUTPUT_NOT_SET = -5,
    RETCODE_ROTATOR_STRIDE_NOT_SET = -11,
    RETCODE_FRAME_NOT_COMPLETE = -6,
    RETCODE_INVALID_FRAME_BUFFER = -7,
    RETCODE_INSUFFICIENT_FRAME_BUFFERS = -8,
    RETCODE_INVALID_STRIDE = -9,
    RETCODE_WRONG_CALL_SEQUENCE = -10,
    RETCODE_CALLED_BEFORE = -12,
    RETCODE_NOT_INITIALIZED = -13,
    RETCODE_DEBLOCKING_OUTPUT_NOT_SET = -14,
    RETCODE_NOT_SUPPORTED = -15,
    RETCODE_REPORT_BUF_NOT_SET = -16,
    RETCODE_FAILURE_TIMEOUT = -17,
    RETCODE_MEMORY_ACCESS_VIOLATION = -18,
    RETCODE_JPEG_EOS = -19,
    RETCODE_JPEG_BIT_EMPTY = -20
} RetCode;

typedef enum {
    LINEAR_FRAME_MAP = 0,
    TILED_FRAME_MB_RASTER_MAP = 1,
    TILED_FIELD_MB_RASTER_MAP = 2,
    TILED_MAP_TYPE_MAX
} GDI_TILED_MAP_TYPE;

typedef enum {
    ENABLE_ROTATION,
    DISABLE_ROTATION,
    ENABLE_MIRRORING,
    DISABLE_MIRRORING,
    ENABLE_DERING,
    DISABLE_DERING,
    SET_MIRROR_DIRECTION,
    SET_ROTATION_ANGLE,
    SET_ROTATOR_OUTPUT,
    SET_ROTATOR_STRIDE,
    ENC_GET_SPS_RBSP,
    ENC_GET_PPS_RBSP,
    DEC_SET_SPS_RBSP,
    DEC_SET_PPS_RBSP,
    ENC_PUT_MP4_HEADER,
    ENC_PUT_AVC_HEADER,
    ENC_SET_SEARCHRAM_PARAM,
    ENC_GET_VIDEO_HEADER,
    ENC_GET_VOS_HEADER,
    ENC_GET_VO_HEADER,
    ENC_GET_VOL_HEADER,
    ENC_GET_JPEG_HEADER,
    ENC_SET_INTRA_MB_REFRESH_NUMBER,
    DEC_SET_DEBLOCK_OUTPUT,
    ENC_ENABLE_HEC,
    ENC_DISABLE_HEC,
    ENC_SET_SLICE_INFO,
    ENC_SET_GOP_NUMBER,
    ENC_SET_INTRA_QP,
    ENC_SET_BITRATE,
    ENC_SET_FRAME_RATE,
    ENC_SET_REPORT_MBINFO,
    ENC_SET_REPORT_MVINFO,
    ENC_SET_REPORT_SLICEINFO,
    DEC_SET_REPORT_BUFSTAT,
    DEC_SET_REPORT_MBINFO,
    DEC_SET_REPORT_MVINFO,
    DEC_SET_REPORT_USERDATA,
    SET_DBK_OFFSET,
    SET_WRITE_MEM_PROTECT,

    ENC_SET_SUB_FRAME_SYNC,
    ENC_ENABLE_SUB_FRAME_SYNC,
    ENC_DISABLE_SUB_FRAME_SYNC,

    DEC_SET_FRAME_DELAY,
    ENC_SET_INTRA_REFRESH_MODE,
    ENC_ENABLE_SOF_STUFF
} CodecCommand;

typedef struct {
    Uint32 strideY;
    Uint32 strideC;
    int myIndex;
    PhysicalAddress bufY;
    PhysicalAddress bufCb;
    PhysicalAddress bufCr;
    PhysicalAddress bufMvCol; //TODO: Check if this is used ?
} FrameBuffer;

typedef struct {
    Uint32 left;
    Uint32 top;
    Uint32 right;
    Uint32 bottom;
} Rect;


typedef enum {
    FORMAT_420,
    FORMAT_422,
    FORMAT_224,
    FORMAT_444,
    FORMAT_400
} ChromaFormat;

typedef struct {
    int DbkOffsetA;
    int DbkOffsetB;
    int DbkOffsetEnable;
} DbkOffset;


/* Decode struct and definition */
typedef struct CodecInst DecInst;
typedef DecInst *DecHandle;

typedef struct {
    CodStd bitstreamFormat;
    PhysicalAddress bitstreamBuffer;
    Uint8 *pBitStream;
    int bitstreamBufferSize;
    int qpReport;
    int mp4DeblkEnable;			//TODO: Check this
    int reorderEnable;
    int chromaInterleave;
    int picWidth;
    int picHeight;
    int streamStartByteOffset;
    PhysicalAddress psSaveBuffer;
    int psSaveBufferSize;
    int mp4Class;
    int avcExtension;
    int mapType;
    int tiled2LinearEnable;
    int bitstreamMode;

} DecOpenParam;



typedef struct {
    int picWidth;
    int picHeight;


    Rect picCropRect;

    int mp4_dataPartitionEnable;
    int mp4_reversibleVlcEnable;
    int mp4_shortVideoHeader;

    int minFrameBufferCount;
    int frameBufDelay;
    int nextDecodedIdxNum;
    int normalSliceSize;
    int worstSliceSize;

    int streamInfoObtained;
    int profile;
    int level;
    int interlace;
    int constraint_set_flag[4];
    int direct8x8Flag;
    int vc1_psf;
    int aspectRateInfo;
    Uint32 errorcode;
 Uint32 frameRateRes;
 Uint32 frameRateDiv;   
    int bitRate;

//TOD: Check if this is required.    DecReportBufSize reportBufSize;	
} DecInitialInfo;

typedef struct {
    PhysicalAddress bufferBase;
    int bufferSize;
} ExtBufCfg;


typedef struct {
    PhysicalAddress sliceSaveBuffer;
    int sliceSaveBufferSize;
} DecAvcSliceBufInfo;

typedef struct {
    int maxMbX;
    int maxMbY;
    int maxMbNum;
} DecMaxFrmInfo;

typedef struct {
    ExtBufCfg avcSliceBufInfo;
    ExtBufCfg vp8MbDataBufInfo;
    DecMaxFrmInfo	maxDecFrmInfo;
} DecBufInfo;

typedef enum {
    PARA_TYPE_FRM_BUF_STATUS = 1,
    PARA_TYPE_MB_PARA = 2,
    PARA_TYPE_MV = 4,
} ExtParaType;

typedef struct {

    int dispReorderBuf;
    int iframeSearchEnable;
    int skipframeMode;
    int skipframeNum;
 
} DecParam;


/* MVC specific picture information */
typedef struct {
    int viewIdxDisplay;
    int viewIdxDecoded;
} MvcPicInfo;

/* AVC specific SEI information (frame packing arrangement SEI) */
typedef struct {
    unsigned exist;
    unsigned frame_packing_arrangement_id;
    unsigned frame_packing_arrangement_cancel_flag;
    unsigned quincunx_sampling_flag;
    unsigned spatial_flipping_flag;
    unsigned frame0_flipped_flag;
    unsigned field_views_flag;
    unsigned current_frame_is_frame0_flag;
    unsigned frame0_self_contained_flag;
    unsigned frame1_self_contained_flag;
    unsigned frame_packing_arrangement_ext_flag;
    unsigned frame_packing_arrangement_type;
    unsigned content_interpretation_type;
    unsigned frame0_grid_position_x;
    unsigned frame0_grid_position_y;
    unsigned frame1_grid_position_x;
    unsigned frame1_grid_position_y;
    unsigned frame_packing_arrangement_repetition_period;
} AvcFpaSei;

typedef struct {
    int indexFrameDisplay;
    int indexFrameDecoded;
    int NumDecFrameBuf;
    int picType;
    int picTypeFirst;  
    int idrFlg;      
    int numOfErrMBs;
    int hScaleFlag;
    int vScaleFlag;
    int indexFrameRangemap;
    int notSufficientPsBuffer;
    int notSufficientSliceBuffer;
    int decodingSuccess;
    int interlacedFrame;
    int mp4PackedPBframe;
    int h264Npf;

    int pictureStructure;
    int topFieldFirst;
    int repeatFirstField;
    union {
        int progressiveFrame;
        int vc1_repeatFrame;
    };
    int fieldSequence;

    int decPicHeight;
    int decPicWidth;
    Rect decPicCrop;

    int aspectRateInfo;
     Uint32 frameRateRes;   
    Uint32 frameRateDiv;   
    MvcPicInfo mvcPicInfo; 
    AvcFpaSei avcFpaSei;

    int frameStartPos;   
    int frameEndPos;    
    int consumedByte;   

} DecOutputInfo;


/**
  \struct VuiParam
  \brief  VUI parameters
  */
typedef struct {
    int video_signal_type_pres_flag;
    char video_format;
    char video_full_range_flag;
    int colour_descrip_pres_flag;
    char colour_primaries;
    char transfer_characteristics;
    char matrix_coeff;
} VuiParam;




typedef enum {
    SPS_RBSP,
    PPS_RBSP,
    SPS_RBSP_MVC,
    PPS_RBSP_MVC
} AvcHeaderType;

typedef struct {
    Uint32 gopNumber;
    Uint32 intraQp;
    Uint32 bitrate;
    Uint32 framerate;
} stChangeRcPara;

/*
 * The firmware version is retrieved from bitcode table.
 *
 * The library version convention:
 * lib_major increases when a new platform support is added
 * lib_minor increases when one firmware is upgraded
 * lib_release increases when bug fixes, excluding the above cases
 */
typedef struct vpu_versioninfo {
    int fw_major;		/* firmware major version */
    int fw_minor;		/* firmware minor version */
    int fw_release;		/* firmware release version */
    int fw_code;		/* firmware checkin code number */
    int lib_major;		/* library major version */
    int lib_minor;		/* library minor version */
    int lib_release;	/* library release version */
} vpu_versioninfo;

typedef enum {
    MX27 = 0,
    MX51,
    MX53,
    MX61,
    MX63
} SocIndex;

typedef struct {
    int id;
    char *name;
} SocInfo;

static const SocInfo soc_info[] = {
    {0x27, "i.MX27"},
    {0x51, "i.MX51"},
    {0x53, "i.MX53"},
    {0x61, "i.MX6DL"},
    {0x63, "i.MX6Q"}
};

#define VPU_FW_VERSION(major, minor, release)	 \
    (((major) << 12) + ((minor) << 8) + (release))

#define VPU_LIB_VERSION(major, minor, release)	 \
    (((major) << 12) + ((minor) << 8) + (release))

/*
 * Revision History:
 * v5.3.0 [2011.07.30] Add mx6q vpu support
 * v5.2.0 [2011.07.04] Upgrading mx5x f/w to infinite instances support
 * v5.1.5 [2011.06.16] Remove code for not mx5x platforms
 * v5.0.1 [2010.03.03] Integrate mx53 vpu
 * v4.7.1 [2009.09.18] remove share memory file and update SWReset function
 * v4.7.0 [2009.08.03] upgrade mx51 fw to v1.2.0
 * v4.3.2 [2008.10.28] support loopback on MX51
 * v4.2.2 [2008.09.03] support encoder on MX51
 * v4.0.2 [2008.08.21] add the IOClkGateSet() for power saving.
 */
#define VPU_LIB_VERSION_CODE	VPU_LIB_VERSION(5, 4, 28)

//extern unsigned int system_rev;

#define CHIP_REV_1_0            	0x10
#define CHIP_REV_2_0			0x20
#define CHIP_REV_2_1            	0x21

//#define mxc_cpu()               (system_rev >> 12)
//#define mxc_is_cpu(part)        ((mxc_cpu() == (unsigned int)part) ? 1 : 0)
//#define mxc_cpu_rev()           (system_rev & 0xFF)
//#define mxc_cpu_is_rev(rev)     \
//       ((mxc_cpu_rev() == (unsigned int)rev) ? 1 : ((mxc_cpu_rev() < (unsigned int)rev) ? -1 : 2))
//#define MXC_REV(type)                           \
//static inline int type## _rev (int rev)         \
//{                                               \
//        return (type() ? mxc_cpu_is_rev(rev) : 0);      \
//}

//#define cpu_is_mx27()		mxc_is_cpu(soc_info[MX27].id)
//#define cpu_is_mx51()		mxc_is_cpu(soc_info[MX51].id)
//#define cpu_is_mx53()		mxc_is_cpu(soc_info[MX53].id)
//#define cpu_is_mx5x()		(cpu_is_mx51() || cpu_is_mx53())
//#define cpu_is_mx6q()		mxc_is_cpu(soc_info[MX63].id)
//#define cpu_is_mx6dl()		mxc_is_cpu(soc_info[MX61].id)
//#define cpu_is_mx6x()		(cpu_is_mx6q() || cpu_is_mx6dl())

//MXC_REV(cpu_is_mx27);

RetCode vpu_Init(void);
void vpu_UnInit(void);
RetCode vpu_GetVersionInfo(vpu_versioninfo * verinfo);

RetCode vpu_DecOpen(DecHandle *, DecOpenParam *);
RetCode vpu_DecClose(DecHandle);
RetCode vpu_DecSetEscSeqInit(DecHandle handle, int escape);
RetCode vpu_DecGetInitialInfo(DecHandle handle, DecInitialInfo * info);
RetCode vpu_DecRegisterFrameBuffer(DecHandle handle,
        FrameBuffer * bufArray, int num, int stride,
        DecBufInfo * pBufInfo);
RetCode vpu_DecGetBitstreamBuffer(DecHandle handle, PhysicalAddress * paRdPtr,
        PhysicalAddress * paWrPtr, Uint32 * size);
RetCode vpu_DecUpdateBitstreamBuffer(DecHandle handle, Uint32 size);
RetCode vpu_DecStartOneFrame(DecHandle handle, DecParam * param);
RetCode vpu_DecGetOutputInfo(DecHandle handle, DecOutputInfo * info);
RetCode vpu_DecBitBufferFlush(DecHandle handle);
RetCode vpu_DecClrDispFlag(DecHandle handle, int index);
RetCode vpu_DecGiveCommand(DecHandle handle, CodecCommand cmd, void *parameter);

int vpu_IsBusy(void);
int vpu_WaitForInt(int timeout_in_ms);
RetCode vpu_SWReset(DecHandle handle, int index);
int vpu_GetXY2AXIAddr(DecHandle handle, int ycbcr, int posY, int posX, int stride,
        unsigned int addrY, unsigned int addrCb, unsigned int addrCr);

#endif
