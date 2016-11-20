#define NOMINMAX

#include <cstdlib>
#include <cstring>
#include <limits>
#include <thread>

#if defined(NDEBUG)
#undef NDEBUG
#endif
#include <cassert>

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
    for (long long int i = 0; result != 100500; ++i)
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
    volatile int i = 0;

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

#if !defined(PLATFORM_OS_WINDOWS)
static int infiniteRecursion(int arg)
{
    return infiniteRecursion(arg - 1) + infiniteRecursion(arg - 2);
}
#endif

TEST(DeathTest, UnhandledExceptionHandlingNotStarted)
{
    EXPECT_DEATH(raiseRecoverableException(), "");
}

TEST(DeathTest, UnhandledExceptionHandlingStarted)
{
    HwExceptionHandler hw_exception_handler;

    EXPECT_DEATH(raiseRecoverableException(), "");
}

TEST(DeathTest, UnhandledExceptionHandlingStopped)
{
    { HwExceptionHandler hw_exception_handler; }
    EXPECT_DEATH(raiseRecoverableException(), "");
}

TEST(DeathTest, UnhandledExceptionInsideHwTryHandlingStopped)
{
    {
        HwExceptionHandler hw_exception_handler;

        HW_TRY {
            raiseRecoverableException();
        }
        HW_CATCH() {
        }
        HW_FINALLY {
        }

    }

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
    HwExceptionHandler hw_exception_handler;

    HW_TRY {
        EXPECT_DEATH(raiseUnrecoverableException(), "");
    }
    HW_CATCH() {
    }
    HW_FINALLY {
    }
}

TEST(DeathTest, ExceptionInsideCatch)
{
    HwExceptionHandler hw_exception_handler;

    EXPECT_DEATH(exceptionInsideCatch(), "");
}

TEST(DeathTest, ReturnFromCatch)
{
    HwExceptionHandler hw_exception_handler;

    returnFromCatch();
    EXPECT_DEATH(raiseRecoverableException(), "");
}

TEST(DeathTest, AssertInsideTry)
{
    bool status = true;

    HwExceptionHandler hw_exception_handler;

    HW_TRY {
        EXPECT_DEATH(assert(0), "");
    }
    HW_CATCH() {
        status = false;
    }
    HW_FINALLY{
    }

    EXPECT_EQ(status, true);
}

TEST(DeathTest, AssertInsideCatch)
{
    bool status = true;

    HwExceptionHandler hw_exception_handler;

    HW_TRY {
        raiseRecoverableException();
    }
    HW_CATCH() {
        EXPECT_DEATH(assert(0), "");
        status = false;
    }
    HW_FINALLY{
    }

    EXPECT_EQ(status, false);
}

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

static void singleIteration()
{
    bool status = true;

    HwExceptionHandler hw_exception_handler;

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

static void multipleIterations()
{
    HwExceptionHandler hw_exception_handler;

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
}

static void multipleIterationsWithReinitialization()
{
    for (int i = 0; i < 100; ++i) {
        bool status = true;

        HwExceptionHandler hw_exception_handler;

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
}


TEST(SingleThread, SingleIteration)
{
    singleIteration();
}

TEST(SingleThread, MultipleIterations)
{
    multipleIterations();
}

TEST(SingleThread, MultipleIterationsWithReinitialization)
{
    multipleIterationsWithReinitialization();
}

static int nestedTryCatch(int depth, int max_depth, int exception_depth)
{
    bool status = true;
    volatile int result = depth;

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

    HwExceptionHandler hw_exception_handler;

    for (int i = 0; i < 100; ++i) {
        int exception_depth = rand() % max_depth;
        EXPECT_EQ(nestedTryCatch(0, max_depth, exception_depth), exception_depth);
    }
}

TEST(SingleThread, NestedCppTryCatch)
{
    bool status_hw = true, status_cpp = true;

    HwExceptionHandler hw_exception_handler;

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

    EXPECT_EQ(status_hw, true);
    EXPECT_EQ(status_cpp, false);
}

TEST(SingleThread, TryCatchFinallyCountMatch)
{
    volatile int try_count = 0;
    volatile int catch_count = 0;
    volatile int finally_count = 0;

    HwExceptionHandler hw_exception_handler;

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

    EXPECT_EQ(try_count, catch_count);
    EXPECT_EQ(try_count, finally_count);
}

#if !defined(PLATFORM_OS_WINDOWS)
TEST(SingleThread, StackOverflow)
{
    bool status = true;
    int result = 100500;

    HwExceptionHandler hw_exception_handler;

    HW_TRY {
        result = infiniteRecursion(100500);
    }
    HW_CATCH() {
        status = false;
    }
    HW_FINALLY {
    }

    EXPECT_EQ(status, false);
    EXPECT_EQ(result, 100500);
}
#endif

#if defined(PLATFORM_OS_WINDOWS)
TEST(SingleThread, OutputDebugStringA)
{
    bool status = true;

    HwExceptionHandler hw_exception_handler;

    HW_TRY {
        OutputDebugStringA("Debug message.\n");
    }
    HW_CATCH() {
        status = false;
    }
    HW_FINALLY {
    }

    EXPECT_EQ(status, true);
}
#endif

TEST(MultiThread, SingleIteration)
{
    std::thread threads[100];

    for (int i = 0; i < 100; ++i)
        threads[i] = std::thread(singleIteration);

    for (int i = 0; i < 100; ++i)
        threads[i].join();
}

TEST(MultiThread, MultipleIterations)
{
    std::thread threads[100];

    for (int i = 0; i < 100; ++i)
        threads[i] = std::thread(multipleIterations);

    for (int i = 0; i < 100; ++i)
        threads[i].join();
}

#if 0
TEST(MultiThread, MultipleIterationsWithReinitialization)
{
    std::thread threads[100];

    for (int i = 0; i < 100; ++i)
        threads[i] = std::thread(multipleIterationsWithReinitialization);

    for (int i = 0; i < 100; ++i)
        threads[i].join();
}
#endif
