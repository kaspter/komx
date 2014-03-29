/*
 * @file       JetOMX_Resourcemanager.c
 * @brief
 * @author
 * @version    0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JetOMX_Resourcemanager.h"
#include "JetOMX_Basecomponent.h"
#include "OSAL_Memory.h"
#include "OSAL_Mutex.h"

#undef  EXYNOS_LOG_TAG
#define EXYNOS_LOG_TAG    "EXYNOS_RM"
#define EXYNOS_LOG_OFF
#include "OSAL_Log.h"


#define MAX_RESOURCE_VIDEO_DEC 3 /* for Android */
#define MAX_RESOURCE_VIDEO_ENC 1 /* for Android */

/* Max allowable video scheduler component instance */
static JETOMX_RM_COMPONENT_LIST *gpVideoDecRMComponentList = NULL;
static JETOMX_RM_COMPONENT_LIST *gpVideoDecRMWaitingList = NULL;
static JETOMX_RM_COMPONENT_LIST *gpVideoEncRMComponentList = NULL;
static JETOMX_RM_COMPONENT_LIST *gpVideoEncRMWaitingList = NULL;
static OMX_HANDLETYPE ghVideoRMComponentListMutex = NULL;


OMX_ERRORTYPE addElementList(JETOMX_RM_COMPONENT_LIST **ppList, OMX_COMPONENTTYPE *pOMXComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    JETOMX_RM_COMPONENT_LIST *pTempComp = NULL;
    JETOMX_BASECOMPONENT     *pExynosComponent = NULL;

    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    if (*ppList != NULL) {
        pTempComp = *ppList;
        while (pTempComp->pNext != NULL) {
            pTempComp = pTempComp->pNext;
        }
        pTempComp->pNext = (JETOMX_RM_COMPONENT_LIST *)OSAL_Malloc(sizeof(JETOMX_RM_COMPONENT_LIST));
        if (pTempComp->pNext == NULL) {
            ret = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        ((JETOMX_RM_COMPONENT_LIST *)(pTempComp->pNext))->pNext = NULL;
        ((JETOMX_RM_COMPONENT_LIST *)(pTempComp->pNext))->pOMXStandComp = pOMXComponent;
        ((JETOMX_RM_COMPONENT_LIST *)(pTempComp->pNext))->groupPriority = pExynosComponent->compPriority.nGroupPriority;
        goto EXIT;
    } else {
        *ppList = (JETOMX_RM_COMPONENT_LIST *)OSAL_Malloc(sizeof(JETOMX_RM_COMPONENT_LIST));
        if (*ppList == NULL) {
            ret = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        pTempComp = *ppList;
        pTempComp->pNext = NULL;
        pTempComp->pOMXStandComp = pOMXComponent;
        pTempComp->groupPriority = pExynosComponent->compPriority.nGroupPriority;
    }

EXIT:
    return ret;
}

OMX_ERRORTYPE removeElementList(JETOMX_RM_COMPONENT_LIST **ppList, OMX_COMPONENTTYPE *pOMXComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    JETOMX_RM_COMPONENT_LIST *pCurrComp = NULL;
    JETOMX_RM_COMPONENT_LIST *pPrevComp = NULL;
    OMX_BOOL                  bDetectComp = OMX_FALSE;

    if (*ppList == NULL) {
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    pCurrComp = *ppList;
    while (pCurrComp != NULL) {
        if (pCurrComp->pOMXStandComp == pOMXComponent) {
            if (*ppList == pCurrComp) {
                *ppList = pCurrComp->pNext;
                OSAL_Free(pCurrComp);
            } else {
                if (pPrevComp != NULL)
                    pPrevComp->pNext = pCurrComp->pNext;

                OSAL_Free(pCurrComp);
            }
            bDetectComp = OMX_TRUE;
            break;
        } else {
            pPrevComp = pCurrComp;
            pCurrComp = pCurrComp->pNext;
        }
    }

    if (bDetectComp == OMX_FALSE)
        ret = OMX_ErrorComponentNotFound;
    else
        ret = OMX_ErrorNone;

EXIT:
    return ret;
}

int searchLowPriority(JETOMX_RM_COMPONENT_LIST *RMComp_list, OMX_U32 inComp_priority, JETOMX_RM_COMPONENT_LIST **outLowComp)
{
    int ret = 0;
    JETOMX_RM_COMPONENT_LIST *pTempComp = NULL;
    JETOMX_RM_COMPONENT_LIST *pCandidateComp = NULL;

    if (RMComp_list == NULL)
        ret = -1;

    pTempComp = RMComp_list;
    *outLowComp = 0;

    while (pTempComp != NULL) {
        if (pTempComp->groupPriority > inComp_priority) {
            if (pCandidateComp != NULL) {
                if (pCandidateComp->groupPriority < pTempComp->groupPriority)
                    pCandidateComp = pTempComp;
            } else {
                pCandidateComp = pTempComp;
            }
        }

        pTempComp = pTempComp->pNext;
    }

    *outLowComp = pCandidateComp;
    if (pCandidateComp == NULL)
        ret = 0;
    else
        ret = 1;

EXIT:
    return ret;
}

OMX_ERRORTYPE removeComponent(OMX_COMPONENTTYPE *pOMXComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT *pExynosComponent = NULL;

    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    if (pExynosComponent->currentState == OMX_StateIdle) {
        (*(pExynosComponent->pCallbacks->EventHandler))
            (pOMXComponent, pExynosComponent->callbackData,
            OMX_EventError, OMX_ErrorResourcesLost, 0, NULL);
        ret = OMX_SendCommand(pOMXComponent, OMX_CommandStateSet, OMX_StateLoaded, NULL);
        if (ret != OMX_ErrorNone) {
            ret = OMX_ErrorUndefined;
            goto EXIT;
        }
    } else if ((pExynosComponent->currentState == OMX_StateExecuting) || (pExynosComponent->currentState == OMX_StatePause)) {
        /* Todo */
    }

    ret = OMX_ErrorNone;

EXIT:
    return ret;
}


OMX_ERRORTYPE JetOMX_ResourceManager_Init()
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();
    ret = OSAL_MutexCreate(&ghVideoRMComponentListMutex);
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_ResourceManager_Deinit()
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    JETOMX_RM_COMPONENT_LIST *pCurrComponent;
    JETOMX_RM_COMPONENT_LIST *pNextComponent;

    FunctionIn();

    OSAL_MutexLock(ghVideoRMComponentListMutex);

    if (gpVideoDecRMComponentList) {
        pCurrComponent = gpVideoDecRMComponentList;
        while (pCurrComponent != NULL) {
            pNextComponent = pCurrComponent->pNext;
            OSAL_Free(pCurrComponent);
            pCurrComponent = pNextComponent;
        }
        gpVideoDecRMComponentList = NULL;
    }
    if (gpVideoDecRMWaitingList) {
        pCurrComponent = gpVideoDecRMWaitingList;
        while (pCurrComponent != NULL) {
            pNextComponent = pCurrComponent->pNext;
            OSAL_Free(pCurrComponent);
            pCurrComponent = pNextComponent;
        }
        gpVideoDecRMWaitingList = NULL;
    }

    if (gpVideoEncRMComponentList) {
        pCurrComponent = gpVideoEncRMComponentList;
        while (pCurrComponent != NULL) {
            pNextComponent = pCurrComponent->pNext;
            OSAL_Free(pCurrComponent);
            pCurrComponent = pNextComponent;
        }
        gpVideoEncRMComponentList = NULL;
    }
    if (gpVideoEncRMWaitingList) {
        pCurrComponent = gpVideoEncRMWaitingList;
        while (pCurrComponent != NULL) {
            pNextComponent = pCurrComponent->pNext;
            OSAL_Free(pCurrComponent);
            pCurrComponent = pNextComponent;
        }
        gpVideoEncRMWaitingList = NULL;
    }

    OSAL_MutexUnlock(ghVideoRMComponentListMutex);

    OSAL_MutexTerminate(ghVideoRMComponentListMutex);
    ghVideoRMComponentListMutex = NULL;

    ret = OMX_ErrorNone;
EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_Get_Resource(OMX_COMPONENTTYPE *pOMXComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT     *pExynosComponent = NULL;
    JETOMX_RM_COMPONENT_LIST *pComponentTemp = NULL;
    JETOMX_RM_COMPONENT_LIST *pComponentCandidate = NULL;
    int numElem = 0;
    int lowCompDetect = 0;

    FunctionIn();

    OSAL_MutexLock(ghVideoRMComponentListMutex);

    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (pExynosComponent->codecType == HW_VIDEO_DEC_CODEC) {
        pComponentTemp = gpVideoDecRMComponentList;
        if (pComponentTemp != NULL) {
            while (pComponentTemp) {
                numElem++;
                pComponentTemp = pComponentTemp->pNext;
            }
        } else {
            numElem = 0;
        }
        if (numElem >= MAX_RESOURCE_VIDEO_DEC) {
            lowCompDetect = searchLowPriority(gpVideoDecRMComponentList, pExynosComponent->compPriority.nGroupPriority, &pComponentCandidate);
            if (lowCompDetect <= 0) {
                ret = OMX_ErrorInsufficientResources;
                goto EXIT;
            } else {
                ret = removeComponent(pComponentCandidate->pOMXStandComp);
                if (ret != OMX_ErrorNone) {
                    ret = OMX_ErrorInsufficientResources;
                    goto EXIT;
                } else {
                    ret = removeElementList(&gpVideoDecRMComponentList, pComponentCandidate->pOMXStandComp);
                    ret = addElementList(&gpVideoDecRMComponentList, pOMXComponent);
                    if (ret != OMX_ErrorNone) {
                        ret = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                }
            }
        } else {
            ret = addElementList(&gpVideoDecRMComponentList, pOMXComponent);
            if (ret != OMX_ErrorNone) {
                ret = OMX_ErrorInsufficientResources;
                goto EXIT;
            }
        }
    } else if (pExynosComponent->codecType == HW_VIDEO_ENC_CODEC) {
        pComponentTemp = gpVideoEncRMComponentList;
        if (pComponentTemp != NULL) {
            while (pComponentTemp) {
                numElem++;
                pComponentTemp = pComponentTemp->pNext;
            }
        } else {
            numElem = 0;
        }
        if (numElem >= MAX_RESOURCE_VIDEO_ENC) {
            lowCompDetect = searchLowPriority(gpVideoEncRMComponentList, pExynosComponent->compPriority.nGroupPriority, &pComponentCandidate);
            if (lowCompDetect <= 0) {
                ret = OMX_ErrorInsufficientResources;
                goto EXIT;
            } else {
                ret = removeComponent(pComponentCandidate->pOMXStandComp);
                if (ret != OMX_ErrorNone) {
                    ret = OMX_ErrorInsufficientResources;
                    goto EXIT;
                } else {
                    ret = removeElementList(&gpVideoEncRMComponentList, pComponentCandidate->pOMXStandComp);
                    ret = addElementList(&gpVideoEncRMComponentList, pOMXComponent);
                    if (ret != OMX_ErrorNone) {
                        ret = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                }
            }
        } else {
            ret = addElementList(&gpVideoEncRMComponentList, pOMXComponent);
            if (ret != OMX_ErrorNone) {
                ret = OMX_ErrorInsufficientResources;
                goto EXIT;
            }
        }
    }
    ret = OMX_ErrorNone;

EXIT:

    OSAL_MutexUnlock(ghVideoRMComponentListMutex);

    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_Release_Resource(OMX_COMPONENTTYPE *pOMXComponent)
{
    OMX_ERRORTYPE                 ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT     *pExynosComponent = NULL;
    JETOMX_RM_COMPONENT_LIST *pComponentTemp = NULL;
    OMX_COMPONENTTYPE            *pOMXWaitComponent = NULL;
    int numElem = 0;

    FunctionIn();

    OSAL_MutexLock(ghVideoRMComponentListMutex);

    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;

    if (pExynosComponent->codecType == HW_VIDEO_DEC_CODEC) {
        pComponentTemp = gpVideoDecRMWaitingList;
        if (gpVideoDecRMComponentList == NULL) {
            ret = OMX_ErrorUndefined;
            goto EXIT;
        }

        ret = removeElementList(&gpVideoDecRMComponentList, pOMXComponent);
        if (ret != OMX_ErrorNone) {
            ret = OMX_ErrorUndefined;
            goto EXIT;
        }
        while (pComponentTemp) {
            numElem++;
            pComponentTemp = pComponentTemp->pNext;
        }
        if (numElem > 0) {
            pOMXWaitComponent = gpVideoDecRMWaitingList->pOMXStandComp;
            removeElementList(&gpVideoDecRMWaitingList, pOMXWaitComponent);
            ret = OMX_SendCommand(pOMXWaitComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
            if (ret != OMX_ErrorNone) {
                goto EXIT;
            }
        }
    } else if (pExynosComponent->codecType == HW_VIDEO_ENC_CODEC) {
        pComponentTemp = gpVideoEncRMWaitingList;
        if (gpVideoEncRMComponentList == NULL) {
            ret = OMX_ErrorUndefined;
            goto EXIT;
        }

        ret = removeElementList(&gpVideoEncRMComponentList, pOMXComponent);
        if (ret != OMX_ErrorNone) {
            ret = OMX_ErrorUndefined;
            goto EXIT;
        }
        while (pComponentTemp) {
            numElem++;
            pComponentTemp = pComponentTemp->pNext;
        }
        if (numElem > 0) {
            pOMXWaitComponent = gpVideoEncRMWaitingList->pOMXStandComp;
            removeElementList(&gpVideoEncRMWaitingList, pOMXWaitComponent);
            ret = OMX_SendCommand(pOMXWaitComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
            if (ret != OMX_ErrorNone) {
                goto EXIT;
            }
        }
    }

EXIT:

    OSAL_MutexUnlock(ghVideoRMComponentListMutex);

    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_In_WaitForResource(OMX_COMPONENTTYPE *pOMXComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT *pExynosComponent = NULL;

    FunctionIn();

    OSAL_MutexLock(ghVideoRMComponentListMutex);

    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    if (pExynosComponent->codecType == HW_VIDEO_DEC_CODEC)
        ret = addElementList(&gpVideoDecRMWaitingList, pOMXComponent);
    else if (pExynosComponent->codecType == HW_VIDEO_ENC_CODEC)
        ret = addElementList(&gpVideoEncRMWaitingList, pOMXComponent);

    OSAL_MutexUnlock(ghVideoRMComponentListMutex);

    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_Out_WaitForResource(OMX_COMPONENTTYPE *pOMXComponent)
{
    OMX_ERRORTYPE             ret = OMX_ErrorNone;
    JETOMX_BASECOMPONENT *pExynosComponent = NULL;

    FunctionIn();

    OSAL_MutexLock(ghVideoRMComponentListMutex);

    pExynosComponent = (JETOMX_BASECOMPONENT *)pOMXComponent->pComponentPrivate;
    if (pExynosComponent->codecType == HW_VIDEO_DEC_CODEC)
        ret = removeElementList(&gpVideoDecRMWaitingList, pOMXComponent);
    else if (pExynosComponent->codecType == HW_VIDEO_ENC_CODEC)
        ret = removeElementList(&gpVideoEncRMWaitingList, pOMXComponent);

    OSAL_MutexUnlock(ghVideoRMComponentListMutex);

    FunctionOut();

    return ret;
}
