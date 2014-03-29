/*
 * @file        OSAL_Mutex.c
 * @brief
 * @author
 * @version     0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "OSAL_Memory.h"
#include "OSAL_Mutex.h"

OMX_ERRORTYPE OSAL_MutexCreate(OMX_HANDLETYPE *mutexHandle)
{
    pthread_mutex_t *mutex;

    mutex = (pthread_mutex_t *)OSAL_Malloc(sizeof(pthread_mutex_t));
    if (!mutex)
        return OMX_ErrorInsufficientResources;

    if (pthread_mutex_init(mutex, NULL) != 0) {
        OSAL_Free(mutex);
        return OMX_ErrorUndefined;
    }

    *mutexHandle = (OMX_HANDLETYPE)mutex;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OSAL_MutexTerminate(OMX_HANDLETYPE mutexHandle)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)mutexHandle;

    if (mutex == NULL)
        return OMX_ErrorBadParameter;

    if (pthread_mutex_destroy(mutex) != 0)
        return OMX_ErrorUndefined;

    OSAL_Free(mutex);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OSAL_MutexLock(OMX_HANDLETYPE mutexHandle)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)mutexHandle;

    if (mutex == NULL)
        return OMX_ErrorBadParameter;

    if (pthread_mutex_lock(mutex) != 0)
        return OMX_ErrorUndefined;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OSAL_MutexUnlock(OMX_HANDLETYPE mutexHandle)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)mutexHandle;

    if (mutex == NULL)
        return OMX_ErrorBadParameter;

    if (pthread_mutex_unlock(mutex) != 0)
        return OMX_ErrorUndefined;

    return OMX_ErrorNone;
}
