/*
 * @file        OSAL_Semaphore.c
 * @brief
 * @author
 * @version     0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "OSAL_Memory.h"
#include "OSAL_Semaphore.h"

#undef JETOMX_LOG_TAG
#define JETOMX_LOG_TAG    "JETOMX_LOG_SEMA"
#define JETOMX_LOG_OFF
#define JETOMX_TRACE_ON
#include "OSAL_Log.h"


OMX_ERRORTYPE OSAL_SemaphoreCreate(OMX_HANDLETYPE *semaphoreHandle)
{
    sem_t *sema;

    sema = (sem_t *)OSAL_Malloc(sizeof(sem_t));
    if (!sema)
        return OMX_ErrorInsufficientResources;

    if (sem_init(sema, 0, 0) != 0) {
        OSAL_Free(sema);
        return OMX_ErrorUndefined;
    }

    *semaphoreHandle = (OMX_HANDLETYPE)sema;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OSAL_SemaphoreTerminate(OMX_HANDLETYPE semaphoreHandle)
{
    sem_t *sema = (sem_t *)semaphoreHandle;

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_destroy(sema) != 0)
        return OMX_ErrorUndefined;

    OSAL_Free(sema);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OSAL_SemaphoreWait(OMX_HANDLETYPE semaphoreHandle)
{
    sem_t *sema = (sem_t *)semaphoreHandle;

    FunctionIn();

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_wait(sema) != 0)
        return OMX_ErrorUndefined;

    FunctionOut();

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OSAL_SemaphorePost(OMX_HANDLETYPE semaphoreHandle)
{
    sem_t *sema = (sem_t *)semaphoreHandle;

    FunctionIn();

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_post(sema) != 0)
        return OMX_ErrorUndefined;

    FunctionOut();

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OSAL_Set_SemaphoreCount(OMX_HANDLETYPE semaphoreHandle, OMX_S32 val)
{
    sem_t *sema = (sem_t *)semaphoreHandle;

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_init(sema, 0, val) != 0)
        return OMX_ErrorUndefined;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OSAL_Get_SemaphoreCount(OMX_HANDLETYPE semaphoreHandle, OMX_S32 *val)
{
    sem_t *sema = (sem_t *)semaphoreHandle;
    int semaVal = 0;

    if (sema == NULL)
        return OMX_ErrorBadParameter;

    if (sem_getvalue(sema, &semaVal) != 0)
        return OMX_ErrorUndefined;

    *val = (OMX_S32)semaVal;

    return OMX_ErrorNone;
}
