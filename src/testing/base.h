#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <common/logging/logging.h>

namespace testing
{
#define TEST_MODULE        \
    int main(int, char **) \
    {                      \
        bool __test_module_valid = true;

#define TEST_MODULE_END      \
    if (__test_module_valid) \
        return 0;            \
    else                     \
        return -1;           \
    }

#define TEST_CASE(fnc) void fnc()

#define TEST_CASE_RUN(fnc)                                                   \
    {                                                                        \
        try                                                                  \
        {                                                                    \
            (fnc)();                                                         \
        }                                                                    \
        catch (const std::exception &e)                                      \
        {                                                                    \
            std::cerr << "Testcase \"" << #fnc << "\" failed:" << std::endl; \
            std::cerr << e.what() << '\n';                                   \
            __test_module_valid = false;                                     \
        }                                                                    \
    }

#define VARIABLE_INFORMATION(x)                                                                              \
    (std::to_string((x)).compare(#x) != 0                                                                    \
         ? std::string("var ") + std::string(#x) + std::string("(") + std::to_string((x)) + std::string(")") \
         : std::string(#x))

#define ASSERT_EQUAL(x, y)                                                                                                                            \
    {                                                                                                                                                 \
        if ((x) != (y))                                                                                                                               \
        {                                                                                                                                             \
            throw std::runtime_error(LINE_INFORMATION + std::string("\t") + VARIABLE_INFORMATION(x) + std::string(" != ") + VARIABLE_INFORMATION(y)); \
        }                                                                                                                                             \
    }

#define ASSERT_NOT_EQUAL(x, y)                                                                                                                        \
    {                                                                                                                                                 \
        if ((x) == (y))                                                                                                                               \
        {                                                                                                                                             \
            throw std::runtime_error(LINE_INFORMATION + std::string("\t") + VARIABLE_INFORMATION(x) + std::string(" == ") + VARIABLE_INFORMATION(y)); \
        }                                                                                                                                             \
    }

#define ASSERT_TRUE(x)                                                                                                           \
    {                                                                                                                            \
        if (!(x))                                                                                                                \
        {                                                                                                                        \
            throw std::runtime_error(LINE_INFORMATION + std::string("\t") + VARIABLE_INFORMATION(x) + std::string(" is False")); \
        }                                                                                                                        \
    }

#define ASSERT_FALSE(x)                                                                                                         \
    {                                                                                                                           \
        if ((x))                                                                                                                \
        {                                                                                                                       \
            throw std::runtime_error(LINE_INFORMATION + std::string("\t") + VARIABLE_INFORMATION(x) + std::string(" is True")); \
        }                                                                                                                       \
    }
} // namespace testing