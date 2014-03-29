/*
 * @file        OSAL_Log.h
 * @brief
 * @author
 * @version     0.1.0
 */

#ifndef OSAL_LOG
#define OSAL_LOG

#ifdef __cplusplus
extern "C" {
#endif

#ifndef JETOMX_LOG_OFF
#define JETOMX_LOG
#endif

#ifndef JETOMX_LOG_TAG
#define JETOMX_LOG_TAG    "JETOMX_LOG"
#endif

#ifdef JETOMX_TRACE_ON
#define JETOMX_TRACE
#endif

typedef enum _LOG_LEVEL
{
    JETOMX_LOG_TRACE,
    JETOMX_LOG_INFO,
    JETOMX_LOG_WARNING,
    JETOMX_LOG_ERROR
} JETOMX_LOG_LEVEL;

#ifdef JETOMX_LOG
#define OSAL_Log(a, ...)    ((void)_OSAL_Log(a, JETOMX_LOG_TAG, __VA_ARGS__))
#else
#define OSAL_Log(a, ...)                                                \
    do {                                                                \
        if (a == JETOMX_LOG_ERROR)                                     \
            ((void)_OSAL_Log(a, JETOMX_LOG_TAG, __VA_ARGS__)); \
    } while (0)
#endif

#ifdef JETOMX_TRACE
#define FunctionIn() _OSAL_Log(JETOMX_LOG_TRACE, JETOMX_LOG_TAG, "%s In , Line: %d", __FUNCTION__, __LINE__)
#define FunctionOut() _OSAL_Log(JETOMX_LOG_TRACE, JETOMX_LOG_TAG, "%s Out , Line: %d", __FUNCTION__, __LINE__)
#else
#define FunctionIn() ((void *)0)
#define FunctionOut() ((void *)0)
#endif

extern void _OSAL_Log(JETOMX_LOG_LEVEL logLevel, const char *tag, const char *msg, ...);

#ifdef __cplusplus
}
#endif

#endif
