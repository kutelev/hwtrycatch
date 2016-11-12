#include <atomic>
#include <mutex>

#include "hwtrycatch.h"
#include "platform.h"

#if defined(PLATFORM_OS_WINDOWS)
#include <excpt.h>
#include <windows.h>
#else
#include <signal.h>
#endif

static thread_local ExecutionContext * execution_context = nullptr;
static std::atomic<int> user_count(0);
static std::mutex mutex;

ExecutionContext::ExecutionContext() : prev_context(execution_context)
{
#if defined(PLATFORM_OS_WINDOWS)
    dirty = false;
#endif
    execution_context = this;
}

#if defined(PLATFORM_OS_WINDOWS)

static PVOID exception_handler_handle = 0;

static LONG WINAPI vectoredExceptionHandler(struct _EXCEPTION_POINTERS *_exception_info)
{
    struct ExecutionContextStruct {
        jmp_buf environment;
        ExecutionContext * prev_context;
        bool dirty;
    };

    if (!execution_context ||
        _exception_info->ExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C ||
        _exception_info->ExceptionRecord->ExceptionCode == 0xE06D7363L /* C++ exception */
    )
        return EXCEPTION_CONTINUE_SEARCH;

    reinterpret_cast<ExecutionContextStruct *>(execution_context)->dirty = true;
    longjmp(execution_context->environment, 0);
}

#else

#define MIN2(A, B) (A < B ? A : B)
#define MIN4(A, B, C, D) (MIN2(A, B) < MIN2(C, D) ? MIN2(A, B) : MIN2(C, D))
#define MAX2(A, B) (A > B ? A : B)
#define MAX4(A, B, C, D) (MAX2(A, B) > MAX2(C, D) ? MAX2(A, B) : MAX2(C, D))

#define MIN_SIGNUM MIN4(SIGBUS, SIGFPE, SIGILL, SIGSEGV)
#define MAX_SIGNUM MAX4(SIGBUS, SIGFPE, SIGILL, SIGSEGV)

static const int handled_signals[4] = { SIGBUS, SIGFPE, SIGILL, SIGSEGV };
static struct sigaction prev_handlers[MAX_SIGNUM - MIN_SIGNUM + 1];
static uint8_t exception_handler_stack[SIGSTKSZ];

static void signalHandler(int signum)
{
    if (execution_context) {
        sigset_t signals;
        sigemptyset(&signals);
        sigaddset(&signals, signum);
        sigprocmask(SIG_UNBLOCK, &signals, NULL);
        longjmp(execution_context->environment, 0);
    }
    else if (prev_handlers[signum - MIN_SIGNUM].sa_handler) {
        prev_handlers[signum - MIN_SIGNUM].sa_handler(signum);
    }
    else {
        signal(signum, SIG_DFL);
        raise(signum);
    }
}

#endif

void ExecutionContext::startHwExceptionHandling()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (++user_count != 1)
        return;

#if defined(PLATFORM_OS_WINDOWS)
    exception_handler_handle = AddVectoredExceptionHandler(1, vectoredExceptionHandler);
#else

    stack_t ss;
    ss.ss_sp = exception_handler_stack;
    ss.ss_flags = 0;
    ss.ss_size = SIGSTKSZ;
    sigaltstack(&ss, 0);

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_ONSTACK;
    sa.sa_handler = signalHandler;

    for (int signum : handled_signals)
        sigaction(signum, &sa, &prev_handlers[signum - MIN_SIGNUM]);
#endif
}

void ExecutionContext::stopHwExceptionHandling()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (--user_count != 0)
        return;

#if defined(PLATFORM_OS_WINDOWS)
    RemoveVectoredExceptionHandler(exception_handler_handle);
    exception_handler_handle = 0;
#else
    for (int signum : handled_signals) {
        if (prev_handlers[signum - MIN_SIGNUM].sa_handler)
            sigaction(signum, &prev_handlers[signum - MIN_SIGNUM], 0);
        else
            signal(signum, SIG_DFL);
    }
#endif
}

void ExecutionContext::finally()
{
#if defined(PLATFORM_OS_WINDOWS)
    if (execution_context->dirty)
        RemoveVectoredExceptionHandler(exception_handler_handle);
#endif
    execution_context = execution_context->prev_context;
}

void ExecutionContext::throwHwException()
{
    longjmp(execution_context->environment, 0);
}
