/*
 * @file       JetOMX_Basecomponent.h
 * @brief
 * @author
 * @version    0.1.0
 */

#ifndef JETOMX_BASECOMP
#define JETOMX_BASECOMP

#include "JetOMX_Def.h"
#include "OMX_Component.h"
#include "OSAL_Queue.h"
#include "JetOMX_Baseport.h"


typedef struct _EXYNOS_OMX_MESSAGE
{
    OMX_U32 messageType;
    OMX_U32 messageParam;
    OMX_PTR pCmdData;
} EXYNOS_OMX_MESSAGE;

/* for Check TimeStamp after Seek */
typedef struct _EXYNOS_OMX_TIMESTAMP
{
    OMX_BOOL  needSetStartTimeStamp;
    OMX_BOOL  needCheckStartTimeStamp;
    OMX_TICKS startTimeStamp;
    OMX_U32   nStartFlags;
} EXYNOS_OMX_TIMESTAMP;

typedef struct _JETOMX_BASECOMPONENT
{
    OMX_STRING                  componentName;
    OMX_VERSIONTYPE             componentVersion;
    OMX_VERSIONTYPE             specVersion;

    OMX_STATETYPE               currentState;
    EXYNOS_OMX_TRANS_STATETYPE  transientState;
    OMX_BOOL                    abendState;
    OMX_HANDLETYPE              abendStateEvent;

    EXYNOS_CODEC_TYPE           codecType;
    EXYNOS_OMX_PRIORITYMGMTTYPE compPriority;
    OMX_MARKTYPE                propagateMarkType;
    OMX_HANDLETYPE              compMutex;

    OMX_HANDLETYPE              hComponentHandle;

    /* Message Handler */
    OMX_BOOL                    bExitMessageHandlerThread;
    OMX_HANDLETYPE              hMessageHandler;
    OMX_HANDLETYPE              msgSemaphoreHandle;
    OSAL_QUEUE                  messageQ;

    /* Port */
    OMX_PORT_PARAM_TYPE         portParam;
    JETOMX_BASEPORT        *pExynosPort;

    OMX_HANDLETYPE              pauseEvent;

    /* Callback function */
    OMX_CALLBACKTYPE           *pCallbacks;
    OMX_PTR                     callbackData;

    /* Save Timestamp */
    OMX_TICKS                   timeStamp[MAX_TIMESTAMP];
    EXYNOS_OMX_TIMESTAMP        checkTimeStamp;

    /* Save Flags */
    OMX_U32                     nFlags[MAX_FLAGS];

    OMX_BOOL                    getAllDelayBuffer;
    OMX_BOOL                    reInputData;

    OMX_BOOL bUseFlagEOF;
    OMX_BOOL bSaveFlagEOS;

    /* Check for Old & New OMX Process type switch */
    OMX_BOOL bMultiThreadProcess;

    OMX_ERRORTYPE (*exynos_codec_componentInit)(OMX_COMPONENTTYPE *pOMXComponent);
    OMX_ERRORTYPE (*exynos_codec_componentTerminate)(OMX_COMPONENTTYPE *pOMXComponent);

    OMX_ERRORTYPE (*exynos_AllocateTunnelBuffer)(JETOMX_BASEPORT *pOMXBasePort, OMX_U32 nPortIndex);
    OMX_ERRORTYPE (*exynos_FreeTunnelBuffer)(JETOMX_BASEPORT *pOMXBasePort, OMX_U32 nPortIndex);
    OMX_ERRORTYPE (*exynos_BufferProcessCreate)(OMX_COMPONENTTYPE *pOMXComponent);
    OMX_ERRORTYPE (*exynos_BufferProcessTerminate)(OMX_COMPONENTTYPE *pOMXComponent);
    OMX_ERRORTYPE (*exynos_BufferFlush)(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 nPortIndex, OMX_BOOL bEvent);
} JETOMX_BASECOMPONENT;

OMX_ERRORTYPE JetOMX_GetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE  nParamIndex,
    OMX_INOUT OMX_PTR     ComponentParameterStructure);

OMX_ERRORTYPE JetOMX_SetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE  nIndex,
    OMX_IN OMX_PTR        ComponentParameterStructure);

OMX_ERRORTYPE JetOMX_GetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE  nIndex,
    OMX_INOUT OMX_PTR     pComponentConfigStructure);

OMX_ERRORTYPE JetOMX_SetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE  nIndex,
    OMX_IN OMX_PTR        pComponentConfigStructure);

OMX_ERRORTYPE JetOMX_GetExtensionIndex(
    OMX_IN OMX_HANDLETYPE  hComponent,
    OMX_IN OMX_STRING      cParameterName,
    OMX_OUT OMX_INDEXTYPE *pIndexType);

OMX_ERRORTYPE JetOMX_BaseComponent_Constructor(OMX_IN OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE JetOMX_BaseComponent_Destructor(OMX_IN OMX_HANDLETYPE hComponent);

#ifdef __cplusplus
extern "C" {
#endif

    OMX_ERRORTYPE JetOMX_Check_SizeVersion(OMX_PTR header, OMX_U32 size);


#ifdef __cplusplus
};
#endif

#endif
