/*
 * @file        OSAL_Semaphore.h
 * @brief
 * @author
 * @version     0.1.0
 */

#ifndef OSAL_SEMAPHORE
#define OSAL_SEMAPHORE

#include "OMX_Types.h"
#include "OMX_Core.h"


#ifdef __cplusplus
extern "C" {
#endif

OMX_ERRORTYPE OSAL_SemaphoreCreate(OMX_HANDLETYPE *semaphoreHandle);
OMX_ERRORTYPE OSAL_SemaphoreTerminate(OMX_HANDLETYPE semaphoreHandle);
OMX_ERRORTYPE OSAL_SemaphoreWait(OMX_HANDLETYPE semaphoreHandle);
OMX_ERRORTYPE OSAL_SemaphorePost(OMX_HANDLETYPE semaphoreHandle);
OMX_ERRORTYPE OSAL_Set_SemaphoreCount(OMX_HANDLETYPE semaphoreHandle, OMX_S32 val);
OMX_ERRORTYPE OSAL_Get_SemaphoreCount(OMX_HANDLETYPE semaphoreHandle, OMX_S32 *val);

#ifdef __cplusplus
}
#endif

#endif
