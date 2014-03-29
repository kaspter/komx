/*
 * @file        OSAL_Queue.h
 * @brief
 * @author
 * @version     0.1.0
 */

#ifndef EXYNOS_OSAL_QUEUE
#define EXYNOS_OSAL_QUEUE

#include "OMX_Types.h"
#include "OMX_Core.h"

#define QUEUE_ELEMENTS        10
#define MAX_QUEUE_ELEMENTS    40

typedef struct _OSAL_QElem
{
    void             *data;
    struct _OSAL_QElem *qNext;
} OSAL_QElem;

typedef struct _OSAL_QUEUE
{
    OSAL_QElem     *first;
    OSAL_QElem     *last;
    int            numElem;
    int            maxNumElem;
    OMX_HANDLETYPE qMutex;
} OSAL_QUEUE;


#ifdef __cplusplus
extern "C" {
#endif

OMX_ERRORTYPE OSAL_QueueCreate(OSAL_QUEUE *queueHandle, int maxNumElem);
OMX_ERRORTYPE OSAL_QueueTerminate(OSAL_QUEUE *queueHandle);
int           OSAL_Queue(OSAL_QUEUE *queueHandle, void *data);
void         *OSAL_Dequeue(OSAL_QUEUE *queueHandle);
int           OSAL_GetElemNum(OSAL_QUEUE *queueHandle);
int           OSAL_SetElemNum(OSAL_QUEUE *queueHandle, int ElemNum);
int           OSAL_ResetQueue(OSAL_QUEUE *queueHandle);

#ifdef __cplusplus
}
#endif

#endif
