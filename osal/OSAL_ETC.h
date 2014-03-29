/*
 * @file        OSAL_ETC.h
 * @brief
 * @author
 * @version     0.1.0
 */

#ifndef OSAL_ETC
#define OSAL_ETC

#include "OMX_Types.h"


#ifdef __cplusplus
extern "C" {
#endif

OMX_PTR OSAL_Strcpy(OMX_PTR dest, OMX_PTR src);
OMX_S32 OSAL_Strncmp(OMX_PTR str1, OMX_PTR str2, size_t num);
OMX_S32 OSAL_Strcmp(OMX_PTR str1, OMX_PTR str2);
OMX_PTR OSAL_Strcat(OMX_PTR dest, OMX_PTR src);
size_t OSAL_Strlen(const char *str);
ssize_t getline(char **ppLine, size_t *len, FILE *stream);

/* perf */
typedef enum _PERF_ID_TYPE {
    PERF_ID_CSC = 0,
    PERF_ID_DEC,
    PERF_ID_ENC,
    PERF_ID_USER,
    PERF_ID_MAX,
} PERF_ID_TYPE;

void OSAL_PerfInit(PERF_ID_TYPE id);
void OSAL_PerfStart(PERF_ID_TYPE id);
void OSAL_PerfStop(PERF_ID_TYPE id);
OMX_U32 OSAL_PerfFrame(PERF_ID_TYPE id);
OMX_U32 OSAL_PerfTotal(PERF_ID_TYPE id);
OMX_U32 OSAL_PerfFrameCount(PERF_ID_TYPE id);
int OSAL_PerfOver30ms(PERF_ID_TYPE id);
void OSAL_PerfPrint(OMX_STRING prefix, PERF_ID_TYPE id);

#ifdef __cplusplus
}
#endif

#endif
