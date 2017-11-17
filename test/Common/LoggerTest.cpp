#include "gtest/gtest.h"
#include <stdio.h>
#include "pbb/Logging.h"

using namespace pbb::log;

class LoggerTest : public ::testing::Test {
public:
};

TEST_F(LoggerTest, Push)
{
    char buffer[65536];
    LoggerSink sink(buffer, sizeof(buffer));
    LogReader reader(sink);
    Logger& root = Logger::get("");
    root.setSink(&sink);    
    root.info("hello world");
    
    char buff[2048];
    MessageHeader header;
    ASSERT_EQ(1, reader.pop(header, buff));

    ASSERT_EQ(header.length, 11);
    buff[header.length] = 0;
    ASSERT_STREQ(buff, "hello world");
    
}

TEST_F(LoggerTest, Parents)
{
    char buffer[65536];
    char buffer1[65536];
    char buffer2[65536];

    LoggerSink sink(buffer, sizeof(buffer));
    LoggerSink sink1(buffer1, sizeof(buffer1));
    LoggerSink sink2(buffer2, sizeof(buffer2));
    LogReader reader(sink);
    Logger& root = Logger::get("");
    root.setSink(&sink);

    Logger& a = Logger::get("a");
    Logger& b = Logger::get("b");
    a.setSink(&sink1);
    b.setSink(&sink2);
    Logger& a_a = Logger::get("a.a");
    Logger& b_a = Logger::get("b.a");
    Logger& caba = Logger::get("c.a.b.a");


    ASSERT_EQ(&sink, root.getSink());
    ASSERT_EQ(&sink1, a_a.getSink());
    ASSERT_EQ(&sink2, b_a.getSink());
    ASSERT_EQ(&sink, caba.getSink());


}