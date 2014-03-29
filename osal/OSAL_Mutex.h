/*
 * @file        OSAL_Mutex.h
 * @brief
 * @author
 * @version     0.1.0
*/

#ifndef OSAL_MUTEX
#define OSAL_MUTEX

#include "OMX_Types.h"
#include "OMX_Core.h"


#ifdef __cplusplus
extern "C" {
#endif

OMX_ERRORTYPE OSAL_MutexCreate(OMX_HANDLETYPE *mutexHandle);
OMX_ERRORTYPE OSAL_MutexTerminate(OMX_HANDLETYPE mutexHandle);
OMX_ERRORTYPE OSAL_MutexLock(OMX_HANDLETYPE mutexHandle);
OMX_ERRORTYPE OSAL_MutexUnlock(OMX_HANDLETYPE mutexHandle);

#ifdef __cplusplus
}
#endif

#endif
