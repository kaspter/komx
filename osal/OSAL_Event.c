/*
 * @file        OSAL_Event.c
 * @brief
 * @author
 * @version     0.1.0
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include "OSAL_Memory.h"
#include "OSAL_Mutex.h"
#include "OSAL_Event.h"

#undef  EXYNOS_LOG_TAG
#define EXYNOS_LOG_TAG    "Exynos_OSAL_EVENT"
#define EXYNOS_LOG_OFF
#include "OSAL_Log.h"


OMX_ERRORTYPE OSAL_SignalCreate(OMX_HANDLETYPE *eventHandle)
{
    Exynos_OSAL_THREADEVENT *event;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    event = (Exynos_OSAL_THREADEVENT *)OSAL_Malloc(sizeof(Exynos_OSAL_THREADEVENT));
    if (!event) {
        ret = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    OSAL_Memset(event, 0, sizeof(Exynos_OSAL_THREADEVENT));
    event->signal = OMX_FALSE;

    ret = OSAL_MutexCreate(&event->mutex);
    if (ret != OMX_ErrorNone) {
        OSAL_Free(event);
        goto EXIT;
    }

    if (pthread_cond_init(&event->condition, NULL)) {
        OSAL_MutexTerminate(event->mutex);
        OSAL_Free(event);
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    *eventHandle = (OMX_HANDLETYPE)event;
    ret = OMX_ErrorNone;

EXIT:
    return ret;
}

OMX_ERRORTYPE OSAL_SignalTerminate(OMX_HANDLETYPE eventHandle)
{
    Exynos_OSAL_THREADEVENT *event = (Exynos_OSAL_THREADEVENT *)eventHandle;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (!event) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    ret = OSAL_MutexLock(event->mutex);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (pthread_cond_destroy(&event->condition)) {
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    ret = OSAL_MutexUnlock(event->mutex);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    ret = OSAL_MutexTerminate(event->mutex);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    OSAL_Free(event);

EXIT:
    return ret;
}

OMX_ERRORTYPE OSAL_SignalReset(OMX_HANDLETYPE eventHandle)
{
    Exynos_OSAL_THREADEVENT *event = (Exynos_OSAL_THREADEVENT *)eventHandle;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (!event) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    ret = OSAL_MutexLock(event->mutex);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    event->signal = OMX_FALSE;

    OSAL_MutexUnlock(event->mutex);

EXIT:
    return ret;
}

OMX_ERRORTYPE OSAL_SignalSet(OMX_HANDLETYPE eventHandle)
{
    Exynos_OSAL_THREADEVENT *event = (Exynos_OSAL_THREADEVENT *)eventHandle;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if (!event) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    ret = OSAL_MutexLock(event->mutex);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    event->signal = OMX_TRUE;
    pthread_cond_signal(&event->condition);

    OSAL_MutexUnlock(event->mutex);

EXIT:
    return ret;
}

OMX_ERRORTYPE OSAL_SignalWait(OMX_HANDLETYPE eventHandle, OMX_U32 ms)
{
    Exynos_OSAL_THREADEVENT *event = (Exynos_OSAL_THREADEVENT *)eventHandle;
    OMX_ERRORTYPE         ret = OMX_ErrorNone;
    struct timespec       timeout;
    struct timeval        now;
    int                   funcret = 0;
    OMX_U32               tv_us;

    FunctionIn();

    if (!event) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    gettimeofday(&now, NULL);

    tv_us = now.tv_usec + ms * 1000;
    timeout.tv_sec = now.tv_sec + tv_us / 1000000;
    timeout.tv_nsec = (tv_us % 1000000) * 1000;

    ret = OSAL_MutexLock(event->mutex);
    if (ret != OMX_ErrorNone) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (ms == 0) {
        if (!event->signal)
            ret = OMX_ErrorTimeout;
    } else if (ms == DEF_MAX_WAIT_TIME) {
        while (!event->signal)
            pthread_cond_wait(&event->condition, (pthread_mutex_t *)(event->mutex));
        ret = OMX_ErrorNone;
    } else {
        while (!event->signal) {
            funcret = pthread_cond_timedwait(&event->condition, (pthread_mutex_t *)(event->mutex), &timeout);
            if ((!event->signal) && (funcret == ETIMEDOUT)) {
                ret = OMX_ErrorTimeout;
                break;
            }
        }
    }

    OSAL_MutexUnlock(event->mutex);

EXIT:
    FunctionOut();

    return ret;
}
