/*
 * @file       OSAL_Library.h
 * @brief
 * @author
 * @version    0.1.0
 */

#ifndef OSAL_LIBRARY
#define OSAL_LIBRARY

#include "OMX_Types.h"


#ifdef __cplusplus
extern "C" {
#endif

void *OSAL_dlopen(const char *filename, int flag);
void *OSAL_dlsym(void *handle, const char *symbol);
int   OSAL_dlclose(void *handle);
const char *OSAL_dlerror(void);

#ifdef __cplusplus
}
#endif

#endif
