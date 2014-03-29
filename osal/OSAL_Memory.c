/*
 * @file        OSAL_Memory.c
 * @brief
 * @author
 * @version     0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OSAL_Memory.h"

#define EXYNOS_LOG_OFF
#include "OSAL_Log.h"


static int mem_cnt = 0;

OMX_PTR OSAL_Malloc(OMX_U32 size)
{
    mem_cnt++;
    OSAL_Log(JETOMX_LOG_TRACE, "alloc count: %d", mem_cnt);

    return (OMX_PTR)malloc(size);
}

void OSAL_Free(OMX_PTR addr)
{
    mem_cnt--;
    OSAL_Log(JETOMX_LOG_TRACE, "free count: %d", mem_cnt);

    if (addr)
        free(addr);

    return;
}

OMX_PTR OSAL_Memset(OMX_PTR dest, OMX_S32 c, OMX_S32 n)
{
    return memset(dest, c, n);
}

OMX_PTR OSAL_Memcpy(OMX_PTR dest, OMX_PTR src, OMX_S32 n)
{
    return memcpy(dest, src, n);
}

OMX_PTR OSAL_Memmove(OMX_PTR dest, OMX_PTR src, OMX_S32 n)
{
    return memmove(dest, src, n);
}
