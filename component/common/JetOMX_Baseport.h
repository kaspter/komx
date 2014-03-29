/*
 * @file       JetOMX_Baseport.h
 * @brief
 * @author
 * @version    0.1.0
 */

#ifndef JetOMX_BASE_PORT
#define JetOMX_BASE_PORT


#include "OMX_Component.h"
#include "OSAL_Queue.h"
#include "JetOMX_Def.h"


#define BUFFER_STATE_ALLOCATED  (1 << 0)
#define BUFFER_STATE_ASSIGNED   (1 << 1)
#define HEADER_STATE_ALLOCATED  (1 << 2)
#define BUFFER_STATE_FREE        0

#define MAX_BUFFER_NUM          40

#define INPUT_PORT_INDEX    0
#define OUTPUT_PORT_INDEX   1
#define ALL_PORT_INDEX     -1
#define ALL_PORT_NUM        2


typedef struct _EXYNOS_OMX_BUFFERHEADERTYPE
{
    OMX_BUFFERHEADERTYPE *OMXBufferHeader;
    OMX_BOOL              bBufferInOMX;
    OMX_HANDLETYPE        ANBHandle;
    void                 *pYUVBuf[MAX_BUFFER_PLANE];
    int                   buf_fd[MAX_BUFFER_PLANE];
} EXYNOS_OMX_BUFFERHEADERTYPE;

typedef struct _EXYNOS_OMX_DATABUFFER
{
    OMX_HANDLETYPE        bufferMutex;
    OMX_BUFFERHEADERTYPE* bufferHeader;
    OMX_BOOL              dataValid;
    OMX_U32               allocSize;
    OMX_U32               dataLen;
    OMX_U32               usedDataLen;
    OMX_U32               remainDataLen;
    OMX_U32               nFlags;
    OMX_TICKS             timeStamp;
    OMX_PTR               pPrivate;
} EXYNOS_OMX_DATABUFFER;

typedef void* CODEC_EXTRA_BUFFERINFO;

typedef struct _EXYNOS_OMX_SINGLEPLANE_DATA
{
    OMX_PTR dataBuffer;
    int fd;
} EXYNOS_OMX_SINGLEPLANE_DATA;

typedef struct _EXYNOS_OMX_MULTIPLANE_DATA
{
    OMX_U32 validPlaneNum;
    OMX_PTR dataBuffer[MAX_BUFFER_PLANE];
    int fd[MAX_BUFFER_PLANE];
} EXYNOS_OMX_MULTIPLANE_DATA;

typedef struct _EXYNOS_OMX_DATA
{
    union {
        EXYNOS_OMX_SINGLEPLANE_DATA singlePlaneBuffer;
        EXYNOS_OMX_MULTIPLANE_DATA multiPlaneBuffer;
    } buffer;
    OMX_U32   allocSize;
    OMX_U32   dataLen;
    OMX_U32   usedDataLen;
    OMX_U32   remainDataLen;
    OMX_U32   nFlags;
    OMX_TICKS timeStamp;
    OMX_PTR   pPrivate;
    CODEC_EXTRA_BUFFERINFO extInfo;

    /* For Share Buffer */
    OMX_BUFFERHEADERTYPE* bufferHeader;
} EXYNOS_OMX_DATA;

typedef struct _EXYNOS_OMX_WAY1_PORT_DATABUFFER
{
    EXYNOS_OMX_DATABUFFER dataBuffer;
} EXYNOS_OMX_PORT_1WAY_DATABUFFER;

typedef struct _EXYNOS_OMX_WAY2_PORT_DATABUFFER
{
    EXYNOS_OMX_DATABUFFER inputDataBuffer;
    EXYNOS_OMX_DATABUFFER outputDataBuffer;
} EXYNOS_OMX_PORT_2WAY_DATABUFFER;

typedef enum _EXYNOS_OMX_PORT_WAY_TYPE
{
    WAY1_PORT = 0x00,
    WAY2_PORT
} EXYNOS_OMX_PORT_WAY_TYPE;

typedef enum _EXYNOS_OMX_EXCEPTION_STATE
{
    GENERAL_STATE = 0x00,
    NEED_PORT_FLUSH,
    NEED_PORT_DISABLE,
} EXYNOS_OMX_EXCEPTION_STATE;

typedef enum _EXYNOS_OMX_PLANE
{
    ONE_PLANE       = 0x01,
    TWO_PLANE       = 0x02,
    THREE_PLANE     = 0x03,
/*
    ANB_START_PLANE = 0x10,
    ANB_ONE_PLANE   = 0x11,
    ANB_TWO_PLANE   = 0x12,
    ANB_THREE_PLANE = 0x13,
*/
} EXYNOS_OMX_PLANE;

typedef struct _JETOMX_BASEPORT
{
    EXYNOS_OMX_BUFFERHEADERTYPE   *extendBufferHeader;
    OMX_U32                       *bufferStateAllocate;
    OMX_PARAM_PORTDEFINITIONTYPE   portDefinition;
    OMX_HANDLETYPE                 bufferSemID;
    OSAL_QUEUE                     bufferQ;
    OMX_U32                        assignedBufferNum;
    OMX_STATETYPE                  portState;
    OMX_HANDLETYPE                 loadedResource;
    OMX_HANDLETYPE                 unloadedResource;

    OMX_BOOL                       bIsPortFlushed;
    OMX_BOOL                       bIsPortDisabled;
    OMX_MARKTYPE                   markType;

    OMX_CONFIG_RECTTYPE            cropRectangle;

    /* Tunnel Info */
    OMX_HANDLETYPE                 tunneledComponent;
    OMX_U32                        tunneledPort;
    OMX_U32                        tunnelBufferNum;
    OMX_BUFFERSUPPLIERTYPE         bufferSupplier;
    OMX_U32                        tunnelFlags;

    OMX_BOOL                       bIsANBEnabled;
    OMX_BOOL                       bStoreMetaData;

    EXYNOS_OMX_BUFFERPROCESS_TYPE  bufferProcessType;
    EXYNOS_OMX_PORT_WAY_TYPE       portWayType;
    OMX_HANDLETYPE                 codecSemID;
    OSAL_QUEUE                     codecBufferQ;

    OMX_HANDLETYPE                 pauseEvent;

    /* Buffer */
    union {
        EXYNOS_OMX_PORT_1WAY_DATABUFFER port1WayDataBuffer;
        EXYNOS_OMX_PORT_2WAY_DATABUFFER port2WayDataBuffer;
    } way;

    /* Data */
    EXYNOS_OMX_DATA                processData;

    /* for flush of Shared buffer scheme */
    OMX_HANDLETYPE                 hAllCodecBufferReturnEvent;
    OMX_HANDLETYPE                 hPortMutex;
    EXYNOS_OMX_EXCEPTION_STATE     exceptionFlag;
} JETOMX_BASEPORT;


#ifdef __cplusplus
extern "C" {
#endif

OMX_ERRORTYPE JetOMX_PortEnableProcess(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 nPortIndex);
OMX_ERRORTYPE JetOMX_PortDisableProcess(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 nPortIndex);
OMX_ERRORTYPE JetOMX_BufferFlushProcess(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 nPortIndex, OMX_BOOL bEvent);
OMX_ERRORTYPE JetOMX_Port_Constructor(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE JetOMX_Port_Destructor(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE Exynos_ResetDataBuffer(EXYNOS_OMX_DATABUFFER *pDataBuffer);
OMX_ERRORTYPE Exynos_ResetCodecData(EXYNOS_OMX_DATA *pData);
OMX_ERRORTYPE Exynos_Shared_BufferToData(EXYNOS_OMX_DATABUFFER *pUseBuffer, EXYNOS_OMX_DATA *pData, EXYNOS_OMX_PLANE nPlane);
OMX_ERRORTYPE Exynos_Shared_DataToBuffer(EXYNOS_OMX_DATA *pData, EXYNOS_OMX_DATABUFFER *pUseBuffer);

#ifdef __cplusplus
};
#endif


#endif
