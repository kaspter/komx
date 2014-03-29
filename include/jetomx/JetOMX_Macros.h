/*
 * @file        JetOMX_Macros.h
 * @brief       Macros
 * @author
 * @version     0.1.0
 */

#ifndef JETOMX_MACROS
#define JETOMX_MACROS

#include "JetOMX_Def.h"
#include "OSAL_Memory.h"


/*
 * MACROS
 */
#define ALIGN(x, a)       (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_TO_16B(x)   ((((x) + (1 <<  4) - 1) >>  4) <<  4)
#define ALIGN_TO_32B(x)   ((((x) + (1 <<  5) - 1) >>  5) <<  5)
#define ALIGN_TO_128B(x)  ((((x) + (1 <<  7) - 1) >>  7) <<  7)
#define ALIGN_TO_8KB(x)   ((((x) + (1 << 13) - 1) >> 13) << 13)

#define INIT_SET_SIZE_VERSION(_struct_, _structType_)               \
    do {                                                            \
        OSAL_Memset((_struct_), 0, sizeof(_structType_));       \
        (_struct_)->nSize = sizeof(_structType_);                   \
        (_struct_)->nVersion.s.nVersionMajor = VERSIONMAJOR_NUMBER; \
        (_struct_)->nVersion.s.nVersionMinor = VERSIONMINOR_NUMBER; \
        (_struct_)->nVersion.s.nRevision = REVISION_NUMBER;         \
        (_struct_)->nVersion.s.nStep = STEP_NUMBER;                 \
    } while (0)

/*
 * Port Specific
 */
#define EXYNOS_TUNNEL_ESTABLISHED 0x0001
#define EXYNOS_TUNNEL_IS_SUPPLIER 0x0002

#define CHECK_PORT_BEING_FLUSHED(port)                 (port->bIsPortFlushed == OMX_TRUE)
#define CHECK_PORT_BEING_DISABLED(port)                (port->bIsPortDisabled == OMX_TRUE)
#define CHECK_PORT_BEING_FLUSHED_OR_DISABLED(port)     ((port->bIsPortFlushed == OMX_TRUE) || (port->bIsPortDisabled == OMX_TRUE))
#define CHECK_PORT_ENABLED(port)                       (port->portDefinition.bEnabled == OMX_TRUE)
#define CHECK_PORT_POPULATED(port)                     (port->portDefinition.bPopulated == OMX_TRUE)
#define CHECK_PORT_TUNNELED(port)                      (port->tunnelFlags & EXYNOS_TUNNEL_ESTABLISHED)
#define CHECK_PORT_BUFFER_SUPPLIER(port)               (port->tunnelFlags & EXYNOS_TUNNEL_IS_SUPPLIER)

#endif
