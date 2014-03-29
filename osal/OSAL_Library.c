/*
 * @file        OSAL_Library.c
 * @brief
 * @author
 * @version     0.1.0
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "OSAL_Library.h"


void *OSAL_dlopen(const char *filename, int flag)
{
    return dlopen(filename, flag);
}

void *OSAL_dlsym(void *handle, const char *symbol)
{
    return dlsym(handle, symbol);
}

int OSAL_dlclose(void *handle)
{
    return dlclose(handle);
}

const char *OSAL_dlerror(void)
{
    return dlerror();
}
