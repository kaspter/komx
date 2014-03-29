/*
 * @file        OSAL_Event.h
 * @brief
 * @author
 * @version     0.1.0
 */

#ifndef Exynos_OSAL_EVENT
#define Exynos_OSAL_EVENT

#include <pthread.h>
#include "OMX_Types.h"
#include "OMX_Core.h"


#define DEF_MAX_WAIT_TIME 0xFFFFFFFF

typedef struct _Exynos_OSAL_THREADEVENT
{
    OMX_BOOL       signal;
    OMX_HANDLETYPE mutex;
    pthread_cond_t condition;
} Exynos_OSAL_THREADEVENT;


#ifdef __cplusplus
extern "C" {
#endif


OMX_ERRORTYPE OSAL_SignalCreate(OMX_HANDLETYPE *eventHandle);
OMX_ERRORTYPE OSAL_SignalTerminate(OMX_HANDLETYPE eventHandle);
OMX_ERRORTYPE OSAL_SignalReset(OMX_HANDLETYPE eventHandle);
OMX_ERRORTYPE OSAL_SignalSet(OMX_HANDLETYPE eventHandle);
OMX_ERRORTYPE OSAL_SignalWait(OMX_HANDLETYPE eventHandle, OMX_U32 ms);


#ifdef __cplusplus
}
#endif

#endif
