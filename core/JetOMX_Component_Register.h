/*
 * @file       JetOMX_Component_Register.h
 * @brief      Jetlab OpenMAX IL Component Register
 * @author
 * @version    0.1.0
 */

#ifndef JETOMX_COMPONENT_REG
#define JETOMX_COMPONENT_REG

#include "JetOMX_Def.h"
#include "OMX_Types.h"
#include "OMX_Core.h"
#include "OMX_Component.h"


typedef struct _RegisterComponentType
{
    OMX_U8  componentName[MAX_OMX_COMPONENT_NAME_SIZE];
    OMX_U8  roles[MAX_OMX_COMPONENT_ROLE_NUM][MAX_OMX_COMPONENT_ROLE_SIZE];
    OMX_U32 totalRoleNum;
} RegisterComponentType;

typedef struct _JETOMX_COMPONENT_REGLIST
{
    RegisterComponentType component;
    OMX_U8  libName[MAX_OMX_COMPONENT_LIBNAME_SIZE];
} JETOMX_COMPONENT_REGLIST;

struct JETOMX_COMPONENT;
typedef struct _JETOMX_COMPONENT
{
    OMX_U8                        componentName[MAX_OMX_COMPONENT_NAME_SIZE];
    OMX_U8                        libName[MAX_OMX_COMPONENT_LIBNAME_SIZE];
    OMX_HANDLETYPE                libHandle;
    OMX_COMPONENTTYPE             *pOMXComponent;
    struct _JETOMX_COMPONENT      *nextOMXComp;
} JETOMX_COMPONENT;


#ifdef __cplusplus
extern "C" {
#endif


OMX_ERRORTYPE JetOMX_Component_Register(JETOMX_COMPONENT_REGLIST **compList, OMX_U32 *compNum);
OMX_ERRORTYPE JetOMX_Component_Unregister(JETOMX_COMPONENT_REGLIST *componentList);
OMX_ERRORTYPE JetOMX_ComponentLoad(JETOMX_COMPONENT *jetomx_component);
OMX_ERRORTYPE JetOMX_ComponentUnload(JETOMX_COMPONENT *jetomx_component);


#ifdef __cplusplus
}
#endif

#endif
