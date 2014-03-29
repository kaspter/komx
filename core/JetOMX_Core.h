/*
 * @file       JetOMX_Core.h
 * @brief      Jetlab OpenMAX IL Core
 * @author
 * @version    0.1.0
 */

#ifndef JETOMX_CORE
#define JETOMX_CORE

#include "JetOMX_Def.h"
#include "OMX_Types.h"
#include "OMX_Core.h"


#ifdef __cplusplus
extern "C" {
#endif


JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_Init(void);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_Deinit(void);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_ComponentNameEnum(
    OMX_OUT   OMX_STRING        cComponentName,
    OMX_IN    OMX_U32           nNameLength,
    OMX_IN    OMX_U32           nIndex);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_GetHandle(
    OMX_OUT   OMX_HANDLETYPE   *pHandle,
    OMX_IN    OMX_STRING        cComponentName,
    OMX_IN    OMX_PTR           pAppData,
    OMX_IN    OMX_CALLBACKTYPE *pCallBacks);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_FreeHandle(
    OMX_IN    OMX_HANDLETYPE    hComponent);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_SetupTunnel(
    OMX_IN    OMX_HANDLETYPE    hOutput,
    OMX_IN    OMX_U32           nPortOutput,
    OMX_IN    OMX_HANDLETYPE    hInput,
    OMX_IN    OMX_U32           nPortInput);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_TeardownTunnel(
    OMX_IN  OMX_HANDLETYPE hOutput,
    OMX_IN  OMX_U32 nPortOutput,
    OMX_IN  OMX_HANDLETYPE hInput,
    OMX_IN  OMX_U32 nPortInput);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE              JetOMX_GetContentPipe(
    OMX_OUT   OMX_HANDLETYPE   *hPipe,
    OMX_IN    OMX_STRING        szURI);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE              JetOMX_GetComponentsOfRole(
    OMX_IN    OMX_STRING        role,
    OMX_INOUT OMX_U32          *pNumComps,
    OMX_INOUT OMX_U8          **compNames);
JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE              JetOMX_GetRolesOfComponent(
    OMX_IN    OMX_STRING        compName,
    OMX_INOUT OMX_U32          *pNumRoles,
    OMX_OUT   OMX_U8          **roles);

JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_GetCoreInterface(
    OMX_OUT void ** ppItf,
    OMX_IN OMX_STRING cExtensionName);

JETOMX_EXPORT_REF OMX_API void OMX_APIENTRY JetOMX_FreeCoreInterface(
    OMX_IN void * pItf);

JETOMX_EXPORT_REF OMX_API OMX_ERRORTYPE OMX_APIENTRY JetOMX_ComponentOfRoleEnum(
    OMX_OUT OMX_STRING compName,
    OMX_IN  OMX_STRING role,
    OMX_IN  OMX_U32 nIndex);

#ifdef __cplusplus
}
#endif

#endif
