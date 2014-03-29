/*
 * @file        OSAL_ETC.c
 * @brief
 * @author
 * @version     0.1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "OSAL_Memory.h"
#include "OSAL_ETC.h"
#include "OSAL_Log.h"

static struct timeval perfStart[PERF_ID_MAX+1], perfStop[PERF_ID_MAX+1];
static unsigned long perfTime[PERF_ID_MAX+1], totalPerfTime[PERF_ID_MAX+1];
static unsigned int perfFrameCount[PERF_ID_MAX+1], perfOver30ms[PERF_ID_MAX+1];

#ifndef HAVE_GETLINE
ssize_t getline(char **ppLine, size_t *pLen, FILE *pStream)
{
    char *pCurrentPointer = NULL;
    size_t const chunk = 512;

    size_t defaultBufferSize = chunk + 1;
    size_t retSize = 0;

    if (*ppLine == NULL) {
        *ppLine = (char *)malloc(defaultBufferSize);
        if (*ppLine == NULL) {
            retSize = -1;
            goto EXIT;
        }
        *pLen = defaultBufferSize;
    }
    else {
        if (*pLen < defaultBufferSize) {
            *ppLine = (char *)realloc(*ppLine, defaultBufferSize);
            if (*ppLine == NULL) {
                retSize = -1;
                goto EXIT;
            }
            *pLen = defaultBufferSize;
        }
    }

    while (1) {
        size_t i;
        size_t j = 0;
        size_t readByte = 0;

        pCurrentPointer = *ppLine + readByte;

        i = fread(pCurrentPointer, 1, chunk, pStream);
        if (i < chunk && ferror(pStream)) {
            retSize = -1;
            goto EXIT;
        }
        while (j < i) {
            ++j;
            if (*pCurrentPointer++ == (char)'\n') {
                *pCurrentPointer = '\0';
                if (j != i) {
                    if (fseek(pStream, j - i, SEEK_CUR)) {
                        retSize = -1;
                        goto EXIT;
                }
                    if (feof(pStream))
                        clearerr(pStream);
                }
                readByte += j;
                retSize = readByte;
                goto EXIT;
            }
        }

        readByte += j;
        if (feof(pStream)) {
            if (readByte) {
                retSize = readByte;
                goto EXIT;
            }
            if (!i) {
                retSize = -1;
                goto EXIT;
            }
        }

        i = ((readByte + (chunk * 2)) / chunk) * chunk;
        if (i != *pLen) {
            *ppLine = (char *)realloc(*ppLine, i);
            if (*ppLine == NULL) {
                retSize = -1;
                goto EXIT;
        }
            *pLen = i;
        }
    }

EXIT:
    return retSize;
}
#endif /* HAVE_GETLINE */

OMX_PTR OSAL_Strcpy(OMX_PTR dest, OMX_PTR src)
{
    return strcpy(dest, src);
}

OMX_PTR OSAL_Strncpy(OMX_PTR dest, OMX_PTR src, size_t num)
{
    return strncpy(dest, src, num);
}

OMX_S32 OSAL_Strcmp(OMX_PTR str1, OMX_PTR str2)
{
    return strcmp(str1, str2);
}

OMX_S32 OSAL_Strncmp(OMX_PTR str1, OMX_PTR str2, size_t num)
{
    return strncmp(str1, str2, num);
}

OMX_PTR OSAL_Strcat(OMX_PTR dest, OMX_PTR src)
{
    return strcat(dest, src);
}

OMX_PTR OSAL_Strncat(OMX_PTR dest, OMX_PTR src, size_t num)
{
    return strncat(dest, src, num);
}

size_t OSAL_Strlen(const char *str)
{
    return strlen(str);
}

static OMX_U32 MeasureTime(struct timeval *start, struct timeval *stop)
{
    unsigned long sec, usec, time;

    sec = stop->tv_sec - start->tv_sec;
    if (stop->tv_usec >= start->tv_usec) {
        usec = stop->tv_usec - start->tv_usec;
    } else {
        usec = stop->tv_usec + 1000000 - start->tv_usec;
        sec--;
    }

    time = sec * 1000000 + (usec);

    return time;
}

void OSAL_PerfInit(PERF_ID_TYPE id)
{
    memset(&perfStart[id], 0, sizeof(perfStart[id]));
    memset(&perfStop[id], 0, sizeof(perfStop[id]));
    perfTime[id] = 0;
    totalPerfTime[id] = 0;
    perfFrameCount[id] = 0;
    perfOver30ms[id] = 0;
}

void OSAL_PerfStart(PERF_ID_TYPE id)
{
    gettimeofday(&perfStart[id], NULL);
}

void OSAL_PerfStop(PERF_ID_TYPE id)
{
    gettimeofday(&perfStop[id], NULL);

    perfTime[id] = MeasureTime(&perfStart[id], &perfStop[id]);
    totalPerfTime[id] += perfTime[id];
    perfFrameCount[id]++;

    if (perfTime[id] > 30000)
        perfOver30ms[id]++;
}

OMX_U32 OSAL_PerfFrame(PERF_ID_TYPE id)
{
    return perfTime[id];
}

OMX_U32 OSAL_PerfTotal(PERF_ID_TYPE id)
{
    return totalPerfTime[id];
}

OMX_U32 OSAL_PerfFrameCount(PERF_ID_TYPE id)
{
    return perfFrameCount[id];
}

int OSAL_PerfOver30ms(PERF_ID_TYPE id)
{
    return perfOver30ms[id];
}

void OSAL_PerfPrint(OMX_STRING prefix, PERF_ID_TYPE id)
{
    OMX_U32 perfTotal;
    int frameCount;

    frameCount = OSAL_PerfFrameCount(id);
    perfTotal = OSAL_PerfTotal(id);

    OSAL_Log(JETOMX_LOG_INFO, "%s Frame Count: %d", prefix, frameCount);
    OSAL_Log(JETOMX_LOG_INFO, "%s Avg Time: %.2f ms, Over 30ms: %d",
                prefix, (float)perfTotal / (float)(frameCount * 1000),
                OSAL_PerfOver30ms(id));
}
