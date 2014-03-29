/*
 * @file       JetOMX_Resourcemanager.h
 * @brief
 * @author
 * @version    0.1.0
 */

#ifndef JETOMX_RESOURCEMANAGER
#define JETOMX_RESOURCEMANAGER


#include "JetOMX_Def.h"
#include "OMX_Component.h"


struct JETOMX_RM_COMPONENT_LIST;
typedef struct _JETOMX_RM_COMPONENT_LIST
{
    OMX_COMPONENTTYPE         *pOMXStandComp;
    OMX_U32                    groupPriority;
    struct _JETOMX_RM_COMPONENT_LIST *pNext;
} JETOMX_RM_COMPONENT_LIST;


#ifdef __cplusplus
extern "C" {
#endif

OMX_ERRORTYPE JetOMX_ResourceManager_Init();
OMX_ERRORTYPE JetOMX_ResourceManager_Deinit();
OMX_ERRORTYPE JetOMX_Get_Resource(OMX_COMPONENTTYPE *pOMXComponent);
OMX_ERRORTYPE JetOMX_Release_Resource(OMX_COMPONENTTYPE *pOMXComponent);
OMX_ERRORTYPE JetOMX_In_WaitForResource(OMX_COMPONENTTYPE *pOMXComponent);
OMX_ERRORTYPE JetOMX_Out_WaitForResource(OMX_COMPONENTTYPE *pOMXComponent);

#ifdef __cplusplus
};
#endif

#endif
