/*
 * @file        OSAL_Thread.h
 * @brief
 * @author
 * @version     0.1.0
 */

#ifndef OSAL_THREAD
#define OSAL_THREAD

#include "OMX_Types.h"
#include "OMX_Core.h"


#ifdef __cplusplus
extern "C" {
#endif

OMX_ERRORTYPE OSAL_ThreadCreate(OMX_HANDLETYPE *threadHandle, OMX_PTR function_name, OMX_PTR argument);
OMX_ERRORTYPE OSAL_ThreadTerminate(OMX_HANDLETYPE threadHandle);
OMX_ERRORTYPE OSAL_ThreadCancel(OMX_HANDLETYPE threadHandle);
void          OSAL_ThreadExit(void *value_ptr);
void          OSAL_SleepMillisec(OMX_U32 ms);

#ifdef __cplusplus
}
#endif

#endif
