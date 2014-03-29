/*
 * @file       JetOMX_Baseport.c
 * @brief
 * @author
 * @version    0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JetOMX_Macros.h"
#include "OSAL_Event.h"
#include "OSAL_Semaphore.h"
#include "OSAL_Mutex.h"

#include "JetOMX_Baseport.h"
#include "JetOMX_Basecomponent.h"

#undef  JETOMX_LOG_TAG
#define JETOMX_LOG_TAG    "JETOMX_BASE_PORT"
#define JETOMX_LOG_OFF
#define JETOMX_TRACE_ON
#include "OSAL_Log.h"


OMX_ERRORTYPE JetOMX_InputBufferReturn(OMX_COMPONENTTYPE *pOMXComponent, OMX_BUFFERHEADERTYPE* bufferHeader)
{
    OMX_ERRORTYPE         ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT *pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    JETOMX_BASEPORT      *pExynosPort = &pExynosComponent->pExynosPort[INPUT_PORT_INDEX];
    OMX_U32               i = 0;

    OSAL_MutexLock(pExynosPort->hPortMutex);
    for (i = 0; i < pExynosPort->portDefinition.nBufferCountActual; i++) {
        if (bufferHeader == pExynosPort->extendBufferHeader[i].OMXBufferHeader) {
            pExynosPort->extendBufferHeader[i].bBufferInOMX = OMX_FALSE;
            break;
        }
    }

    OSAL_MutexUnlock(pExynosPort->hPortMutex);
    pExynosComponent->pCallbacks->EmptyBufferDone(pOMXComponent, pExynosComponent->callbackData, bufferHeader);

    return ret;
}

OMX_ERRORTYPE JetOMX_OutputBufferReturn(OMX_COMPONENTTYPE *pOMXComponent, OMX_BUFFERHEADERTYPE* bufferHeader)
{
    OMX_ERRORTYPE         ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT *pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    JETOMX_BASEPORT      *pExynosPort = &pExynosComponent->pExynosPort[OUTPUT_PORT_INDEX];
    OMX_U32               i = 0;

    OSAL_MutexLock(pExynosPort->hPortMutex);
    for (i = 0; i < pExynosPort->portDefinition.nBufferCountActual; i++) {
        if (bufferHeader == pExynosPort->extendBufferHeader[i].OMXBufferHeader) {
            pExynosPort->extendBufferHeader[i].bBufferInOMX = OMX_FALSE;
            break;
        }
    }

    OSAL_MutexUnlock(pExynosPort->hPortMutex);
    pExynosComponent->pCallbacks->FillBufferDone(pOMXComponent, pExynosComponent->callbackData, bufferHeader);

    return ret;
}

OMX_ERRORTYPE JetOMX_BufferFlushProcess(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 nPortIndex, OMX_BOOL bEvent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT     *pExynosComponent = NULL;
    JETOMX_BASEPORT          *pExynosPort = NULL;
    OMX_S32                   portIndex = 0;
    EXYNOS_OMX_DATABUFFER    *flushPortBuffer[2] = {NULL, NULL};
    OMX_U32                   i = 0, cnt = 0;

    FunctionIn();

    if (pOMXComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    cnt = (nPortIndex == ALL_PORT_INDEX ) ? ALL_PORT_NUM : 1;

    for (i = 0; i < cnt; i++) {
        if (nPortIndex == ALL_PORT_INDEX)
            portIndex = i;
        else
            portIndex = nPortIndex;

        pExynosComponent->exynos_BufferFlush(pOMXComponent, portIndex, bEvent);
    }

EXIT:
    if ((ret != OMX_ErrorNone) && (pOMXComponent != NULL) && (pExynosComponent != NULL)) {
        OSAL_Log(JETOMX_LOG_ERROR,"%s : %d", __FUNCTION__, __LINE__);
        pExynosComponent->pCallbacks->EventHandler(pOMXComponent,
                        pExynosComponent->callbackData,
                        OMX_EventError,
                        ret, 0, NULL);
    }

    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_EnablePort(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 portIndex)
{
    OMX_ERRORTYPE          ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT  *pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    JETOMX_BASEPORT       *pExynosPort = NULL;
    OMX_U32                i = 0, cnt = 0;

    FunctionIn();

    pExynosPort = &pExynosComponent->pExynosPort[portIndex];

    if ((pExynosComponent->currentState != OMX_StateLoaded) && (pExynosComponent->currentState != OMX_StateWaitForResources)) {
        OSAL_SemaphoreWait(pExynosPort->loadedResource);
        pExynosPort->portDefinition.bPopulated = OMX_TRUE;
    }
    pExynosPort->exceptionFlag = GENERAL_STATE;
    pExynosPort->portDefinition.bEnabled = OMX_TRUE;

    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_PortEnableProcess(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 nPortIndex)
{
    OMX_ERRORTYPE          ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT  *pExynosComponent = NULL;
    OMX_S32                portIndex = 0;
    OMX_U32                i = 0, cnt = 0;

    FunctionIn();

    if (pOMXComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    cnt = (nPortIndex == ALL_PORT_INDEX) ? ALL_PORT_NUM : 1;

    for (i = 0; i < cnt; i++) {
        if (nPortIndex == ALL_PORT_INDEX)
            portIndex = i;
        else
            portIndex = nPortIndex;

        ret = JetOMX_EnablePort(pOMXComponent, portIndex);
        if (ret == OMX_ErrorNone) {
            pExynosComponent->pCallbacks->EventHandler(pOMXComponent,
                            pExynosComponent->callbackData,
                            OMX_EventCmdComplete,
                            OMX_CommandPortEnable, portIndex, NULL);
        }
    }

EXIT:
    if ((ret != OMX_ErrorNone) && (pOMXComponent != NULL) && (pExynosComponent != NULL)) {
            pExynosComponent->pCallbacks->EventHandler(pOMXComponent,
                            pExynosComponent->callbackData,
                            OMX_EventError,
                            ret, 0, NULL);
        }

    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_DisablePort(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 portIndex)
{
    OMX_ERRORTYPE          ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT  *pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    JETOMX_BASEPORT       *pExynosPort = NULL;
    OMX_U32                i = 0, elemNum = 0;
    EXYNOS_OMX_MESSAGE       *message;

    FunctionIn();

    pExynosPort = &pExynosComponent->pExynosPort[portIndex];

    if (!CHECK_PORT_ENABLED(pExynosPort)) {
        ret = OMX_ErrorNone;
        goto EXIT;
    }

    if (pExynosComponent->currentState != OMX_StateLoaded) {
        if (CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)) {
            while (OSAL_GetElemNum(&pExynosPort->bufferQ) > 0) {
                message = (EXYNOS_OMX_MESSAGE*)OSAL_Dequeue(&pExynosPort->bufferQ);
                OSAL_Free(message);
            }
        }
        pExynosPort->portDefinition.bPopulated = OMX_FALSE;
        OSAL_SemaphoreWait(pExynosPort->unloadedResource);
    }
    pExynosPort->portDefinition.bEnabled = OMX_FALSE;
    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_PortDisableProcess(OMX_COMPONENTTYPE *pOMXComponent, OMX_S32 nPortIndex)
{
    OMX_ERRORTYPE          ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT  *pExynosComponent = NULL;
    JETOMX_BASEPORT       *pExynosPort = NULL;
    OMX_S32                portIndex = 0;
    OMX_U32                i = 0, cnt = 0;
    EXYNOS_OMX_DATABUFFER    *flushPortBuffer[2] = {NULL, NULL};

    FunctionIn();

    if (pOMXComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    cnt = (nPortIndex == ALL_PORT_INDEX ) ? ALL_PORT_NUM : 1;

    /* port flush*/
    for(i = 0; i < cnt; i++) {
        if (nPortIndex == ALL_PORT_INDEX)
            portIndex = i;
        else
            portIndex = nPortIndex;

        JetOMX_BufferFlushProcess(pOMXComponent, portIndex, OMX_FALSE);
    }

    for(i = 0; i < cnt; i++) {
        if (nPortIndex == ALL_PORT_INDEX)
            portIndex = i;
        else
            portIndex = nPortIndex;

        ret = JetOMX_DisablePort(pOMXComponent, portIndex);
        pExynosComponent->pExynosPort[portIndex].bIsPortDisabled = OMX_FALSE;
        if (ret == OMX_ErrorNone) {
            pExynosComponent->pCallbacks->EventHandler(pOMXComponent,
                            pExynosComponent->callbackData,
                            OMX_EventCmdComplete,
                            OMX_CommandPortDisable, portIndex, NULL);
        }
    }

EXIT:
    if ((ret != OMX_ErrorNone) && (pOMXComponent != NULL) && (pExynosComponent != NULL)) {
        pExynosComponent->pCallbacks->EventHandler(pOMXComponent,
                        pExynosComponent->callbackData,
                        OMX_EventError,
                        ret, 0, NULL);
    }

    FunctionOut();

    return ret;
}

/** The OMX_EmptyThisBuffer macro will send a buffer full of data to an
    input port of a component.  The buffer will be emptied by the component
    and returned to the application via the EmptyBufferDone call back.
    This is a non-blocking call in that the component will record the buffer
    and return immediately and then empty the buffer, later, at the proper
    time.  As expected, this macro may be invoked only while the component
    is in the OMX_StateExecuting.  If nPortIndex does not specify an input
    port, the component shall return an error.

    The component should return from this call within 5 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the OMX_GetHandle function.
    @param [in] pBuffer
        pointer to an OMX_BUFFERHEADERTYPE structure allocated with UseBuffer
        or AllocateBuffer.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp buf
 */
OMX_ERRORTYPE JetOMX_EmptyThisBuffer(
    OMX_IN OMX_HANDLETYPE        hComponent,
    OMX_IN OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE           ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE     *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT  *pExynosComponent = NULL;
    JETOMX_BASEPORT       *pExynosPort = NULL;
    OMX_BOOL               findBuffer = OMX_FALSE;
    EXYNOS_OMX_MESSAGE       *message;
    OMX_U32                i = 0;

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
    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    if (pExynosComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    if (pBuffer == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pBuffer->nInputPortIndex != INPUT_PORT_INDEX) {
        ret = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

    ret = JetOMX_Check_SizeVersion(pBuffer, sizeof(OMX_BUFFERHEADERTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if ((pExynosComponent->currentState != OMX_StateIdle) &&
        (pExynosComponent->currentState != OMX_StateExecuting) &&
        (pExynosComponent->currentState != OMX_StatePause)) {
        ret = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    pExynosPort = &pExynosComponent->pExynosPort[INPUT_PORT_INDEX];
    if ((!CHECK_PORT_ENABLED(pExynosPort)) ||
        ((CHECK_PORT_BEING_FLUSHED(pExynosPort) || CHECK_PORT_BEING_DISABLED(pExynosPort)) &&
        (!CHECK_PORT_TUNNELED(pExynosPort) || !CHECK_PORT_BUFFER_SUPPLIER(pExynosPort))) ||
        ((pExynosComponent->transientState == EXYNOS_OMX_TransStateExecutingToIdle) &&
        (CHECK_PORT_TUNNELED(pExynosPort) && !CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)))) {
        ret = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    OSAL_MutexLock(pExynosPort->hPortMutex);
    for (i = 0; i < pExynosPort->portDefinition.nBufferCountActual; i++) {
        if (pBuffer == pExynosPort->extendBufferHeader[i].OMXBufferHeader) {
            pExynosPort->extendBufferHeader[i].bBufferInOMX = OMX_TRUE;
            findBuffer = OMX_TRUE;
            break;
        }
    }

    if (findBuffer == OMX_FALSE) {
        ret = OMX_ErrorBadParameter;
        OSAL_MutexUnlock(pExynosPort->hPortMutex);
        goto EXIT;
    }

    message = OSAL_Malloc(sizeof(EXYNOS_OMX_MESSAGE));
    if (message == NULL) {
        ret = OMX_ErrorInsufficientResources;
        OSAL_MutexUnlock(pExynosPort->hPortMutex);
        goto EXIT;
    }
    message->messageType = EXYNOS_OMX_CommandEmptyBuffer;
    message->messageParam = (OMX_U32) i;
    message->pCmdData = (OMX_PTR)pBuffer;

    ret = OSAL_Queue(&pExynosPort->bufferQ, (void *)message);
    if (ret != 0) {
        ret = OMX_ErrorUndefined;
        OSAL_MutexUnlock(pExynosPort->hPortMutex);
        goto EXIT;
    }
    ret = OSAL_SemaphorePost(pExynosPort->bufferSemID);
    OSAL_MutexUnlock(pExynosPort->hPortMutex);

EXIT:
    FunctionOut();

    return ret;
}

/** The OMX_FillThisBuffer macro will send an empty buffer to an
    output port of a component.  The buffer will be filled by the component
    and returned to the application via the FillBufferDone call back.
    This is a non-blocking call in that the component will record the buffer
    and return immediately and then fill the buffer, later, at the proper
    time.  As expected, this macro may be invoked only while the component
    is in the OMX_ExecutingState.  If nPortIndex does not specify an output
    port, the component shall return an error.

    The component should return from this call within 5 msec.

    @param [in] hComponent
        Handle of the component to be accessed.  This is the component
        handle returned by the call to the OMX_GetHandle function.
    @param [in] pBuffer
        pointer to an OMX_BUFFERHEADERTYPE structure allocated with UseBuffer
        or AllocateBuffer.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  Otherwise the appropriate OMX error will be returned.
    @ingroup comp buf
 */
OMX_ERRORTYPE JetOMX_FillThisBuffer(
    OMX_IN OMX_HANDLETYPE        hComponent,
    OMX_IN OMX_BUFFERHEADERTYPE *pBuffer)
{
    OMX_ERRORTYPE           ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE     *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT  *pExynosComponent = NULL;
    JETOMX_BASEPORT       *pExynosPort = NULL;
    OMX_BOOL               findBuffer = OMX_FALSE;
    EXYNOS_OMX_MESSAGE       *message;
    OMX_U32                i = 0;

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
    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    if (pExynosComponent->currentState == OMX_StateInvalid) {
        ret = OMX_ErrorInvalidState;
        goto EXIT;
    }

    if (pBuffer == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pBuffer->nOutputPortIndex != OUTPUT_PORT_INDEX) {
        ret = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

    ret = JetOMX_Check_SizeVersion(pBuffer, sizeof(OMX_BUFFERHEADERTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if ((pExynosComponent->currentState != OMX_StateIdle) &&
        (pExynosComponent->currentState != OMX_StateExecuting) &&
        (pExynosComponent->currentState != OMX_StatePause)) {
        ret = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    pExynosPort = &pExynosComponent->pExynosPort[OUTPUT_PORT_INDEX];
    if ((!CHECK_PORT_ENABLED(pExynosPort)) ||
        ((CHECK_PORT_BEING_FLUSHED(pExynosPort) || CHECK_PORT_BEING_DISABLED(pExynosPort)) &&
        (!CHECK_PORT_TUNNELED(pExynosPort) || !CHECK_PORT_BUFFER_SUPPLIER(pExynosPort))) ||
        ((pExynosComponent->transientState == EXYNOS_OMX_TransStateExecutingToIdle) &&
        (CHECK_PORT_TUNNELED(pExynosPort) && !CHECK_PORT_BUFFER_SUPPLIER(pExynosPort)))) {
        ret = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    OSAL_MutexLock(pExynosPort->hPortMutex);
    for (i = 0; i < pExynosPort->portDefinition.nBufferCountActual; i++) {
        if (pBuffer == pExynosPort->extendBufferHeader[i].OMXBufferHeader) {
            pExynosPort->extendBufferHeader[i].bBufferInOMX = OMX_TRUE;
            findBuffer = OMX_TRUE;
            break;
        }
    }

    if (findBuffer == OMX_FALSE) {
        ret = OMX_ErrorBadParameter;
        OSAL_MutexUnlock(pExynosPort->hPortMutex);
        goto EXIT;
    }

    message = OSAL_Malloc(sizeof(EXYNOS_OMX_MESSAGE));
    if (message == NULL) {
        ret = OMX_ErrorInsufficientResources;
        OSAL_MutexUnlock(pExynosPort->hPortMutex);
        goto EXIT;
    }
    message->messageType = EXYNOS_OMX_CommandFillBuffer;
    message->messageParam = (OMX_U32) i;
    message->pCmdData = (OMX_PTR)pBuffer;

    ret = OSAL_Queue(&pExynosPort->bufferQ, (void *)message);
    if (ret != 0) {
        ret = OMX_ErrorUndefined;
        OSAL_MutexUnlock(pExynosPort->hPortMutex);
        goto EXIT;
    }

    ret = OSAL_SemaphorePost(pExynosPort->bufferSemID);
    OSAL_MutexUnlock(pExynosPort->hPortMutex);

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_Port_Constructor(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE          ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE     *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT *pExynosComponent = NULL;
    JETOMX_BASEPORT      *pExynosPort = NULL;
    JETOMX_BASEPORT      *pExynosInputPort = NULL;
    JETOMX_BASEPORT      *pExynosOutputPort = NULL;
    int i = 0;

    FunctionIn();

    if (hComponent == NULL) {
        ret = OMX_ErrorBadParameter;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorBadParameter, Line:%d", __LINE__);
        goto EXIT;
    }
    pOMXComponent = (OMX_COMPONENTTYPE *)hComponent;
    ret = JetOMX_Check_SizeVersion(pOMXComponent, sizeof(OMX_COMPONENTTYPE));
    if (ret != OMX_ErrorNone) {
        goto EXIT;
    }

    if (pOMXComponent->pComponentPrivate == NULL) {
        ret = OMX_ErrorBadParameter;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorBadParameter, Line:%d", __LINE__);
        goto EXIT;
    }
    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    INIT_SET_SIZE_VERSION(&pExynosComponent->portParam, OMX_PORT_PARAM_TYPE);
    pExynosComponent->portParam.nPorts = ALL_PORT_NUM;
    pExynosComponent->portParam.nStartPortNumber = INPUT_PORT_INDEX;

    pExynosPort = OSAL_Malloc(sizeof(JETOMX_BASEPORT) * ALL_PORT_NUM);
    if (pExynosPort == NULL) {
        ret = OMX_ErrorInsufficientResources;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
        goto EXIT;
    }
    OSAL_Memset(pExynosPort, 0, sizeof(JETOMX_BASEPORT) * ALL_PORT_NUM);
    pExynosComponent->pExynosPort = pExynosPort;

    /* Input Port */
    pExynosInputPort = &pExynosPort[INPUT_PORT_INDEX];

    OSAL_QueueCreate(&pExynosInputPort->bufferQ, MAX_QUEUE_ELEMENTS);

    pExynosInputPort->extendBufferHeader = OSAL_Malloc(sizeof(EXYNOS_OMX_BUFFERHEADERTYPE) * MAX_BUFFER_NUM);
    if (pExynosInputPort->extendBufferHeader == NULL) {
        OSAL_Free(pExynosPort);
        pExynosPort = NULL;
        ret = OMX_ErrorInsufficientResources;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
        goto EXIT;
    }
    OSAL_Memset(pExynosInputPort->extendBufferHeader, 0, sizeof(EXYNOS_OMX_BUFFERHEADERTYPE) * MAX_BUFFER_NUM);

    pExynosInputPort->bufferStateAllocate = OSAL_Malloc(sizeof(OMX_U32) * MAX_BUFFER_NUM);
    if (pExynosInputPort->bufferStateAllocate == NULL) {
        OSAL_Free(pExynosInputPort->extendBufferHeader);
        pExynosInputPort->extendBufferHeader = NULL;
        OSAL_Free(pExynosPort);
        pExynosPort = NULL;
        ret = OMX_ErrorInsufficientResources;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInsufficientResources, Line:%d", __LINE__);
        goto EXIT;
    }
    OSAL_Memset(pExynosInputPort->bufferStateAllocate, 0, sizeof(OMX_U32) * MAX_BUFFER_NUM);

    pExynosInputPort->bufferSemID = NULL;
    pExynosInputPort->assignedBufferNum = 0;
    pExynosInputPort->portState = OMX_StateMax;
    pExynosInputPort->bIsPortFlushed = OMX_FALSE;
    pExynosInputPort->bIsPortDisabled = OMX_FALSE;
    pExynosInputPort->tunneledComponent = NULL;
    pExynosInputPort->tunneledPort = 0;
    pExynosInputPort->tunnelBufferNum = 0;
    pExynosInputPort->bufferSupplier = OMX_BufferSupplyUnspecified;
    pExynosInputPort->tunnelFlags = 0;
    ret = OSAL_SemaphoreCreate(&pExynosInputPort->loadedResource);
    if (ret != OMX_ErrorNone) {
        OSAL_Free(pExynosInputPort->bufferStateAllocate);
        pExynosInputPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosInputPort->extendBufferHeader);
        pExynosInputPort->extendBufferHeader = NULL;
        OSAL_Free(pExynosPort);
        pExynosPort = NULL;
        goto EXIT;
    }
    ret = OSAL_SemaphoreCreate(&pExynosInputPort->unloadedResource);
    if (ret != OMX_ErrorNone) {
        OSAL_SemaphoreTerminate(pExynosInputPort->loadedResource);
        pExynosInputPort->loadedResource = NULL;
        OSAL_Free(pExynosInputPort->bufferStateAllocate);
        pExynosInputPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosInputPort->extendBufferHeader);
        pExynosInputPort->extendBufferHeader = NULL;
        OSAL_Free(pExynosPort);
        pExynosPort = NULL;
        goto EXIT;
    }

    INIT_SET_SIZE_VERSION(&pExynosInputPort->portDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
    pExynosInputPort->portDefinition.nPortIndex = INPUT_PORT_INDEX;
    pExynosInputPort->portDefinition.eDir = OMX_DirInput;
    pExynosInputPort->portDefinition.nBufferCountActual = 0;
    pExynosInputPort->portDefinition.nBufferCountMin = 0;
    pExynosInputPort->portDefinition.nBufferSize = 0;
    pExynosInputPort->portDefinition.bEnabled = OMX_FALSE;
    pExynosInputPort->portDefinition.bPopulated = OMX_FALSE;
    pExynosInputPort->portDefinition.eDomain = OMX_PortDomainMax;
    pExynosInputPort->portDefinition.bBuffersContiguous = OMX_FALSE;
    pExynosInputPort->portDefinition.nBufferAlignment = 0;
    pExynosInputPort->markType.hMarkTargetComponent = NULL;
    pExynosInputPort->markType.pMarkData = NULL;
    pExynosInputPort->exceptionFlag = GENERAL_STATE;

    /* Output Port */
    pExynosOutputPort = &pExynosPort[OUTPUT_PORT_INDEX];

    OSAL_QueueCreate(&pExynosOutputPort->bufferQ, MAX_QUEUE_ELEMENTS); /* For in case of "Output Buffer Share", MAX ELEMENTS(DPB + EDPB) */

    pExynosOutputPort->extendBufferHeader = OSAL_Malloc(sizeof(EXYNOS_OMX_BUFFERHEADERTYPE) * MAX_BUFFER_NUM);
    if (pExynosOutputPort->extendBufferHeader == NULL) {
        OSAL_SemaphoreTerminate(pExynosInputPort->unloadedResource);
        pExynosInputPort->unloadedResource = NULL;
        OSAL_SemaphoreTerminate(pExynosInputPort->loadedResource);
        pExynosInputPort->loadedResource = NULL;
        OSAL_Free(pExynosInputPort->bufferStateAllocate);
        pExynosInputPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosInputPort->extendBufferHeader);
        pExynosInputPort->extendBufferHeader = NULL;
        OSAL_Free(pExynosPort);
        pExynosPort = NULL;
        ret = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    OSAL_Memset(pExynosOutputPort->extendBufferHeader, 0, sizeof(EXYNOS_OMX_BUFFERHEADERTYPE) * MAX_BUFFER_NUM);

    pExynosOutputPort->bufferStateAllocate = OSAL_Malloc(sizeof(OMX_U32) * MAX_BUFFER_NUM);
    if (pExynosOutputPort->bufferStateAllocate == NULL) {
        OSAL_Free(pExynosOutputPort->extendBufferHeader);
        pExynosOutputPort->extendBufferHeader = NULL;

        OSAL_SemaphoreTerminate(pExynosInputPort->unloadedResource);
        pExynosInputPort->unloadedResource = NULL;
        OSAL_SemaphoreTerminate(pExynosInputPort->loadedResource);
        pExynosInputPort->loadedResource = NULL;
        OSAL_Free(pExynosInputPort->bufferStateAllocate);
        pExynosInputPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosInputPort->extendBufferHeader);
        pExynosInputPort->extendBufferHeader = NULL;
        OSAL_Free(pExynosPort);
        pExynosPort = NULL;
        ret = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    OSAL_Memset(pExynosOutputPort->bufferStateAllocate, 0, sizeof(OMX_U32) * MAX_BUFFER_NUM);

    pExynosOutputPort->bufferSemID = NULL;
    pExynosOutputPort->assignedBufferNum = 0;
    pExynosOutputPort->portState = OMX_StateMax;
    pExynosOutputPort->bIsPortFlushed = OMX_FALSE;
    pExynosOutputPort->bIsPortDisabled = OMX_FALSE;
    pExynosOutputPort->tunneledComponent = NULL;
    pExynosOutputPort->tunneledPort = 0;
    pExynosOutputPort->tunnelBufferNum = 0;
    pExynosOutputPort->bufferSupplier = OMX_BufferSupplyUnspecified;
    pExynosOutputPort->tunnelFlags = 0;
    ret = OSAL_SemaphoreCreate(&pExynosOutputPort->loadedResource);
    if (ret != OMX_ErrorNone) {
        OSAL_Free(pExynosOutputPort->bufferStateAllocate);
        pExynosOutputPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosOutputPort->extendBufferHeader);
        pExynosOutputPort->extendBufferHeader = NULL;

        OSAL_SemaphoreTerminate(pExynosInputPort->unloadedResource);
        pExynosInputPort->unloadedResource = NULL;
        OSAL_SemaphoreTerminate(pExynosInputPort->loadedResource);
        pExynosInputPort->loadedResource = NULL;
        OSAL_Free(pExynosInputPort->bufferStateAllocate);
        pExynosInputPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosInputPort->extendBufferHeader);
        pExynosInputPort->extendBufferHeader = NULL;
        OSAL_Free(pExynosPort);
        pExynosPort = NULL;
        goto EXIT;
    }
    ret = OSAL_SemaphoreCreate(&pExynosOutputPort->unloadedResource);
    if (ret != OMX_ErrorNone) {
        OSAL_SemaphoreTerminate(pExynosOutputPort->loadedResource);
        pExynosOutputPort->loadedResource = NULL;
        OSAL_Free(pExynosOutputPort->bufferStateAllocate);
        pExynosOutputPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosOutputPort->extendBufferHeader);
        pExynosOutputPort->extendBufferHeader = NULL;

        OSAL_SemaphoreTerminate(pExynosInputPort->unloadedResource);
        pExynosInputPort->unloadedResource = NULL;
        OSAL_SemaphoreTerminate(pExynosInputPort->loadedResource);
        pExynosInputPort->loadedResource = NULL;
        OSAL_Free(pExynosInputPort->bufferStateAllocate);
        pExynosInputPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosInputPort->extendBufferHeader);
        pExynosInputPort->extendBufferHeader = NULL;
        OSAL_Free(pExynosPort);
        pExynosPort = NULL;
        goto EXIT;
    }

    INIT_SET_SIZE_VERSION(&pExynosOutputPort->portDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
    pExynosOutputPort->portDefinition.nPortIndex = OUTPUT_PORT_INDEX;
    pExynosOutputPort->portDefinition.eDir = OMX_DirOutput;
    pExynosOutputPort->portDefinition.nBufferCountActual = 0;
    pExynosOutputPort->portDefinition.nBufferCountMin = 0;
    pExynosOutputPort->portDefinition.nBufferSize = 0;
    pExynosOutputPort->portDefinition.bEnabled = OMX_FALSE;
    pExynosOutputPort->portDefinition.bPopulated = OMX_FALSE;
    pExynosOutputPort->portDefinition.eDomain = OMX_PortDomainMax;
    pExynosOutputPort->portDefinition.bBuffersContiguous = OMX_FALSE;
    pExynosOutputPort->portDefinition.nBufferAlignment = 0;
    pExynosOutputPort->markType.hMarkTargetComponent = NULL;
    pExynosOutputPort->markType.pMarkData = NULL;
    pExynosOutputPort->exceptionFlag = GENERAL_STATE;

    pExynosComponent->checkTimeStamp.needSetStartTimeStamp = OMX_FALSE;
    pExynosComponent->checkTimeStamp.needCheckStartTimeStamp = OMX_FALSE;
    pExynosComponent->checkTimeStamp.startTimeStamp = 0;
    pExynosComponent->checkTimeStamp.nStartFlags = 0x0;

    pOMXComponent->EmptyThisBuffer = &JetOMX_EmptyThisBuffer;
    pOMXComponent->FillThisBuffer  = &JetOMX_FillThisBuffer;

    ret = OMX_ErrorNone;
EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_Port_Destructor(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE        *pOMXComponent = NULL;
    JETOMX_BASECOMPONENT     *pExynosComponent = NULL;
    JETOMX_BASEPORT          *pExynosPort = NULL;

    OMX_S32 countValue = 0;
    int i = 0;

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
    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (pExynosComponent->transientState == EXYNOS_OMX_TransStateLoadedToIdle) {
        pExynosComponent->abendState = OMX_TRUE;
        for (i = 0; i < ALL_PORT_NUM; i++) {
            pExynosPort = &pExynosComponent->pExynosPort[i];
            OSAL_SemaphorePost(pExynosPort->loadedResource);
        }
        OSAL_SignalWait(pExynosComponent->abendStateEvent, DEF_MAX_WAIT_TIME);
        OSAL_SignalReset(pExynosComponent->abendStateEvent);
    }

    for (i = 0; i < ALL_PORT_NUM; i++) {
        pExynosPort = &pExynosComponent->pExynosPort[i];

        OSAL_SemaphoreTerminate(pExynosPort->loadedResource);
        pExynosPort->loadedResource = NULL;
        OSAL_SemaphoreTerminate(pExynosPort->unloadedResource);
        pExynosPort->unloadedResource = NULL;
        OSAL_Free(pExynosPort->bufferStateAllocate);
        pExynosPort->bufferStateAllocate = NULL;
        OSAL_Free(pExynosPort->extendBufferHeader);
        pExynosPort->extendBufferHeader = NULL;

        OSAL_QueueTerminate(&pExynosPort->bufferQ);
    }
    OSAL_Free(pExynosComponent->pExynosPort);
    pExynosComponent->pExynosPort = NULL;
    ret = OMX_ErrorNone;
EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE Exynos_ResetDataBuffer(EXYNOS_OMX_DATABUFFER *pDataBuffer)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (pDataBuffer == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    pDataBuffer->dataValid     = OMX_FALSE;
    pDataBuffer->dataLen       = 0;
    pDataBuffer->remainDataLen = 0;
    pDataBuffer->usedDataLen   = 0;
    pDataBuffer->bufferHeader  = NULL;
    pDataBuffer->nFlags        = 0;
    pDataBuffer->timeStamp     = 0;
    pDataBuffer->pPrivate      = NULL;

EXIT:
    return ret;
}

OMX_ERRORTYPE Exynos_ResetCodecData(EXYNOS_OMX_DATA *pData)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (pData == NULL) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    pData->dataLen       = 0;
    pData->usedDataLen   = 0;
    pData->remainDataLen = 0;
    pData->nFlags        = 0;
    pData->timeStamp     = 0;
    pData->pPrivate      = NULL;
    pData->bufferHeader  = NULL;

EXIT:
    return ret;
}

OMX_ERRORTYPE Exynos_Shared_BufferToData(EXYNOS_OMX_DATABUFFER *pUseBuffer, EXYNOS_OMX_DATA *pData, EXYNOS_OMX_PLANE nPlane)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (nPlane == ONE_PLANE) {
        /* Case of Shared Buffer, Only support singlePlaneBuffer */
        pData->buffer.singlePlaneBuffer.dataBuffer = pUseBuffer->bufferHeader->pBuffer;
    } else {
        OSAL_Log(JETOMX_LOG_ERROR, "Can not support plane");
        ret = OMX_ErrorNotImplemented;
        goto EXIT;
    }

    pData->allocSize     = pUseBuffer->allocSize;
    pData->dataLen       = pUseBuffer->dataLen;
    pData->usedDataLen   = pUseBuffer->usedDataLen;
    pData->remainDataLen = pUseBuffer->remainDataLen;
    pData->timeStamp     = pUseBuffer->timeStamp;
    pData->nFlags        = pUseBuffer->nFlags;
    pData->pPrivate      = pUseBuffer->pPrivate;
    pData->bufferHeader  = pUseBuffer->bufferHeader;

EXIT:
    return ret;
}

OMX_ERRORTYPE Exynos_Shared_DataToBuffer(EXYNOS_OMX_DATA *pData, EXYNOS_OMX_DATABUFFER *pUseBuffer)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    pUseBuffer->bufferHeader          = pData->bufferHeader;
    pUseBuffer->allocSize             = pData->allocSize;
    pUseBuffer->dataLen               = pData->dataLen;
    pUseBuffer->usedDataLen           = pData->usedDataLen;
    pUseBuffer->remainDataLen         = pData->remainDataLen;
    pUseBuffer->timeStamp             = pData->timeStamp;
    pUseBuffer->nFlags                = pData->nFlags;
    pUseBuffer->pPrivate              = pData->pPrivate;

    return ret;
}
