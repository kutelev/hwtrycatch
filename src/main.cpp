#define NOMINMAX

#include <cstdlib>
#include <cstring>
#include <limits>

#include <src/gmock-all.cc>
#include <src/gtest-all.cc>

#include "platform.h"
#include "hwtrycatch.h"

GTEST_API_ int main (int argc, char** argv)
{
    testing::InitGoogleMock (&argc, argv);
    return RUN_ALL_TESTS();
}

static int raiseRecoverableException()
{
    static char tiny_array[4] = {0, 1, 2, 3};
    int result = 0;
    for (long long int i = 0; i < std::numeric_limits<long long int>::max(); ++i)
        result = (result + tiny_array[i]) / tiny_array[i];
    char * invalid_pointer = 0;
    invalid_pointer[0] = 0;
    return result * tiny_array[0];
}

static void raiseUnrecoverableException()
{
#if defined(PLATFORM_OS_WINDOWS)
    char tiny_array[4];
    memset(tiny_array, 0, std::numeric_limits<size_t>::max());
    raiseUnrecoverableException();
#else
    main(0, 0);
#endif
}

static void exceptionInsideCatch()
{
    int i = 0;

    HW_TRY {
        if (i == 0)
            raiseRecoverableException();
    }
    HW_CATCH() {
        if (++i == 100)
            raiseUnrecoverableException();
        raiseRecoverableException();
    }
    HW_FINALLY {
    }
}

#if !defined(PLATFORM_OS_WINDOWS)
static int returnFromCatch()
{
    HW_TRY {
        raiseRecoverableException();
    }
    HW_CATCH() {
        return -1;
    }
    HW_FINALLY {
    }

    return 0;
}
#endif

#if !defined(PLATFORM_OS_WINDOWS)
static int infiniteRecursion(int arg)
{
    if (arg)
        arg = infiniteRecursion(arg ^ 0xAA);
    else
        arg += 100;

    return arg;
}
#endif

TEST(DeathTest, UnhandledExceptionHandlingNotStarted)
{
    EXPECT_DEATH(raiseRecoverableException(), "");
}

TEST(DeathTest, UnhandledExceptionHandlingStarted)
{
    ExecutionContext::startHwExceptionHandling();
    EXPECT_DEATH(raiseRecoverableException(), "");
    ExecutionContext::stopHwExceptionHandling();
}

TEST(DeathTest, UnhandledExceptionHandlingStopped)
{
    ExecutionContext::startHwExceptionHandling();
    ExecutionContext::stopHwExceptionHandling();
    EXPECT_DEATH(raiseRecoverableException(), "");
}

TEST(DeathTest, UnhandledExceptionInsideHwTryHandlingStopped)
{
    ExecutionContext::startHwExceptionHandling();

    HW_TRY {
        raiseRecoverableException();
    }
    HW_CATCH() {
    }
    HW_FINALLY {
    }

    ExecutionContext::stopHwExceptionHandling();

    HW_TRY {
        EXPECT_DEATH(raiseRecoverableException(), "");
    }
    HW_CATCH() {
    }
    HW_FINALLY {
    }
}

TEST(DeathTest, UnrecoverableError)
{
    ExecutionContext::startHwExceptionHandling();

    HW_TRY {
        EXPECT_DEATH(raiseUnrecoverableException(), "");
    }
    HW_CATCH() {
    }
    HW_FINALLY {
    }

    ExecutionContext::stopHwExceptionHandling();
}

TEST(DeathTest, ExceptionInsideCatch)
{
    ExecutionContext::startHwExceptionHandling();
    EXPECT_DEATH(exceptionInsideCatch(), "");
    ExecutionContext::stopHwExceptionHandling();
}

#if !defined(PLATFORM_OS_WINDOWS)
TEST(DeathTest, ReturnFromCatch)
{
    int result = 0;

    ExecutionContext::startHwExceptionHandling();
    returnFromCatch();
    result = raiseRecoverableException();
    ExecutionContext::stopHwExceptionHandling();

    EXPECT_NE(result, 0);
}
#endif

TEST(SingleThread, ExplicitHwThrow)
{
    bool status = true;

    HW_TRY {
        HW_THROW();
    }
    HW_CATCH() {
        status = false;
    }
    HW_FINALLY {
    }

    EXPECT_EQ(status, false);
}

TEST(SingleThread, SingleIteration)
{
    bool status = true;

    ExecutionContext::startHwExceptionHandling();

    HW_TRY {
        raiseRecoverableException();
    }
    HW_CATCH() {
        status = false;
    }
    HW_FINALLY {
    }

    ExecutionContext::stopHwExceptionHandling();

    EXPECT_EQ(status, false);
}

TEST(SingleThread, MultipleIterations)
{
    ExecutionContext::startHwExceptionHandling();

    for (int i = 0; i < 100; ++i) {
        bool status = true;

        HW_TRY {
            raiseRecoverableException();
        }
        HW_CATCH() {
            status = false;
        }
        HW_FINALLY {
        }

        EXPECT_EQ(status, false);
    }

    ExecutionContext::stopHwExceptionHandling();
}

TEST(SingleThread, MultipleIterationsWithReinitialization)
{
    for (int i = 0; i < 100; ++i) {
        bool status = true;

        ExecutionContext::startHwExceptionHandling();

        HW_TRY {
            raiseRecoverableException();
        }
        HW_CATCH() {
            status = false;
        }
        HW_FINALLY {
        }

        ExecutionContext::stopHwExceptionHandling();

        EXPECT_EQ(status, false);
    }
}

static int nestedTryCatch(int depth, int max_depth, int exception_depth)
{
    bool status = true;
    int result = depth;

    HW_TRY {
        if (depth == exception_depth)
            raiseRecoverableException();
        if (depth < max_depth)
            result = nestedTryCatch(depth + 1, max_depth, exception_depth);
        raiseRecoverableException();
    }
    HW_CATCH() {
        status = false;
    }
    HW_FINALLY {
    }

    EXPECT_EQ(status, false);

    return result;
}

TEST(SingleThread, NestedTryCatch)
{
    static int max_depth = 50;

    ExecutionContext::startHwExceptionHandling();

    for (int i = 0; i < 100; ++i) {
        int exception_depth = rand() % max_depth;
        EXPECT_EQ(nestedTryCatch(0, max_depth, exception_depth), exception_depth);
    }

    ExecutionContext::stopHwExceptionHandling();
}

TEST(SingleThread, NestedCppTryCatch)
{
    bool status_hw = true, status_cpp = true;

    ExecutionContext::startHwExceptionHandling();

    HW_TRY {
        try {
            throw 0;
        }
        catch (...) {
            status_cpp = false;
        }
    }
    HW_CATCH() {
        status_hw = false;
    }
    HW_FINALLY {
    }

    ExecutionContext::stopHwExceptionHandling();

    EXPECT_EQ(status_hw, true);
    EXPECT_EQ(status_cpp, false);
}

TEST(SingleThread, TryCatchFinallyCountMatch)
{
    int try_count = 0;
    int catch_count = 0;
    int finally_count = 0;

    ExecutionContext::startHwExceptionHandling();

    for (int i = 0; i < 100; ++i) {
        HW_TRY {
            ++try_count;
            raiseRecoverableException();
        }
        HW_CATCH() {
            ++catch_count;
        }
        HW_FINALLY {
            ++finally_count;
        }
    }

    ExecutionContext::stopHwExceptionHandling();

    EXPECT_EQ(try_count, catch_count);
    EXPECT_EQ(try_count, finally_count);
}

#if !defined(PLATFORM_OS_WINDOWS)
TEST(SingleThread, StackOverflow)
{
    bool status = true;
    int result = 100500;

    ExecutionContext::startHwExceptionHandling();

    HW_TRY {
        result = infiniteRecursion(100);
    }
    HW_CATCH() {
        status = false;
    }
    HW_FINALLY {
    }

    ExecutionContext::stopHwExceptionHandling();

    EXPECT_EQ(status, false);
    EXPECT_EQ(result, 100500);
}
#endif
