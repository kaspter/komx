/*
 * @file       JetOMX_Component_Register.c
 * @brief      Jetlab OpenMAX IL Component Register
 * @author
 * @version    0.1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <dirent.h>

#include "OMX_Component.h"
#include "OSAL_Memory.h"
#include "OSAL_ETC.h"
#include "OSAL_Library.h"
#include "JetOMX_Component_Register.h"
#include "JetOMX_Macros.h"

#undef  JETOMX_LOG_TAG
#define JETOMX_LOG_TAG    "JETOMX_COMP_REGS"
#define JETOMX_LOG_OFF
#include "OSAL_Log.h"


#define JETOMX_VENDOR		"libOMX.JetLab."

OMX_ERRORTYPE JetOMX_Component_Register(JETOMX_COMPONENT_REGLIST **compList, OMX_U32 *compNum)
{
    OMX_ERRORTYPE  ret = OMX_ErrorNone;
    int            componentNum = 0, totalCompNum = 0;
    char          *libName;
    const char    *errorMsg;
    DIR           *dir;
    struct dirent *d;

    int (*JetOMX_COMPONENT_Library_Register)(RegisterComponentType **jetomxComponents);
    RegisterComponentType **ComponentsTemp;
    JETOMX_COMPONENT_REGLIST *componentList;

    FunctionIn();

    dir = opendir(JETOMX_INSTALL_PATH);
    if (dir == NULL) {
        ret = OMX_ErrorUndefined;
        goto EXIT;
    }

    componentList = (JETOMX_COMPONENT_REGLIST *)OSAL_Malloc(sizeof(JETOMX_COMPONENT_REGLIST) * MAX_OMX_COMPONENT_NUM);
    OSAL_Memset(componentList, 0, sizeof(JETOMX_COMPONENT_REGLIST) * MAX_OMX_COMPONENT_NUM);
    libName = OSAL_Malloc(MAX_OMX_COMPONENT_LIBNAME_SIZE);

    while ((d = readdir(dir)) != NULL) {
        OMX_HANDLETYPE soHandle;
        OSAL_Log(JETOMX_LOG_TRACE, "%s", d->d_name);

        if (OSAL_Strncmp(d->d_name, JETOMX_VENDOR, OSAL_Strlen(JETOMX_VENDOR)) == 0) {
            OSAL_Memset(libName, 0, MAX_OMX_COMPONENT_LIBNAME_SIZE);
            OSAL_Strcpy(libName, JETOMX_INSTALL_PATH);
            OSAL_Strcat(libName, d->d_name);
            OSAL_Log(JETOMX_LOG_TRACE, "Path & libName : %s", libName);
            if ((soHandle = OSAL_dlopen(libName, RTLD_NOW)) != NULL) {
                OSAL_dlerror();    /* clear error*/
                if ((JetOMX_COMPONENT_Library_Register = OSAL_dlsym(soHandle, "JetOMX_COMPONENT_Library_Register")) != NULL) {
                    int i = 0;
                    unsigned int j = 0;

                    componentNum = (*JetOMX_COMPONENT_Library_Register)(NULL);
                    ComponentsTemp = (RegisterComponentType **)OSAL_Malloc(sizeof(RegisterComponentType*) * componentNum);
                    for (i = 0; i < componentNum; i++) {
                        ComponentsTemp[i] = OSAL_Malloc(sizeof(RegisterComponentType));
                        OSAL_Memset(ComponentsTemp[i], 0, sizeof(RegisterComponentType));
                    }
                    (*JetOMX_COMPONENT_Library_Register)(ComponentsTemp);

                    for (i = 0; i < componentNum; i++) {
                        OSAL_Strcpy(componentList[totalCompNum].component.componentName, ComponentsTemp[i]->componentName);
                        for (j = 0; j < ComponentsTemp[i]->totalRoleNum; j++)
                            OSAL_Strcpy(componentList[totalCompNum].component.roles[j], ComponentsTemp[i]->roles[j]);
                        componentList[totalCompNum].component.totalRoleNum = ComponentsTemp[i]->totalRoleNum;

                        OSAL_Strcpy(componentList[totalCompNum].libName, libName);

                        totalCompNum++;
                    }
                    for (i = 0; i < componentNum; i++) {
                        OSAL_Free(ComponentsTemp[i]);
                    }

                    OSAL_Free(ComponentsTemp);
                } else {
                    if ((errorMsg = OSAL_dlerror()) != NULL)
                        OSAL_Log(JETOMX_LOG_WARNING, "dlsym failed: %s", errorMsg);
                }
                OSAL_dlclose(soHandle);
            } else {
                OSAL_Log(JETOMX_LOG_WARNING, "dlopen failed: %s", OSAL_dlerror());
            }
        } else {
            /* not a component name line. skip */
            continue;
        }
    }

    OSAL_Free(libName);

    closedir(dir);

    *compList = componentList;
    *compNum = totalCompNum;

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_Component_Unregister(JETOMX_COMPONENT_REGLIST *componentList)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    OSAL_Free(componentList);

EXIT:
    return ret;
}

OMX_ERRORTYPE JetOMX_ComponentAPICheck(OMX_COMPONENTTYPE *component)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;

    if ((NULL == component->GetComponentVersion)    ||
        (NULL == component->SendCommand)            ||
        (NULL == component->GetParameter)           ||
        (NULL == component->SetParameter)           ||
        (NULL == component->GetConfig)              ||
        (NULL == component->SetConfig)              ||
        (NULL == component->GetExtensionIndex)      ||
        (NULL == component->GetState)               ||
        (NULL == component->ComponentTunnelRequest) ||
        (NULL == component->UseBuffer)              ||
        (NULL == component->AllocateBuffer)         ||
        (NULL == component->FreeBuffer)             ||
        (NULL == component->EmptyThisBuffer)        ||
        (NULL == component->FillThisBuffer)         ||
        (NULL == component->SetCallbacks)           ||
        (NULL == component->ComponentDeInit)        ||
        (NULL == component->UseEGLImage)            ||
        (NULL == component->ComponentRoleEnum))
        ret = OMX_ErrorInvalidComponent;
    else
        ret = OMX_ErrorNone;

    return ret;
}

OMX_ERRORTYPE JetOMX_ComponentLoad(JETOMX_COMPONENT *jetomx_component)
{
    OMX_ERRORTYPE      ret = OMX_ErrorNone;
    OMX_HANDLETYPE     libHandle;
    OMX_COMPONENTTYPE *pOMXComponent;

    FunctionIn();

    OMX_ERRORTYPE (*JetOMX_ComponentInit)(OMX_HANDLETYPE hComponent, OMX_STRING componentName);

    libHandle = OSAL_dlopen((OMX_STRING)jetomx_component->libName, RTLD_NOW);
    if (!libHandle) {
        ret = OMX_ErrorInvalidComponentName;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInvalidComponentName, Line:%d", __LINE__);
        goto EXIT;
    }

    JetOMX_ComponentInit = OSAL_dlsym(libHandle, "JetOMX_ComponentInit");
    if (!JetOMX_ComponentInit) {
        OSAL_dlclose(libHandle);
        ret = OMX_ErrorInvalidComponent;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInvalidComponent, Line:%d", __LINE__);
        goto EXIT;
    }

    pOMXComponent = (OMX_COMPONENTTYPE *)OSAL_Malloc(sizeof(OMX_COMPONENTTYPE));
    INIT_SET_SIZE_VERSION(pOMXComponent, OMX_COMPONENTTYPE);
    ret = (*JetOMX_ComponentInit)((OMX_HANDLETYPE)pOMXComponent, (OMX_STRING)jetomx_component->componentName);
    if (ret != OMX_ErrorNone) {
        OSAL_Free(pOMXComponent);
        OSAL_dlclose(libHandle);
        ret = OMX_ErrorInvalidComponent;
        OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInvalidComponent, Line:%d", __LINE__);
        goto EXIT;
    } else {
        if (JetOMX_ComponentAPICheck(pOMXComponent) != OMX_ErrorNone) {
            if (NULL != pOMXComponent->ComponentDeInit)
                pOMXComponent->ComponentDeInit(pOMXComponent);
            OSAL_Free(pOMXComponent);
            OSAL_dlclose(libHandle);
            ret = OMX_ErrorInvalidComponent;
            OSAL_Log(JETOMX_LOG_ERROR, "OMX_ErrorInvalidComponent, Line:%d", __LINE__);
            goto EXIT;
        }
        jetomx_component->libHandle = libHandle;
        jetomx_component->pOMXComponent = pOMXComponent;
        ret = OMX_ErrorNone;
    }

EXIT:
    FunctionOut();

    return ret;
}

OMX_ERRORTYPE JetOMX_ComponentUnload(JETOMX_COMPONENT *jetomx_component)
{
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pOMXComponent = NULL;

    FunctionIn();

    if (!jetomx_component) {
        ret = OMX_ErrorBadParameter;
        goto EXIT;
    }

    pOMXComponent = jetomx_component->pOMXComponent;
    if (pOMXComponent != NULL) {
        pOMXComponent->ComponentDeInit(pOMXComponent);
        OSAL_Free(pOMXComponent);
        jetomx_component->pOMXComponent = NULL;
    }

    if (jetomx_component->libHandle != NULL) {
        OSAL_dlclose(jetomx_component->libHandle);
        jetomx_component->libHandle = NULL;
    }

EXIT:
    FunctionOut();

    return ret;
}
