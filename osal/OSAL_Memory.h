/*
 * @file        OSAL_Memory.h
 * @brief
 * @author
 * @version     0.1.0
 */

#ifndef OSAL_MEMORY
#define OSAL_MEMORY

#include "OMX_Types.h"


#ifdef __cplusplus
extern "C" {
#endif

OMX_PTR OSAL_Malloc(OMX_U32 size);
void    OSAL_Free(OMX_PTR addr);
OMX_PTR OSAL_Memset(OMX_PTR dest, OMX_S32 c, OMX_S32 n);
OMX_PTR OSAL_Memcpy(OMX_PTR dest, OMX_PTR src, OMX_S32 n);
OMX_PTR OSAL_Memmove(OMX_PTR dest, OMX_PTR src, OMX_S32 n);

#ifdef __cplusplus
}
#endif

#endif
