#ifndef UUID_B59F51CC_9A8A_4D87_8B11_2EB7CDC75C88
#define UUID_B59F51CC_9A8A_4D87_8B11_2EB7CDC75C88

#include <csetjmp>

#include "platform.h"

namespace hwtrycatch {

class HwExceptionHandler final {
public:
    HwExceptionHandler();
    ~HwExceptionHandler();
};

class ExecutionContext final {
    friend class HwException;
public:
    ExecutionContext();
    ~ExecutionContext();

    jmp_buf environment;

private:
    const char* humanReadableName() const;

    ExecutionContext* prev_context;
#if defined(PLATFORM_OS_WINDOWS)
    bool dirty;
#endif
    int exception_type;
};

class HwException final : public std::runtime_error {
public:
    explicit HwException(const ExecutionContext& execution_context) : std::runtime_error(execution_context.humanReadableName()) { }
};

#define HW_TO_SW_CONVERTER_UNIQUE_NAME(NAME, LINE) \
    NAME ## LINE

#define HW_TO_SW_CONVERTER_INTERNAL(NAME, LINE) \
    ExecutionContext HW_TO_SW_CONVERTER_UNIQUE_NAME(NAME, LINE); \
    if (setjmp(HW_TO_SW_CONVERTER_UNIQUE_NAME(NAME, LINE).environment)) \
        throw HwException(HW_TO_SW_CONVERTER_UNIQUE_NAME(NAME, LINE))

#define HW_TO_SW_CONVERTER() \
    HW_TO_SW_CONVERTER_INTERNAL(execution_context, __LINE__)

}

#endif
