/*
 * @file       JetOMX_Basecomponent.c
 * @brief
 * @author
 * @version    0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "OSAL_Event.h"
#include "OSAL_Thread.h"
#include "OSAL_ETC.h"
#include "OSAL_Semaphore.h"
#include "OSAL_Mutex.h"
#include "JetOMX_Baseport.h"
#include "JetOMX_Basecomponent.h"
#include "JetOMX_Resourcemanager.h"
#include "JetOMX_Macros.h"

#undef  JETOMX_LOG_TAG
#define JETOMX_LOG_TAG    "JETOMX_BASE_COMP"
#define JETOMX_LOG_OFF
#define JETOMX_TRACE_ON
#include "OSAL_Log.h"


/* Change CHECK_SIZE_VERSION Macro */
OMX_ERRORTYPE JetOMX_Check_SizeVersion(OMX_PTR header, OMX_U32 size)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    OMX_VERSIONTYPE* version = NULL;
    if (header == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    version = (OMX_VERSIONTYPE*)((char*)header + sizeof(OMX_U32));
    if (*((OMX_U32*)header) != size) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (version->s.nVersionMajor != VERSIONMAJOR_NUMBER ||
        version->s.nVersionMinor != VERSIONMINOR_NUMBER) {
        ret = OMX_ErrorVersionMismatch;
        goto EXIT;
    }
    ret = OMX_ErrorNone;
EXIT:
    return ret;
}


/** GetComponentVersion will return information about the component.
    This is a blocking call.  This macro will go directly from the
    application to the component (via a core macro).  The
    component will return from this call within 5 msec.
    @param [in] hComponent
        handle of component to execute the command
    @param [out] pComponentName
        pointer to an empty string of length 128 bytes.  The component
        will write its name into this string.  The name will be
        terminated by a single zero byte.  The name of a component will
        be 127 bytes or less to leave room for the trailing zero byte.
        An example of a valid component name is "OMX.ABC.ChannelMixer\0".
    @param [out] pComponentVersion
        pointer to an OMX Version structure that the component will fill
        in.  The component will fill in a value that indicates the
        component version.  NOTE: the component version is NOT the same
        as the OMX Specification version (found in all structures).  The
        component version is defined by the vendor of the component and
        its value is entirely up to the component vendor.
    @param [out] pSpecVersion
        pointer to an OMX Version structure that the component will fill
        in.  The SpecVersion is the version of the specification that the
        component was built against.  Please note that this value may or
        may not match the structure's version.  For example, if the
        component was built against the 2.0 specification, but the
        application (which creates the structure is built against the
        1.0 specification the versions would be different.
    @param [out] pComponentUUID
        pointer to the UUID of the component which will be filled in by
        the component.  The UUID is a unique identifier that is set at
        RUN time for the component and is unique to each instantion of
        the component.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp
 */

OMX_ERRORTYPE JetOMX_GetComponentVersion(
    OMX_IN  OMX_HANDLETYPE   hComponent,
    OMX_OUT OMX_STRING       pComponentName,
    OMX_OUT OMX_VERSIONTYPE *pComponentVersion,
    OMX_OUT OMX_VERSIONTYPE *pSpecVersion,
    OMX_OUT OMX_UUIDTYPE    *pComponentUUID)
{
    OMX_ERRORTYPE           ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE      *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT   *pJetOMXComponent = NULL;
    OMX_U32                 compUUID[3];

    FunctionIn();

    /* check parameters */
    if (hComponent     == NULL ||
        pComponentName == NULL || pComponentVersion == NULL ||
        pSpecVersion   == NULL || pComponentUUID    == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (pJetOMXComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    OSAL_Strcpy(pComponentName, pJetOMXComponent->componentName);
    OSAL_Memcpy(pComponentVersion, &(pJetOMXComponent->componentVersion), sizeof(OMX_VERSIONTYPE));
    OSAL_Memcpy(pSpecVersion, &(pJetOMXComponent->specVersion), sizeof(OMX_VERSIONTYPE));

    /* Fill UUID with handle address, PID and UID.
     * This should guarantee uiniqness */
    compUUID[0] = (OMX_U32)pOMXComponent;
    compUUID[1] = getpid();
    compUUID[2] = getuid();
    OSAL_Memcpy(*pComponentUUID, compUUID, 3 * sizeof(*compUUID));

    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

/** The OMX_GetState macro will invoke the component to get the current
    state of the component and place the state value into the location
    pointed to by pState.

    The component should return from this call within 5 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the OMX_GetHandle function.
    @param [out] pState
        pointer to the location to receive the state.  The value returned
        is one of the OMX_STATETYPE members
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp
 */
OMX_ERRORTYPE JetOMX_GetState (
    OMX_IN OMX_HANDLETYPE  hComponent,
    OMX_OUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;

    FunctionIn();

    if (hComponent == NULL || pState == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    *pState = pJetOMXComponent->currentState;
    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_ComponentStateSet(OMX_COMPONENTTYPE *pOMXComponent, OMX_U32 messageParam)
{
    OMX_ERRORTYPE          ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT  *pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    EXYNOS_OMX_MESSAGE    *message;
    OMX_STATETYPE          destState = messageParam;
    OMX_STATETYPE          currentState = pJetOMXComponent->currentState;
    JETOMX_BASEPORT       *pExynosPort = NULL;
    OMX_S32                countValue = 0;
    unsigned int           i = 0, j = 0;
    int                    k = 0;

    FunctionIn();

    /* check parameters */
    if (currentState == destState) {
         ret = OMX_ErrorSameState;
         goto EXIT;
    }
    if (currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    if ((currentState == OMX_StateLoaded) && (destState == OMX_StateIdle)) {
        ret = JetOMX_Get_Resource(pOMXComponent);
        if (ret != OMX_ErrorNone) {
            goto EXIT;
        }
    }
    if (((currentState == OMX_StateIdle) && (destState == OMX_StateLoaded))       ||
        ((currentState == OMX_StateIdle) && (destState == OMX_StateInvalid))      ||
        ((currentState == OMX_StateExecuting) && (destState == OMX_StateInvalid)) ||
        ((currentState == OMX_StatePause) && (destState == OMX_StateInvalid))) {
        JetOMX_Release_Resource(pOMXComponent);
    }

    OSAL_Log(JETOMX_LOG_TRACE, "destState: %d", destState);
    switch (destState) {
    case OMX_StateInvalid:
        switch (currentState) {
        case OMX_StateWaitForResources:
            JetOMX_Out_WaitForResource(pOMXComponent);
        case OMX_StateIdle:
        case OMX_StateExecuting:
        case OMX_StatePause:
        case OMX_StateLoaded:
            pJetOMXComponent->currentState = OMX_StateInvalid;
            ret = pJetOMXComponent->exynos_BufferProcessTerminate(pOMXComponent);

            for (i = 0; i < ALL_PORT_NUM; i++) {
                if (pJetOMXComponent->pExynosPort[i].portWayType == WAY1_PORT) {
                    OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port1WayDataBuffer.dataBuffer.bufferMutex);
                    pJetOMXComponent->pExynosPort[i].way.port1WayDataBuffer.dataBuffer.bufferMutex = NULL;
                } else if (pJetOMXComponent->pExynosPort[i].portWayType == WAY2_PORT) {
                    OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.inputDataBuffer.bufferMutex);
                    pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.inputDataBuffer.bufferMutex = NULL;
                    OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.outputDataBuffer.bufferMutex);
                    pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.outputDataBuffer.bufferMutex = NULL;
                }
                OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].hPortMutex);
                pJetOMXComponent->pExynosPort[i].hPortMutex = NULL;
            }

            if (pJetOMXComponent->bMultiThreadProcess == OMX_FALSE) {
                OSAL_SignalTerminate(pJetOMXComponent->pauseEvent);
                pJetOMXComponent->pauseEvent = NULL;
            } else {
                for (i = 0; i < ALL_PORT_NUM; i++) {
                    OSAL_SignalTerminate(pJetOMXComponent->pExynosPort[i].pauseEvent);
                    pJetOMXComponent->pExynosPort[i].pauseEvent = NULL;
                    if (pJetOMXComponent->pExynosPort[i].bufferProcessType == BUFFER_SHARE) {
                        OSAL_SignalTerminate(&pJetOMXComponent->pExynosPort[i].hAllCodecBufferReturnEvent);
                        pJetOMXComponent->pExynosPort[i].hAllCodecBufferReturnEvent = NULL;
                    }
                }
            }
            for (i = 0; i < ALL_PORT_NUM; i++) {
                OSAL_SemaphoreTerminate(pJetOMXComponent->pExynosPort[i].bufferSemID);
                pJetOMXComponent->pExynosPort[i].bufferSemID = NULL;
            }
            if (pJetOMXComponent->exynos_codec_componentTerminate != NULL)
                pJetOMXComponent->exynos_codec_componentTerminate(pOMXComponent);

            ret = OMX_ErrorInvalidState;
            break;
        default:
            ret = OMX_ErrorInvalidState;
            break;
        }
        break;
    case OMX_StateLoaded:
        switch (currentState) {
        case OMX_StateIdle:
            ret = pJetOMXComponent->exynos_BufferProcessTerminate(pOMXComponent);

            for (i = 0; i < ALL_PORT_NUM; i++) {
                if (pJetOMXComponent->pExynosPort[i].portWayType == WAY1_PORT) {
                    OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port1WayDataBuffer.dataBuffer.bufferMutex);
                    pJetOMXComponent->pExynosPort[i].way.port1WayDataBuffer.dataBuffer.bufferMutex = NULL;
                } else if (pJetOMXComponent->pExynosPort[i].portWayType == WAY2_PORT) {
                    OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.inputDataBuffer.bufferMutex);
                    pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.inputDataBuffer.bufferMutex = NULL;
                    OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.outputDataBuffer.bufferMutex);
                    pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.outputDataBuffer.bufferMutex = NULL;
                }
                OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].hPortMutex);
                pJetOMXComponent->pExynosPort[i].hPortMutex = NULL;
            }
            if (pJetOMXComponent->bMultiThreadProcess == OMX_FALSE) {
                OSAL_SignalTerminate(pJetOMXComponent->pauseEvent);
                pJetOMXComponent->pauseEvent = NULL;
            } else {
                for (i = 0; i < ALL_PORT_NUM; i++) {
                    OSAL_SignalTerminate(pJetOMXComponent->pExynosPort[i].pauseEvent);
                    pJetOMXComponent->pExynosPort[i].pauseEvent = NULL;
                    if (pJetOMXComponent->pExynosPort[i].bufferProcessType == BUFFER_SHARE) {
                        OSAL_SignalTerminate(&pJetOMXComponent->pExynosPort[i].hAllCodecBufferReturnEvent);
                        pJetOMXComponent->pExynosPort[i].hAllCodecBufferReturnEvent = NULL;
                    }
                }
            }
            for (i = 0; i < ALL_PORT_NUM; i++) {
                OSAL_SemaphoreTerminate(pJetOMXComponent->pExynosPort[i].bufferSemID);
                pJetOMXComponent->pExynosPort[i].bufferSemID = NULL;
            }

            pJetOMXComponent->exynos_codec_componentTerminate(pOMXComponent);

            for (i = 0; i < (pJetOMXComponent->portParam.nPorts); i++) {
                pExynosPort = (pJetOMXComponent->pExynosPort + i);
                if (CHECK_PORT_TUNNELED(pExynosPort) && CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
                    while (OSAL_GetElemNum(&pExynosPort->bufferQ) > 0) {
                        message = (EXYNOS_OMX_MESSAGE*)OSAL_Dequeue(&pExynosPort->bufferQ);
                        if (message != NULL)
                            OSAL_Free(message);
                    }
                    ret = pJetOMXComponent->exynos_FreeTunnelBuffer(pExynosPort, i);
                    if (OMX_ErrorNone != ret) {
                        goto EXIT;
                    }
                } else {
                    if (CHECK_PORT_ENABLED(pExynosPort)) {
                        OSAL_SemaphoreWait(pExynosPort->unloadedResource);
                        pExynosPort->portDefinition.bPopulated = OMX_FALSE;
                    }
                }
            }
            pJetOMXComponent->currentState = OMX_StateLoaded;
            break;
        case OMX_StateWaitForResources:
            ret = JetOMX_Out_WaitForResource(pOMXComponent);
            pJetOMXComponent->currentState = OMX_StateLoaded;
            break;
        case OMX_StateExecuting:
        case OMX_StatePause:
        default:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        }
        break;
    case OMX_StateIdle:
        switch (currentState) {
        case OMX_StateLoaded:
            for (i = 0; i < pJetOMXComponent->portParam.nPorts; i++) {
                pExynosPort = (pJetOMXComponent->pExynosPort + i);
                if (pExynosPort == NULL) {
                    ret = OMX_ErrorBadParameter;
                    goto EXIT;
                }
                if (CHECK_PORT_TUNNELED(pExynosPort) && CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
                    if (CHECK_PORT_ENABLED(pExynosPort)) {
                        ret = pJetOMXComponent->exynos_AllocateTunnelBuffer(pExynosPort, i);
                        if (ret!=OMX_ErrorNone)
                            goto EXIT;
                    }
                } else {
                    if (CHECK_PORT_ENABLED(pExynosPort)) {
                        OSAL_SemaphoreWait(pJetOMXComponent->pExynosPort[i].loadedResource);
                        if (pJetOMXComponent->abendState == OMX_TRUE) {
                            OSAL_SignalSet(pJetOMXComponent->abendStateEvent);
                            ret = JetOMX_Release_Resource(pOMXComponent);
                            goto EXIT;
                        }
                        pExynosPort->portDefinition.bPopulated = OMX_TRUE;
                    }
                }
            }
            ret = pJetOMXComponent->exynos_codec_componentInit(pOMXComponent);
            if (ret != OMX_ErrorNone) {
                /*
                 * if (CHECK_PORT_TUNNELED == OMX_TRUE) thenTunnel Buffer Free
                 */
                goto EXIT;
            }
            if (pJetOMXComponent->bMultiThreadProcess == OMX_FALSE) {
                OSAL_SignalCreate(&pJetOMXComponent->pauseEvent);
            } else {
                for (i = 0; i < ALL_PORT_NUM; i++) {
                    OSAL_SignalCreate(&pJetOMXComponent->pExynosPort[i].pauseEvent);
                    if (pJetOMXComponent->pExynosPort[i].bufferProcessType == BUFFER_SHARE)
                        OSAL_SignalCreate(&pJetOMXComponent->pExynosPort[i].hAllCodecBufferReturnEvent);
                }
            }
            for (i = 0; i < ALL_PORT_NUM; i++) {
                ret = OSAL_SemaphoreCreate(&pJetOMXComponent->pExynosPort[i].bufferSemID);
                if (ret != OMX_ErrorNone) {
                    ret = OMX_ErrorInsufficientResources;
                    OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
                    goto EXIT;
                }
            }
            for (i = 0; i < ALL_PORT_NUM; i++) {
                if (pJetOMXComponent->pExynosPort[i].portWayType == WAY1_PORT) {
                    ret = OSAL_MutexCreate(&pJetOMXComponent->pExynosPort[i].way.port1WayDataBuffer.dataBuffer.bufferMutex);
                    if (ret != OMX_ErrorNone) {
                        ret = OMX_ErrorInsufficientResources;
                        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
                        goto EXIT;
                    }
                } else if (pJetOMXComponent->pExynosPort[i].portWayType == WAY2_PORT) {
                    ret = OSAL_MutexCreate(&pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.inputDataBuffer.bufferMutex);
                    if (ret != OMX_ErrorNone) {
                        ret = OMX_ErrorInsufficientResources;
                        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
                        goto EXIT;
                    }
                    ret = OSAL_MutexCreate(&pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.outputDataBuffer.bufferMutex);
                    if (ret != OMX_ErrorNone) {
                        ret = OMX_ErrorInsufficientResources;
                        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
                        goto EXIT;
                    }
                }
                ret = OSAL_MutexCreate(&pJetOMXComponent->pExynosPort[i].hPortMutex);
                if (ret != OMX_ErrorNone) {
                    ret = OMX_ErrorInsufficientResources;
                    goto EXIT;
                }
            }

            ret = pJetOMXComponent->exynos_BufferProcessCreate(pOMXComponent);
            if (ret != OMX_ErrorNone) {
                /*
                 * if (CHECK_PORT_TUNNELED == OMX_TRUE) thenTunnel Buffer Free
                 */
                if (pJetOMXComponent->bMultiThreadProcess == OMX_FALSE) {
                    OSAL_SignalTerminate(pJetOMXComponent->pauseEvent);
                    pJetOMXComponent->pauseEvent = NULL;
                } else {
                    for (i = 0; i < ALL_PORT_NUM; i++) {
                        OSAL_SignalTerminate(pJetOMXComponent->pExynosPort[i].pauseEvent);
                        pJetOMXComponent->pExynosPort[i].pauseEvent = NULL;
                        if (pJetOMXComponent->pExynosPort[i].bufferProcessType == BUFFER_SHARE) {
                            OSAL_SignalTerminate(&pJetOMXComponent->pExynosPort[i].hAllCodecBufferReturnEvent);
                            pJetOMXComponent->pExynosPort[i].hAllCodecBufferReturnEvent = NULL;
                        }
                    }
                }
                for (i = 0; i < ALL_PORT_NUM; i++) {
                    if (pJetOMXComponent->pExynosPort[i].portWayType == WAY1_PORT) {
                        OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port1WayDataBuffer.dataBuffer.bufferMutex);
                        pJetOMXComponent->pExynosPort[i].way.port1WayDataBuffer.dataBuffer.bufferMutex = NULL;
                    } else if (pJetOMXComponent->pExynosPort[i].portWayType == WAY2_PORT) {
                        OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.inputDataBuffer.bufferMutex);
                        pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.inputDataBuffer.bufferMutex = NULL;
                        OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.outputDataBuffer.bufferMutex);
                        pJetOMXComponent->pExynosPort[i].way.port2WayDataBuffer.outputDataBuffer.bufferMutex = NULL;
                    }
                    OSAL_MutexTerminate(pJetOMXComponent->pExynosPort[i].hPortMutex);
                    pJetOMXComponent->pExynosPort[i].hPortMutex = NULL;
                }
                for (i = 0; i < ALL_PORT_NUM; i++) {
                    OSAL_SemaphoreTerminate(pJetOMXComponent->pExynosPort[i].bufferSemID);
                    pJetOMXComponent->pExynosPort[i].bufferSemID = NULL;
                }

                ret = OMX_ErrorInsufficientResources;
                goto EXIT;
            }
            pJetOMXComponent->currentState = OMX_StateIdle;
            break;
        case OMX_StateExecuting:
        case OMX_StatePause:
            JetOMX_BufferFlushProcess(pOMXComponent, ALL_PORT_INDEX, OMX_FALSE);
            pJetOMXComponent->currentState = OMX_StateIdle;
            break;
        case OMX_StateWaitForResources:
            pJetOMXComponent->currentState = OMX_StateIdle;
            break;
        default:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        }
        break;
    case OMX_StateExecuting:
        switch (currentState) {
        case OMX_StateLoaded:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        case OMX_StateIdle:
            for (i = 0; i < pJetOMXComponent->portParam.nPorts; i++) {
                pExynosPort = &pJetOMXComponent->pExynosPort[i];
                if (CHECK_PORT_TUNNELED(pExynosPort) && CHECK_PORT_BUFFER_SUPPLIER(pExynosPort) && CHECK_PORT_ENABLED(pExynosPort)) {
                    for (j = 0; j < pExynosPort->tunnelBufferNum; j++) {
                        OSAL_SemaphorePost(pJetOMXComponent->pExynosPort[i].bufferSemID);
                    }
                }
            }

            pJetOMXComponent->transientState = EXYNOS_OMX_TransStateMax;
            pJetOMXComponent->currentState = OMX_StateExecuting;
            if (pJetOMXComponent->bMultiThreadProcess == OMX_FALSE) {
                OSAL_SignalSet(pJetOMXComponent->pauseEvent);
            } else {
                for (i = 0; i < ALL_PORT_NUM; i++) {
                    OSAL_SignalSet(pJetOMXComponent->pExynosPort[i].pauseEvent);
                }
            }
            break;
        case OMX_StatePause:
            for (i = 0; i < pJetOMXComponent->portParam.nPorts; i++) {
                pExynosPort = &pJetOMXComponent->pExynosPort[i];
                if (CHECK_PORT_TUNNELED(pExynosPort) && CHECK_PORT_BUFFER_SUPPLIER(pExynosPort) && CHECK_PORT_ENABLED(pExynosPort)) {
                    OMX_S32 semaValue = 0, cnt = 0;
                    OSAL_Get_SemaphoreCount(pJetOMXComponent->pExynosPort[i].bufferSemID, &semaValue);
                    if (OSAL_GetElemNum(&pExynosPort->bufferQ) > semaValue) {
                        cnt = OSAL_GetElemNum(&pExynosPort->bufferQ) - semaValue;
                        for (k = 0; k < cnt; k++) {
                            OSAL_SemaphorePost(pJetOMXComponent->pExynosPort[i].bufferSemID);
                        }
                    }
                }
            }

            pJetOMXComponent->currentState = OMX_StateExecuting;
            if (pJetOMXComponent->bMultiThreadProcess == OMX_FALSE) {
                OSAL_SignalSet(pJetOMXComponent->pauseEvent);
            } else {
                for (i = 0; i < ALL_PORT_NUM; i++) {
                    OSAL_SignalSet(pJetOMXComponent->pExynosPort[i].pauseEvent);
                }
            }
            break;
        case OMX_StateWaitForResources:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        default:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        }
        break;
    case OMX_StatePause:
        switch (currentState) {
        case OMX_StateLoaded:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        case OMX_StateIdle:
            pJetOMXComponent->currentState = OMX_StatePause;
            break;
        case OMX_StateExecuting:
            pJetOMXComponent->currentState = OMX_StatePause;
            break;
        case OMX_StateWaitForResources:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        default:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        }
        break;
    case OMX_StateWaitForResources:
        switch (currentState) {
        case OMX_StateLoaded:
            ret = JetOMX_In_WaitForResource(pOMXComponent);
            pJetOMXComponent->currentState = OMX_StateWaitForResources;
            break;
        case OMX_StateIdle:
        case OMX_StateExecuting:
        case OMX_StatePause:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        default:
            ret = OMX_ErrorIncorrectStateTransition;
            break;
        }
        break;
    default:
        ret = OMX_ErrorIncorrectStateTransition;
        break;
    }

EXIT:
    if (ret == OMX_ErrorNone) {
        if (pJetOMXComponent->pCallbacks != NULL) {
            pJetOMXComponent->pCallbacks->EventHandler((OMX_HANDLETYPE)pOMXComponent,
            pJetOMXComponent->callbackData,
            OMX_EventCmdComplete, OMX_CommandStateSet,
            destState, NULL);
        }
    } else {
        OSAL_Log(JETOMX_LOG_ERROR, "%s:%d", __FUNCTION__, __LINE__);
        if (pJetOMXComponent->pCallbacks != NULL) {
            pJetOMXComponent->pCallbacks->EventHandler((OMX_HANDLETYPE)pOMXComponent,
            pJetOMXComponent->callbackData,
            OMX_EventError, ret, 0, NULL);
        }
    }
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE JetOMX_MessageHandlerThread(OMX_PTR threadData)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;
    EXYNOS_OMX_MESSAGE       *message = NULL;
    OMX_U32                   messageType = 0, portIndex = 0;

    FunctionIn();

    if (threadData == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    pOMXComponent = (OMX_COMPONENTTYPE *)threadData;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    while (pJetOMXComponent->bExitMessageHandlerThread == OMX_FALSE) {
        OSAL_SemaphoreWait(pJetOMXComponent->msgSemaphoreHandle);
        message = (EXYNOS_OMX_MESSAGE *)OSAL_Dequeue(&pJetOMXComponent->messageQ);
        if (message != NULL) {
            messageType = message->messageType;
            switch (messageType) {
            case OMX_CommandStateSet:
                ret = JetOMX_ComponentStateSet(pOMXComponent, message->messageParam);
                break;
            case OMX_CommandFlush:
                ret = JetOMX_BufferFlushProcess(pOMXComponent, message->messageParam, OMX_TRUE);
                break;
            case OMX_CommandPortDisable:
                ret = JetOMX_PortDisableProcess(pOMXComponent, message->messageParam);
                break;
            case OMX_CommandPortEnable:
                ret = JetOMX_PortEnableProcess(pOMXComponent, message->messageParam);
                break;
            case OMX_CommandMarkBuffer:
                portIndex = message->messageParam;
                pJetOMXComponent->pExynosPort[portIndex].markType.hMarkTargetComponent = ((OMX_MARKTYPE *)message->pCmdData)->hMarkTargetComponent;
                pJetOMXComponent->pExynosPort[portIndex].markType.pMarkData            = ((OMX_MARKTYPE *)message->pCmdData)->pMarkData;
                break;
            case (OMX_COMMANDTYPE)EXYNOS_OMX_CommandComponentDeInit:
                pJetOMXComponent->bExitMessageHandlerThread = OMX_TRUE;
                break;
            default:
                break;
            }
            OSAL_Free(message);
            message = NULL;
        }
    }

    OSAL_ThreadExit(NULL);

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE Exynos_StateSet(JETOMX_BASECOMPONENT *pJetOMXComponent, OMX_U32 nParam)
{
    OMX_U32 destState = nParam;
    OMX_U32 i = 0;

    if ((destState == OMX_StateIdle) && (pJetOMXComponent->currentState == OMX_StateLoaded)) {
        pJetOMXComponent->transientState = EXYNOS_OMX_TransStateLoadedToIdle;
        for(i = 0; i < pJetOMXComponent->portParam.nPorts; i++) {
            pJetOMXComponent->pExynosPort[i].portState = OMX_StateIdle;
        }
        OSAL_Log(JETOMX_LOG_TRACE, "to OMX_StateIdle");
    } else if ((destState == OMX_StateLoaded) && (pJetOMXComponent->currentState == OMX_StateIdle)) {
        pJetOMXComponent->transientState = EXYNOS_OMX_TransStateIdleToLoaded;
        for (i = 0; i < pJetOMXComponent->portParam.nPorts; i++) {
            pJetOMXComponent->pExynosPort[i].portState = OMX_StateLoaded;
        }
        OSAL_Log(JETOMX_LOG_TRACE, "to OMX_StateLoaded");
    } else if ((destState == OMX_StateIdle) && (pJetOMXComponent->currentState == OMX_StateExecuting)) {
        pJetOMXComponent->transientState = EXYNOS_OMX_TransStateExecutingToIdle;
        OSAL_Log(JETOMX_LOG_TRACE, "to OMX_StateIdle");
    } else if ((destState == OMX_StateExecuting) && (pJetOMXComponent->currentState == OMX_StateIdle)) {
        pJetOMXComponent->transientState = EXYNOS_OMX_TransStateIdleToExecuting;
        OSAL_Log(JETOMX_LOG_TRACE, "to OMX_StateExecuting");
    } else if (destState == OMX_StateInvalid) {
        for (i = 0; i < pJetOMXComponent->portParam.nPorts; i++) {
            pJetOMXComponent->pExynosPort[i].portState = OMX_StateInvalid;
        }
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE Exynos_SetPortFlush(JETOMX_BASECOMPONENT *pJetOMXComponent, OMX_U32 nParam)
{
    OMX_ERRORTYPE        ret = OMX_ErrorNone;
    JETOMX_BASEPORT     *pExynosPort = NULL;
    OMX_S32              portIndex = nParam;
    OMX_U16              i = 0, cnt = 0, index = 0;


    if ((pJetOMXComponent->currentState == OMX_StateExecuting) ||
        (pJetOMXComponent->currentState == OMX_StatePause)) {
        if ((portIndex != ALL_PORT_INDEX) &&
           ((OMX_S32)portIndex >= (OMX_S32)pJetOMXComponent->portParam.nPorts)) {
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }

        /*********************
        *    need flush event set ?????
        **********************/
        cnt = (portIndex == ALL_PORT_INDEX ) ? ALL_PORT_NUM : 1;
        for (i = 0; i < cnt; i++) {
            if (portIndex == ALL_PORT_INDEX)
                index = i;
            else
                index = portIndex;
            pJetOMXComponent->pExynosPort[index].bIsPortFlushed = OMX_TRUE;
        }
    } else {
        ret = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }
    ret = OMX_ErrorNone;

EXIT:
    return ret;
}

static OMX_ERRORTYPE Exynos_SetPortEnable(JETOMX_BASECOMPONENT *pJetOMXComponent, OMX_U32 nParam)
{
    OMX_ERRORTYPE        ret = OMX_ErrorNone;
    JETOMX_BASEPORT     *pExynosPort = NULL;
    OMX_S32              portIndex = nParam;
    OMX_U16              i = 0, cnt = 0;

    FunctionIn();

    if ((portIndex != ALL_PORT_INDEX) &&
        ((OMX_S32)portIndex >= (OMX_S32)pJetOMXComponent->portParam.nPorts)) {
        ret = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

    if (portIndex == ALL_PORT_INDEX) {
        for (i = 0; i < pJetOMXComponent->portParam.nPorts; i++) {
            pExynosPort = &pJetOMXComponent->pExynosPort[i];
            if (CHECK_PORT_ENABLED(pExynosPort)) {
                ret = OMX_ErrorIncorrectStateOperation;
                goto EXIT;
            } else {
                pExynosPort->portState = OMX_StateIdle;
            }
        }
    } else {
        pExynosPort = &pJetOMXComponent->pExynosPort[portIndex];
        if (CHECK_PORT_ENABLED(pExynosPort)) {
            ret = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        } else {
            pExynosPort->portState = OMX_StateIdle;
        }
    }
    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;

}

static OMX_ERRORTYPE Exynos_SetPortDisable(JETOMX_BASECOMPONENT *pJetOMXComponent, OMX_U32 nParam)
{
    OMX_ERRORTYPE        ret = OMX_ErrorNone;
    JETOMX_BASEPORT     *pExynosPort = NULL;
    OMX_S32              portIndex = nParam;
    OMX_U16              i = 0, cnt = 0;

    FunctionIn();

    if ((portIndex != ALL_PORT_INDEX) &&
        ((OMX_S32)portIndex >= (OMX_S32)pJetOMXComponent->portParam.nPorts)) {
        ret = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

    if (portIndex == ALL_PORT_INDEX) {
        for (i = 0; i < pJetOMXComponent->portParam.nPorts; i++) {
            pExynosPort = &pJetOMXComponent->pExynosPort[i];
            if (!CHECK_PORT_ENABLED(pExynosPort)) {
                ret = OMX_ErrorIncorrectStateOperation;
                goto EXIT;
            }
            pExynosPort->portState = OMX_StateLoaded;
            pExynosPort->bIsPortDisabled = OMX_TRUE;
        }
    } else {
        pExynosPort = &pJetOMXComponent->pExynosPort[portIndex];
        pExynosPort->portState = OMX_StateLoaded;
        pExynosPort->bIsPortDisabled = OMX_TRUE;
    }
    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

static OMX_ERRORTYPE Exynos_SetMarkBuffer(JETOMX_BASECOMPONENT *pJetOMXComponent, OMX_U32 nParam)
{
    OMX_ERRORTYPE        ret = OMX_ErrorNone;
    JETOMX_BASEPORT     *pExynosPort = NULL;
    OMX_U32              portIndex = nParam;
    OMX_U16              i = 0, cnt = 0;


    if (nParam >= pJetOMXComponent->portParam.nPorts) {
        ret = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

    if ((pJetOMXComponent->currentState == OMX_StateExecuting) ||
        (pJetOMXComponent->currentState == OMX_StatePause)) {
        ret = OMX_ErrorNone;
    } else {
        ret = OMX_ErrorIncorrectStateOperation;
    }

EXIT:
    return ret;
}

static OMX_ERRORTYPE JetOMX_CommandQueue(
    JETOMX_BASECOMPONENT  *pJetOMXComponent,
    OMX_COMMANDTYPE        Cmd,
    OMX_U32                nParam,
    OMX_PTR                pCmdData)
{
    OMX_ERRORTYPE    ret = OMX_ErrorNone;
    EXYNOS_OMX_MESSAGE *command = (EXYNOS_OMX_MESSAGE *)OSAL_Malloc(sizeof(EXYNOS_OMX_MESSAGE));

    if (command == NULL) {
        ret = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    command->messageType  = (OMX_U32)Cmd;
    command->messageParam = nParam;
    command->pCmdData     = pCmdData;

    ret = OSAL_Queue(&pJetOMXComponent->messageQ, (void *)command);
    if (ret != 0) {
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }
    ret = OSAL_SemaphorePost(pJetOMXComponent->msgSemaphoreHandle);

EXIT:
    return ret;
}

/** Send a command to the component.  This call is a non-blocking call.
    The component should check the parameters and then queue the command
    to the component thread to be executed.  The component thread shall
    send the EventHandler() callback at the conclusion of the command.
    This macro will go directly from the application to the component (via
    a core macro).  The component will return from this call within 5 msec.

    When the command is "OMX_CommandStateSet" the component will queue a
    state transition to the new state idenfied in nParam.

    When the command is "OMX_CommandFlush", to flush a port's buffer queues,
    the command will force the component to return all buffers NOT CURRENTLY
    BEING PROCESSED to the application, in the order in which the buffers
    were received.

    When the command is "OMX_CommandPortDisable" or
    "OMX_CommandPortEnable", the component's port (given by the value of
    nParam) will be stopped or restarted.

    When the command "OMX_CommandMarkBuffer" is used to mark a buffer, the
    pCmdData will point to a OMX_MARKTYPE structure containing the component
    handle of the component to examine the buffer chain for the mark.  nParam
    contains the index of the port on which the buffer mark is applied.

    Specification text for more details.

    @param [in] hComponent
        handle of component to execute the command
    @param [in] Cmd
        Command for the component to execute
    @param [in] nParam
        Parameter for the command to be executed.  When Cmd has the value
        OMX_CommandStateSet, value is a member of OMX_STATETYPE.  When Cmd has
        the value OMX_CommandFlush, value of nParam indicates which port(s)
        to flush. -1 is used to flush all ports a single port index will
        only flush that port.  When Cmd has the value "OMX_CommandPortDisable"
        or "OMX_CommandPortEnable", the component's port is given by
        the value of nParam.  When Cmd has the value "OMX_CommandMarkBuffer"
        the components pot is given by the value of nParam.
    @param [in] pCmdData
        Parameter pointing to the OMX_MARKTYPE structure when Cmd has the value
        "OMX_CommandMarkBuffer".
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp
 */
OMX_ERRORTYPE JetOMX_SendCommand(
    OMX_IN OMX_HANDLETYPE  hComponent,
    OMX_IN OMX_COMMANDTYPE Cmd,
    OMX_IN OMX_U32         nParam,
    OMX_IN OMX_PTR         pCmdData)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;
    EXYNOS_OMX_MESSAGE       *message = NULL;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (pJetOMXComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (Cmd) {
    case OMX_CommandStateSet :
        OSAL_Log(JETOMX_LOG_TRACE, "Command: OMX_CommandStateSet");
        Exynos_StateSet(pJetOMXComponent, nParam);
        break;
    case OMX_CommandFlush :
        OSAL_Log(JETOMX_LOG_TRACE, "Command: OMX_CommandFlush");
        ret = Exynos_SetPortFlush(pJetOMXComponent, nParam);
        if (ret != OMX_ErrorNone)
            goto EXIT;
        break;
    case OMX_CommandPortDisable :
        OSAL_Log(JETOMX_LOG_TRACE, "Command: OMX_CommandPortDisable");
        ret = Exynos_SetPortDisable(pJetOMXComponent, nParam);
        if (ret != OMX_ErrorNone)
            goto EXIT;
        break;
    case OMX_CommandPortEnable :
        OSAL_Log(JETOMX_LOG_TRACE, "Command: OMX_CommandPortEnable");
        ret = Exynos_SetPortEnable(pJetOMXComponent, nParam);
        if (ret != OMX_ErrorNone)
            goto EXIT;
        break;
    case OMX_CommandMarkBuffer :
        OSAL_Log(JETOMX_LOG_TRACE, "Command: OMX_CommandMarkBuffer");
        ret = Exynos_SetMarkBuffer(pJetOMXComponent, nParam);
        if (ret != OMX_ErrorNone)
            goto EXIT;
        break;
    default:
        break;
    }

    ret = JetOMX_CommandQueue(pJetOMXComponent, Cmd, nParam, pCmdData);

EXIT:
    FunctionOut();

    return ret;
}

/** The OMX_GetParameter macro will get one of the current parameter
    settings from the component.  This macro cannot only be invoked when
    the component is in the OMX_StateInvalid state.  The nParamIndex
    parameter is used to indicate which structure is being requested from
    the component.  The application shall allocate the correct structure
    and shall fill in the structure size and version information before
    invoking this macro.  When the parameter applies to a port, the
    caller shall fill in the appropriate nPortIndex value indicating the
    port on which the parameter applies. If the component has not had
    any settings changed, then the component should return a set of
    valid DEFAULT  parameters for the component.  This is a blocking
    call.

    The component should return from this call within 20 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the OMX_GetHandle function.
    @param [in] nParamIndex
        Index of the structure to be filled.  This value is from the
        OMX_INDEXTYPE enumeration.
    @param [in,out] pComponentParameterStructure
        Pointer to application allocated structure to be filled by the
        component.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp
 */

OMX_ERRORTYPE JetOMX_GetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE  nParamIndex,
    OMX_INOUT OMX_PTR     ComponentParameterStructure)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (ComponentParameterStructure == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pJetOMXComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (nParamIndex) {
    case OMX_IndexParamAudioInit:
    case OMX_IndexParamVideoInit:
    case OMX_IndexParamImageInit:
    case OMX_IndexParamOtherInit:
    {
        OMX_PORT_PARAM_TYPE *portParam = (OMX_PORT_PARAM_TYPE *)ComponentParameterStructure;
        ret = JetOMX_Check_SizeVersion(portParam, sizeof(OMX_PORT_PARAM_TYPE));
        if (ret != OMX_ErrorNone) {
            goto EXIT;
        }
        portParam->nPorts         = 0;
        portParam->nStartPortNumber     = 0;
    }
        break;
    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *portDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_U32                       portIndex = portDefinition->nPortIndex;
        JETOMX_BASEPORT          *pExynosPort;

        if (portIndex >= pJetOMXComponent->portParam.nPorts) {
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        ret = JetOMX_Check_SizeVersion(portDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
        if (ret != OMX_ErrorNone) {
            goto EXIT;
        }

        pExynosPort = &pJetOMXComponent->pExynosPort[portIndex];
        OSAL_Memcpy(portDefinition, &pExynosPort->portDefinition, portDefinition->nSize);
    }
        break;
    case OMX_IndexParamPriorityMgmt:
    {
        OMX_PRIORITYMGMTTYPE *compPriority = (OMX_PRIORITYMGMTTYPE *)ComponentParameterStructure;

        ret = JetOMX_Check_SizeVersion(compPriority, sizeof(OMX_PRIORITYMGMTTYPE));
        if (ret != OMX_ErrorNone) {
            goto EXIT;
        }

        compPriority->nGroupID       = pJetOMXComponent->compPriority.nGroupID;
        compPriority->nGroupPriority = pJetOMXComponent->compPriority.nGroupPriority;
    }
        break;

    case OMX_IndexParamCompBufferSupplier:
    {
        OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        OMX_U32                       portIndex = bufferSupplier->nPortIndex;
        JETOMX_BASEPORT          *pExynosPort;

        if ((pJetOMXComponent->currentState == OMX_StateLoaded) ||
            (pJetOMXComponent->currentState == OMX_StateWaitForResources)) {
            if (portIndex >= pJetOMXComponent->portParam.nPorts) {
                ret = OMX_ErrorBadPortIndex;
                goto EXIT;
            }
            ret = JetOMX_Check_SizeVersion(bufferSupplier, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
            if (ret != OMX_ErrorNone) {
                goto EXIT;
            }

            pExynosPort = &pJetOMXComponent->pExynosPort[portIndex];


            if (pExynosPort->portDefinition.eDir == OMX_DirInput) {
                if (CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
                    bufferSupplier->eBufferSupplier = OMX_BufferSupplyInput;
                } else if (CHECK_PORT_TUNNELED(pExynosPort)) {
                    bufferSupplier->eBufferSupplier = OMX_BufferSupplyOutput;
                } else {
                    bufferSupplier->eBufferSupplier = OMX_BufferSupplyUnspecified;
                }
            } else {
                if (CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
                    bufferSupplier->eBufferSupplier = OMX_BufferSupplyOutput;
                } else if (CHECK_PORT_TUNNELED(pExynosPort)) {
                    bufferSupplier->eBufferSupplier = OMX_BufferSupplyInput;
                } else {
                    bufferSupplier->eBufferSupplier = OMX_BufferSupplyUnspecified;
                }
            }
        }
        else
        {
            ret = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }
    }
        break;
    default:
    {
        ret = OMX_ErrorUnsupportedIndex;
        goto EXIT;
    }
        break;
    }

    ret = OMX_ErrorNone;

EXIT:

    FunctionOut();

    return ret;
}

/** The OMX_SetParameter macro will send an initialization parameter
    structure to a component.  Each structure shall be sent one at a time,
    in a separate invocation of the macro.  This macro can only be
    invoked when the component is in the OMX_StateLoaded state, or the
    port is disabled (when the parameter applies to a port). The
    nParamIndex parameter is used to indicate which structure is being
    passed to the component.  The application shall allocate the
    correct structure and shall fill in the structure size and version
    information (as well as the actual data) before invoking this macro.
    The application is free to dispose of this structure after the call
    as the component is required to copy any data it shall retain.  This
    is a blocking call.

    The component should return from this call within 20 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the OMX_GetHandle function.
    @param [in] nIndex
        Index of the structure to be sent.  This value is from the
        OMX_INDEXTYPE enumeration.
    @param [in] pComponentParameterStructure
        pointer to application allocated structure to be used for
        initialization by the component.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp
 */
OMX_ERRORTYPE JetOMX_SetParameter(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE  nIndex,
    OMX_IN OMX_PTR        ComponentParameterStructure)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (ComponentParameterStructure == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pJetOMXComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (nIndex) {
    case OMX_IndexParamAudioInit:
    case OMX_IndexParamVideoInit:
    case OMX_IndexParamImageInit:
    case OMX_IndexParamOtherInit:
    {
        OMX_PORT_PARAM_TYPE *portParam = (OMX_PORT_PARAM_TYPE *)ComponentParameterStructure;
        ret = JetOMX_Check_SizeVersion(portParam, sizeof(OMX_PORT_PARAM_TYPE));
        if (ret != OMX_ErrorNone) {
            goto EXIT;
        }

        if ((pJetOMXComponent->currentState != OMX_StateLoaded) &&
            (pJetOMXComponent->currentState != OMX_StateWaitForResources)) {
            ret = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }
        ret = OMX_ErrorUndefined;
        /* OSAL_Memcpy(&pJetOMXComponent->portParam, portParam, sizeof(OMX_PORT_PARAM_TYPE)); */
    }
        break;
    case OMX_IndexParamPortDefinition:
    {
        OMX_PARAM_PORTDEFINITIONTYPE *portDefinition = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        OMX_U32                       portIndex = portDefinition->nPortIndex;
        JETOMX_BASEPORT          *pExynosPort;

        if (portIndex >= pJetOMXComponent->portParam.nPorts) {
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        ret = JetOMX_Check_SizeVersion(portDefinition, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
        if (ret != OMX_ErrorNone) {
            goto EXIT;
        }

        pExynosPort = &pJetOMXComponent->pExynosPort[portIndex];

        if ((pJetOMXComponent->currentState != OMX_StateLoaded) && (pJetOMXComponent->currentState != OMX_StateWaitForResources)) {
            if (pExynosPort->portDefinition.bEnabled == OMX_TRUE) {
                ret = OMX_ErrorIncorrectStateOperation;
                goto EXIT;
            }
        }
        if (portDefinition->nBufferCountActual < pExynosPort->portDefinition.nBufferCountMin) {
            ret = OMX_ErrorBadParameter;
            goto EXIT;
        }

        OSAL_Memcpy(&pExynosPort->portDefinition, portDefinition, portDefinition->nSize);
    }
        break;
    case OMX_IndexParamPriorityMgmt:
    {
        OMX_PRIORITYMGMTTYPE *compPriority = (OMX_PRIORITYMGMTTYPE *)ComponentParameterStructure;

        if ((pJetOMXComponent->currentState != OMX_StateLoaded) &&
            (pJetOMXComponent->currentState != OMX_StateWaitForResources)) {
            ret = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        ret = JetOMX_Check_SizeVersion(compPriority, sizeof(OMX_PRIORITYMGMTTYPE));
        if (ret != OMX_ErrorNone) {
            goto EXIT;
        }

        pJetOMXComponent->compPriority.nGroupID = compPriority->nGroupID;
        pJetOMXComponent->compPriority.nGroupPriority = compPriority->nGroupPriority;
    }
        break;
    case OMX_IndexParamCompBufferSupplier:
    {
        OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        OMX_U32           portIndex = bufferSupplier->nPortIndex;
        JETOMX_BASEPORT *pExynosPort = NULL;


        if (portIndex >= pJetOMXComponent->portParam.nPorts) {
            ret = OMX_ErrorBadPortIndex;
            goto EXIT;
        }
        ret = JetOMX_Check_SizeVersion(bufferSupplier, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE));
        if (ret != OMX_ErrorNone) {
            goto EXIT;
        }

        pExynosPort = &pJetOMXComponent->pExynosPort[portIndex];
        if ((pJetOMXComponent->currentState != OMX_StateLoaded) && (pJetOMXComponent->currentState != OMX_StateWaitForResources)) {
            if (pExynosPort->portDefinition.bEnabled == OMX_TRUE) {
                ret = OMX_ErrorIncorrectStateOperation;
                goto EXIT;
            }
        }

        if (bufferSupplier->eBufferSupplier == OMX_BufferSupplyUnspecified) {
            ret = OMX_ErrorNone;
            goto EXIT;
        }
        if (CHECK_PORT_TUNNELED(pExynosPort) == 0) {
            ret = OMX_ErrorNone; /*OMX_ErrorNone ?????*/
            goto EXIT;
        }

        if (pExynosPort->portDefinition.eDir == OMX_DirInput) {
            if (bufferSupplier->eBufferSupplier == OMX_BufferSupplyInput) {
                /*
                if (CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
                    ret = OMX_ErrorNone;
                }
                */
                pExynosPort->tunnelFlags |= EXYNOS_TUNNEL_IS_SUPPLIER;
                bufferSupplier->nPortIndex = pExynosPort->tunneledPort;
                ret = OMX_SetParameter(pExynosPort->tunneledComponent, OMX_IndexParamCompBufferSupplier, bufferSupplier);
                goto EXIT;
            } else if (bufferSupplier->eBufferSupplier == OMX_BufferSupplyOutput) {
                ret = OMX_ErrorNone;
                if (CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
                    pExynosPort->tunnelFlags &= ~EXYNOS_TUNNEL_IS_SUPPLIER;
                    bufferSupplier->nPortIndex = pExynosPort->tunneledPort;
                    ret = OMX_SetParameter(pExynosPort->tunneledComponent, OMX_IndexParamCompBufferSupplier, bufferSupplier);
                }
                goto EXIT;
            }
        } else if (pExynosPort->portDefinition.eDir == OMX_DirOutput) {
            if (bufferSupplier->eBufferSupplier == OMX_BufferSupplyInput) {
                ret = OMX_ErrorNone;
                if (CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
                    pExynosPort->tunnelFlags &= ~EXYNOS_TUNNEL_IS_SUPPLIER;
                    ret = OMX_ErrorNone;
                }
                goto EXIT;
            } else if (bufferSupplier->eBufferSupplier == OMX_BufferSupplyOutput) {
                /*
                if (CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
                    ret = OMX_ErrorNone;
                }
                */
                pExynosPort->tunnelFlags |= EXYNOS_TUNNEL_IS_SUPPLIER;
                ret = OMX_ErrorNone;
                goto EXIT;
            }
        }
    }
        break;
    default:
    {
        ret = OMX_ErrorUnsupportedIndex;
        goto EXIT;
    }
        break;
    }

    ret = OMX_ErrorNone;

EXIT:

    FunctionOut();

    return ret;
}

/** The OMX_GetConfig macro will get one of the configuration structures
    from a component.  This macro can be invoked anytime after the
    component has been loaded.  The nParamIndex call parameter is used to
    indicate which structure is being requested from the component.  The
    application shall allocate the correct structure and shall fill in the
    structure size and version information before invoking this macro.
    If the component has not had this configuration parameter sent before,
    then the component should return a set of valid DEFAULT values for the
    component.  This is a blocking call.

    The component should return from this call within 5 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the OMX_GetHandle function.
    @param [in] nIndex
        Index of the structure to be filled.  This value is from the
        OMX_INDEXTYPE enumeration.
    @param [in,out] pComponentConfigStructure
        pointer to application allocated structure to be filled by the
        component.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp
*/
OMX_ERRORTYPE JetOMX_GetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE  nIndex,
    OMX_INOUT OMX_PTR     pComponentConfigStructure)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (pComponentConfigStructure == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pJetOMXComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (nIndex) {
    default:
        ret = OMX_ErrorUnsupportedIndex;
        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

/** The OMX_SetConfig macro will send one of the configuration
    structures to a component.  Each structure shall be sent one at a time,
    each in a separate invocation of the macro.  This macro can be invoked
    anytime after the component has been loaded.  The application shall
    allocate the correct structure and shall fill in the structure size
    and version information (as well as the actual data) before invoking
    this macro.  The application is free to dispose of this structure after
    the call as the component is required to copy any data it shall retain.
    This is a blocking call.

    The component should return from this call within 5 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the OMX_GetHandle function.
    @param [in] nConfigIndex
        Index of the structure to be sent.  This value is from the
        OMX_INDEXTYPE enumeration above.
    @param [in] pComponentConfigStructure
        pointer to application allocated structure to be used for
        initialization by the component.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp
 */
OMX_ERRORTYPE JetOMX_SetConfig(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE  nIndex,
    OMX_IN OMX_PTR        pComponentConfigStructure)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (pComponentConfigStructure == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pJetOMXComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (nIndex) {
    default:
        ret = OMX_ErrorUnsupportedIndex;
        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

/** The OMX_GetExtensionIndex macro will invoke a component to translate
    a vendor specific configuration or parameter string into an OMX
    structure index.  There is no requirement for the vendor to support
    this command for the indexes already found in the OMX_INDEXTYPE
    enumeration (this is done to save space in small components).  The
    component shall support all vendor supplied extension indexes not found
    in the master OMX_INDEXTYPE enumeration.  This is a blocking call.

    The component should return from this call within 5 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the GetHandle function.
    @param [in] cParameterName
        OMX_STRING that shall be less than 128 characters long including
        the trailing null byte.  This is the string that will get
        translated by the component into a configuration index.
    @param [out] pIndexType
        a pointer to a OMX_INDEXTYPE to receive the index value.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp
 */
OMX_ERRORTYPE JetOMX_GetExtensionIndex(
    OMX_IN OMX_HANDLETYPE  hComponent,
    OMX_IN OMX_STRING      cParameterName,
    OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if ((cParameterName == NULL) || (pIndexType == NULL)) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pJetOMXComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    ret = OMX_ErrorBadParameter;

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_SetCallbacks (
    OMX_IN OMX_HANDLETYPE    hComponent,
    OMX_IN OMX_CALLBACKTYPE* pCallbacks,
    OMX_IN OMX_PTR           pAppData)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (pCallbacks == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pJetOMXComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }
    if (pJetOMXComponent->currentState != OMX_StateLoaded) {
        ret = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    pJetOMXComponent->pCallbacks = pCallbacks;
    pJetOMXComponent->callbackData = pAppData;

    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

/** The OMX_UseEGLImage macro will request that the component use
    a EGLImage provided by EGL (and allocate its own buffer header)
    This is a blocking call.

    The component should return from this call within 20 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the OMX_GetHandle function.
    @param [out] ppBuffer
        pointer to an OMX_BUFFERHEADERTYPE structure used to receive the
        pointer to the buffer header.  Note that the memory location used
        for this buffer is NOT visible to the IL Client.
    @param [in] nPortIndex
        nPortIndex is used to select the port on the component the buffer will
        be used with.  The port can be found by using the nPortIndex
        value as an index into the Port Definition array of the component.
    @param [in] pAppPrivate
        pAppPrivate is used to initialize the pAppPrivate member of the
        buffer header structure.
    @param [in] eglImage
        eglImage contains the handle of the EGLImage to use as a buffer on the
        specified port.  The component is expected to validate properties of
        the EGLImage against the configuration of the port to ensure the component
        can use the EGLImage as a buffer.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp buf
 */
OMX_ERRORTYPE JetOMX_UseEGLImage(
    OMX_IN OMX_HANDLETYPE            hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
    OMX_IN OMX_U32                   nPortIndex,
    OMX_IN OMX_PTR                   pAppPrivate,
    OMX_IN void                     *eglImage)
{
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE JetOMX_BaseComponent_Constructor(
    OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorBadParameter, Line:%d", __LINE__);
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    pJetOMXComponent = OSAL_Malloc(sizeof(JETOMX_BASECOMPONENT));
    if (pJetOMXComponent == NULL) {
        ret = OMX_ErrorInsufficientResources;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
        goto EXIT;
    }
    OSAL_Memset(pJetOMXComponent, 0, sizeof(JETOMX_BASECOMPONENT));
    pOMXComponent->pComponentPrivate = (OMX_PTR)pJetOMXComponent;

    ret = OSAL_SemaphoreCreate(&pJetOMXComponent->msgSemaphoreHandle);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorInsufficientResources;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
        goto EXIT;
    }
    ret = OSAL_MutexCreate(&pJetOMXComponent->compMutex);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorInsufficientResources;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
        goto EXIT;
    }
    ret = OSAL_SignalCreate(&pJetOMXComponent->abendStateEvent);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorInsufficientResources;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
        goto EXIT;
    }

    pJetOMXComponent->bExitMessageHandlerThread = OMX_FALSE;
    OSAL_QueueCreate(&pJetOMXComponent->messageQ, MAX_QUEUE_ELEMENTS);
    ret = OSAL_ThreadCreate(&pJetOMXComponent->hMessageHandler, JetOMX_MessageHandlerThread, pOMXComponent);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorInsufficientResources;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
        goto EXIT;
    }

    pJetOMXComponent->bMultiThreadProcess = OMX_FALSE;

    pOMXComponent->GetComponentVersion = &JetOMX_GetComponentVersion;
    pOMXComponent->SendCommand         = &JetOMX_SendCommand;
    pOMXComponent->GetState            = &JetOMX_GetState;
    pOMXComponent->SetCallbacks        = &JetOMX_SetCallbacks;
    pOMXComponent->UseEGLImage         = &JetOMX_UseEGLImage;

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_BaseComponent_Destructor(
    OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pJetOMXComponent = NULL;
    OMX_S32                   semaValue = 0;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pJetOMXComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    JetOMX_CommandQueue(pJetOMXComponent, EXYNOS_OMX_CommandComponentDeInit, 0, NULL);
    OSAL_SleepMillisec(0);
    OSAL_Get_SemaphoreCount(pJetOMXComponent->msgSemaphoreHandle, &semaValue);
    if (semaValue == 0)
        OSAL_SemaphorePost(pJetOMXComponent->msgSemaphoreHandle);
    OSAL_SemaphorePost(pJetOMXComponent->msgSemaphoreHandle);

    OSAL_ThreadTerminate(pJetOMXComponent->hMessageHandler);
    pJetOMXComponent->hMessageHandler = NULL;

    OSAL_SignalTerminate(pJetOMXComponent->abendStateEvent);
    pJetOMXComponent->abendStateEvent = NULL;
    OSAL_MutexTerminate(pJetOMXComponent->compMutex);
    pJetOMXComponent->compMutex = NULL;
    OSAL_SemaphoreTerminate(pJetOMXComponent->msgSemaphoreHandle);
    pJetOMXComponent->msgSemaphoreHandle = NULL;
    OSAL_QueueTerminate(&pJetOMXComponent->messageQ);

    OSAL_Free(pJetOMXComponent);
    pJetOMXComponent = NULL;

    ret = OMX_ErrorNone;
EXIT:
    FunctionOut();

    return ret;
}
