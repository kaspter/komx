/*
 * @file       JetOMX_Core.c
 * @brief      Jetlab OpenMAX IL Core
 * @author
 * @version    0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JetOMX_Core.h"
#include "JetOMX_Component_Register.h"
#include "OSAL_Memory.h"
#include "OSAL_Mutex.h"
#include "OSAL_ETC.h"
#include "JetOMX_Resourcemanager.h"

#undef  JETOMX_LOG_TAG
#define JETOMX_LOG_TAG    "JETOMX_CORE"
#define JETOMX_LOG_OFF
#define JETOMX_TRACE_ON
#include "OSAL_Log.h"


static int gInitialized = 0;
static OMX_U32 gComponentNum = 0;

static JETOMX_COMPONENT_REGLIST *gComponentList = NULL;
static JETOMX_COMPONENT *gLoadComponentList = NULL;
static OMX_HANDLETYPE ghLoadComponentListMutex = NULL;


/******************************Public*Routine******************************\
* OMX_Init()
*
* Description:This method will initialize the OMX Core.  It is the
* responsibility of the application to call OMX_Init to ensure the proper
* set up of core resources.
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
\**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_Init(void)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();

    if (gInitialized == 0) {
        if (JetOMX_Component_Register(&gComponentList, &gComponentNum)) {
            ret = OMX_ErrorInsufficientResources;
            OSAL_Log(JETOMX_LOG_ERROR, "JetOMX_Init : %s", "OMX_ErrorInsufficientResources");
            goto EXIT;
        }

        ret = JetOMX_ResourceManager_Init();
        if (OMX_ErrorNone != ret) {
            OSAL_Log(JETOMX_LOG_ERROR, "JetOMX_Init : JetOMX_ResourceManager_Init failed");
            goto EXIT;
        }

        ret = OSAL_MutexCreate(&ghLoadComponentListMutex);
        if (OMX_ErrorNone != ret) {
            OSAL_Log(JETOMX_LOG_ERROR, "JetOMX_Init : OSAL_MutexCreate(&ghLoadComponentListMutex) failed");
            goto EXIT;
        }

        gInitialized = 1;
        OSAL_Log(JETOMX_LOG_TRACE, "JetOMX_Init : %s", "OMX_ErrorNone");
    }

EXIT:
    FunctionOut();

    return ret;
}

/******************************Public*Routine******************************\
* OMX_DeInit()
*
* Description:This method will release the resources of the OMX Core.
* It is the responsibility of the application to call OMX_DeInit to
* ensure the clean up of these resources.
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
\**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_Deinit(void)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();

    OSAL_MutexTerminate(ghLoadComponentListMutex);
    ghLoadComponentListMutex = NULL;

    JetOMX_ResourceManager_Deinit();

    if (OMX_ErrorNone != JetOMX_Component_Unregister(gComponentList)) {
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }
    gComponentList = NULL;
    gComponentNum = 0;
    gInitialized = 0;

EXIT:
    FunctionOut();

    return ret;
}

/*************************************************************************
* OMX_ComponentNameEnum()
*
* Description: This method will provide the name of the component at the given nIndex
*
*Parameters:
* @param[out] cComponentName       The name of the component at nIndex
* @param[in]  nNameLength          The length of the component name
* @param[in]  nIndex               The index number of the component
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_ComponentNameEnum(
    OMX_OUT OMX_STRING cComponentName,
    OMX_IN  OMX_U32 nNameLength,
    OMX_IN  OMX_U32 nIndex)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    FunctionIn();

    if (nIndex >= gComponentNum) {
        ret = OMX_ErrorNoMore;
        goto EXIT;
    }

    snprintf(cComponentName, nNameLength, "%s", gComponentList[nIndex].component.componentName);
    ret = OMX_ErrorNone;

EXIT:
    FunctionOut();

    return ret;
}

/******************************Public*Routine******************************\
* OMX_GetHandle
*
* Description: This method will create the handle of the COMPONENTTYPE
* If the component is currently loaded, this method will reutrn the
* hadle of existingcomponent or create a new instance of the component.
* It will call the JetOMX_ComponentLoad function and then the setcallback
* method to initialize the callback functions
* Parameters:
* @param[out] pHandle            Handle of the loaded components
* @param[in]  cComponentName     Name of the component to load
* @param[in]  pAppData           Used to identify the callbacks of component
* @param[in]  pCallBacks         Application callbacks
*
* @retval OMX_ErrorUndefined
* @retval OMX_ErrorInvalidComponentName
* @retval OMX_ErrorInvalidComponent
* @retval OMX_ErrorInsufficientResources
* @retval OMX_NOERROR                      Successful
*
* Note
*
\**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_GetHandle(
    OMX_OUT OMX_HANDLETYPE *pHandle,
    OMX_IN  OMX_STRING cComponentName,
    OMX_IN  OMX_PTR pAppData,
    OMX_IN  OMX_CALLBACKTYPE *pCallBacks)
{
    OMX_ERRORTYPE    ret = OMX_ErrorNone;
    JETOMX_COMPONENT *loadComponent;
    JETOMX_COMPONENT *currentComponent;
    unsigned int i = 0;

    FunctionIn();

    if (gInitialized != 1) {
        ret = OMX_ErrorNotReady;
        goto EXIT;
    }

    if ((pHandle == NULL) || (cComponentName == NULL) || (pCallBacks == NULL)) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }
    OSAL_Log(JETOMX_LOG_TRACE, "ComponentName : %s", cComponentName);

    for (i = 0; i < gComponentNum; i++) {
        if (OSAL_Strcmp(cComponentName, gComponentList[i].component.componentName) == 0) {
            loadComponent = OSAL_Malloc(sizeof(JETOMX_COMPONENT));
            OSAL_Memset(loadComponent, 0, sizeof(JETOMX_COMPONENT));

            OSAL_Strcpy(loadComponent->libName, gComponentList[i].libName);
            OSAL_Strcpy(loadComponent->componentName, gComponentList[i].component.componentName);
            ret = JetOMX_ComponentLoad(loadComponent);
            if (ret != OMX_ErrorNone) {
                OSAL_Free(loadComponent);
                OSAL_Log(JETOMX_LOG_ERROR, "OMX_Error, Line:%d", __LINE__);
                goto EXIT;
            }

            ret = loadComponent->pOMXComponent->SetCallbacks(loadComponent->pOMXComponent, pCallBacks, pAppData);
            if (ret != OMX_ErrorNone) {
                JetOMX_ComponentUnload(loadComponent);
                OSAL_Free(loadComponent);
                OSAL_Log(JETOMX_LOG_ERROR, "OMX_Error, Line:%d", __LINE__);
                goto EXIT;
            }

            OSAL_MutexLock(ghLoadComponentListMutex);
            if (gLoadComponentList == NULL) {
                gLoadComponentList = loadComponent;
            } else {
                currentComponent = gLoadComponentList;
                while (currentComponent->nextOMXComp != NULL) {
                    currentComponent = currentComponent->nextOMXComp;
                }
                currentComponent->nextOMXComp = loadComponent;
            }
            OSAL_MutexUnlock(ghLoadComponentListMutex);

            *pHandle = loadComponent->pOMXComponent;
            ret = OMX_ErrorNone;
            OSAL_Log(JETOMX_LOG_TRACE, "JetOMX_GetHandle : %s", "OMX_ErrorNone");
            goto EXIT;
        }
    }

    ret = OMX_ErrorComponentNotFound;

EXIT:
    FunctionOut();

    return ret;
}

/******************************Public*Routine******************************\
* OMX_FreeHandle()
*
* Description:This method will unload the OMX component pointed by
* OMX_HANDLETYPE. It is the responsibility of the calling method to ensure
* that the Deinit method of the component has been called prior to
* unloading component
*
* Parameters:
* @param[in] hComponent the component to unload
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
\**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE    ret = OMX_ErrorNone;
    JETOMX_COMPONENT *currentComponent;
    JETOMX_COMPONENT *deleteComponent;

    FunctionIn();

    if (gInitialized != 1) {
        ret = OMX_ErrorNotReady;
        goto EXIT;
    }

    if (!hComponent) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    OSAL_MutexLock(ghLoadComponentListMutex);
    currentComponent = gLoadComponentList;
    if (gLoadComponentList->pOMXComponent == hComponent) {
        deleteComponent = gLoadComponentList;
        gLoadComponentList = gLoadComponentList->nextOMXComp;
    } else {
        while ((currentComponent != NULL) && (((JETOMX_COMPONENT *)(currentComponent->nextOMXComp))->pOMXComponent != hComponent))
            currentComponent = currentComponent->nextOMXComp;

        if (((JETOMX_COMPONENT *)(currentComponent->nextOMXComp))->pOMXComponent == hComponent) {
            deleteComponent = currentComponent->nextOMXComp;
            currentComponent->nextOMXComp = deleteComponent->nextOMXComp;
        } else if (currentComponent == NULL) {
            ret = OMX_ErrorComponentNotFound;
            OSAL_MutexUnlock(ghLoadComponentListMutex);
            goto EXIT;
        }
    }
    OSAL_MutexUnlock(ghLoadComponentListMutex);

    JetOMX_ComponentUnload(deleteComponent);
    OSAL_Free(deleteComponent);

EXIT:
    FunctionOut();

    return ret;
}

/*************************************************************************
* OMX_SetupTunnel()
*
* Description: Setup the specified tunnel the two components
*
* Parameters:
* @param[in] hOutput     Handle of the component to be accessed
* @param[in] nPortOutput Source port used in the tunnel
* @param[in] hInput      Component to setup the tunnel with.
* @param[in] nPortInput  Destination port used in the tunnel
*
* Returns:    OMX_NOERROR          Successful
*
* Note
*
**************************************************************************/
OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_SetupTunnel(
    OMX_IN OMX_HANDLETYPE hOutput,
    OMX_IN OMX_U32 nPortOutput,
    OMX_IN OMX_HANDLETYPE hInput,
    OMX_IN OMX_U32 nPortInput)
{
    OMX_ERRORTYPE ret = OMX_ErrorNotImplemented;

EXIT:
    return ret;
}

OMX_API OMX_ERRORTYPE JetOMX_GetContentPipe(
    OMX_OUT OMX_HANDLETYPE *hPipe,
    OMX_IN  OMX_STRING szURI)
{
    OMX_ERRORTYPE ret = OMX_ErrorNotImplemented;

EXIT:
    return ret;
}

OMX_API OMX_ERRORTYPE JetOMX_GetComponentsOfRole (
    OMX_IN    OMX_STRING role,
    OMX_INOUT OMX_U32 *pNumComps,
    OMX_INOUT OMX_U8  **compNames)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    int           max_role_num = 0;
    OMX_STRING    RoleString[MAX_OMX_COMPONENT_ROLE_SIZE];
    int i = 0, j = 0;

    FunctionIn();

    if (gInitialized != 1) {
        ret = OMX_ErrorNotReady;
        goto EXIT;
    }

    *pNumComps = 0;

    for (i = 0; i < MAX_OMX_COMPONENT_NUM; i++) {
        max_role_num = gComponentList[i].component.totalRoleNum;

        for (j = 0; j < max_role_num; j++) {
            if (OSAL_Strcmp(gComponentList[i].component.roles[j], role) == 0) {
                if (compNames != NULL) {
                    OSAL_Strcpy((OMX_STRING)compNames[*pNumComps], gComponentList[i].component.componentName);
                }
                *pNumComps = (*pNumComps + 1);
            }
        }
    }

EXIT:
    FunctionOut();

    return ret;
}

OMX_API OMX_ERRORTYPE JetOMX_GetRolesOfComponent (
    OMX_IN    OMX_STRING compName,
    OMX_INOUT OMX_U32 *pNumRoles,
    OMX_OUT   OMX_U8 **roles)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_BOOL      detectComp = OMX_FALSE;
    int           compNum = 0, totalRoleNum = 0;
    int i = 0;

    FunctionIn();

    if (gInitialized != 1) {
        ret = OMX_ErrorNotReady;
        goto EXIT;
    }

    for (i = 0; i < MAX_OMX_COMPONENT_NUM; i++) {
        if (gComponentList != NULL) {
            if (OSAL_Strcmp(gComponentList[i].component.componentName, compName) == 0) {
                *pNumRoles = totalRoleNum = gComponentList[i].component.totalRoleNum;
                compNum = i;
                detectComp = OMX_TRUE;
                break;
            }
        } else {
            ret = OMX_ErrorUndefined;
            goto EXIT;
        }
    }

    if (detectComp == OMX_FALSE) {
        *pNumRoles = 0;
        ret = OMX_ErrorComponentNotFound;
        goto EXIT;
    }

    if (roles != NULL) {
        for (i = 0; i < totalRoleNum; i++) {
            OSAL_Strcpy(roles[i], gComponentList[compNum].component.roles[i]);
        }
    }

EXIT:
    FunctionOut();

    return ret;
}

OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_GetCoreInterface(
    OMX_OUT void ** ppItf,
    OMX_IN OMX_STRING cExtensionName)
{
    OMX_ERRORTYPE ret = OMX_ErrorNotImplemented;

EXIT:
    return ret;
}

OMX_API void OMX_APIENTRY JetOMX_FreeCoreInterface(
    OMX_IN void * pItf)
{
    OMX_ERRORTYPE ret = OMX_ErrorNotImplemented;

EXIT:
    return ret;
}
