#ifndef UUID_B59F51CC_9A8A_4D87_8B11_2EB7CDC75C88
#define UUID_B59F51CC_9A8A_4D87_8B11_2EB7CDC75C88

#include "platform.h"

#include <setjmp.h>

class ExecutionContext final {
public:
    ExecutionContext();

    static void startHwExceptionHandling();
    static void stopHwExceptionHandling();

    static void finally();

    static void throwHwException();

    jmp_buf environment;

private:
    ExecutionContext * prev_context;

#if defined(PLATFORM_OS_WINDOWS)
public:
    bool dirty;
#endif
};

#define HW_TRY { ExecutionContext execution_context; if (!setjmp(execution_context.environment))
#define HW_CATCH() else
#define HW_FINALLY ExecutionContext::finally(); }

#define HW_THROW() ExecutionContext::throwHwException()

#endif
