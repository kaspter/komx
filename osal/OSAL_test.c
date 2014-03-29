#include "OSAL_Memory.h"
#include "OSAL_Mutex.h"


#undef  JETOMX_LOG_TAG
#define JETOMX_LOG_TAG    "JETOMX_OSAL_TEST"
#define JETOMX_LOG_OFF
#define JETOMX_TRACE_ON
#include "OSAL_Log.h"


//LD_LIBRARY_PATH=. ./dumy_test

int main()
{


    char          *libName;

    libName = OSAL_Malloc(24);

    OSAL_Log(JETOMX_LOG_TRACE, "alloc count\n");

    OSAL_Free(libName);

    return 0;

}
