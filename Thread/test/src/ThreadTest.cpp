#include "pbb/Thread.h"
#include "gtest/gtest.h"

using namespace pbb;

class ThreadTest : public ::testing::Test {
public:
};

class TestRunnable : public IRunnable
{
public:
    int started;
    Thread* self;
    TestRunnable()
    {
        started = 0;
        self = 0;
    }
    void Run()
    {
        self = Thread::Current();
        started = 1;
    }
};

TEST_F(ThreadTest, Start)
{
    TestRunnable run1;
    Thread thread1;

    // hasn't started
    ASSERT_EQ(0, run1.started);

    thread1.Start(run1, "Thread1");
    ASSERT_EQ("Thread1", thread1.GetName());
    thread1.Join();

    // Check thread ran and set value
    ASSERT_EQ(1, run1.started);

    ASSERT_NE(Thread::Current(), run1.self);
    ASSERT_NE((Thread*)0, run1.self);
}
