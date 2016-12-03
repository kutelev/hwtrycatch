#ifndef UUID_B59F51CC_9A8A_4D87_8B11_2EB7CDC75C88
#define UUID_B59F51CC_9A8A_4D87_8B11_2EB7CDC75C88

#include <csetjmp>

#include "platform.h"

class HwExceptionHandler final {
public:
    HwExceptionHandler();
    ~HwExceptionHandler();
};

class ExecutionContext final {
public:
    ExecutionContext();
    ~ExecutionContext();

    static void throwHwException();

    jmp_buf environment;

private:
    ExecutionContext * prev_context;
#if defined(PLATFORM_OS_WINDOWS)
    bool dirty;
#endif
};

#define HW_TO_SW_CONVERTER_UNIQUE_NAME(NAME, LINE) NAME ## LINE
#define HW_TO_SW_CONVERTER_INTERNAL(NAME, LINE) ExecutionContext HW_TO_SW_CONVERTER_UNIQUE_NAME(NAME, LINE); if (setjmp(HW_TO_SW_CONVERTER_UNIQUE_NAME(NAME, LINE).environment)) throw 0
#define HW_TO_SW_CONVERTER() HW_TO_SW_CONVERTER_INTERNAL(execution_context, __LINE__)

#endif
