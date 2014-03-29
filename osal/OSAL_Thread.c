/*
 * @file        OSAL_Thread.c
 * @brief
 * @author
 * @version     0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "OSAL_Memory.h"
#include "OSAL_Thread.h"

#undef JETOMX_LOG_TAG
#define JETOMX_LOG_TAG    "JETOMX_LOG_THREAD"
#define JETOMX_LOG_OFF
#define JETOMX_TRACE_ON
#include "OSAL_Log.h"


typedef struct _THREAD_HANDLE_TYPE
{
    pthread_t          pthread;
    pthread_attr_t     attr;
    struct sched_param schedparam;
    int                stack_size;
} THREAD_HANDLE_TYPE;


OMX_ERRORTYPE OSAL_ThreadCreate(OMX_HANDLETYPE *threadHandle, OMX_PTR function_name, OMX_PTR argument)
{
    FunctionIn();

    int result = 0;
    int detach_ret = 0;
    THREAD_HANDLE_TYPE *thread;
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    thread = OSAL_Malloc(sizeof(THREAD_HANDLE_TYPE));
    OSAL_Memset(thread, 0, sizeof(THREAD_HANDLE_TYPE));

    pthread_attr_init(&thread->attr);
    if (thread->stack_size != 0)
        pthread_attr_setstacksize(&thread->attr, thread->stack_size);

    /* set priority */
    if (thread->schedparam.sched_priority != 0)
        pthread_attr_setschedparam(&thread->attr, &thread->schedparam);

    detach_ret = pthread_attr_setdetachstate(&thread->attr, PTHREAD_CREATE_JOINABLE);
    if (detach_ret != 0) {
        OSAL_Free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    result = pthread_create(&thread->pthread, &thread->attr, function_name, (void *)argument);
    /* pthread_setschedparam(thread->pthread, SCHED_RR, &thread->schedparam); */

    switch (result) {
    case 0:
        *threadHandle = (OMX_HANDLETYPE)thread;
        ret = OMX_ErrorNone;
        break;
    case EAGAIN:
        OSAL_Free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorInsufficientResources;
        break;
    default:
        OSAL_Free(thread);
        *threadHandle = NULL;
        ret = OMX_ErrorUndefined;
        break;
    }

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE OSAL_ThreadTerminate(OMX_HANDLETYPE threadHandle)
{
    FunctionIn();

    OMX_ERRORTYPE ret = OMX_ErrorNone;
    THREAD_HANDLE_TYPE *thread = (THREAD_HANDLE_TYPE *)threadHandle;

    if (!thread) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    if (pthread_join(thread->pthread, NULL) != 0) {
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    OSAL_Free(thread);
    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE OSAL_ThreadCancel(OMX_HANDLETYPE threadHandle)
{
    THREAD_HANDLE_TYPE *thread = (THREAD_HANDLE_TYPE *)threadHandle;

    if (!thread)
        return OMX_ErrorBadParameter;

    /* thread_cancel(thread->pthread); */
    pthread_exit(&thread->pthread);
    pthread_join(thread->pthread, NULL);

    OSAL_Free(thread);
    return OMX_ErrorNone;
}

void OSAL_ThreadExit(void *value_ptr)
{
    pthread_exit(value_ptr);
    return;
}

void OSAL_SleepMillisec(OMX_U32 ms)
{
    usleep(ms * 1000);
    return;
}
