#ifndef MINI_TEST_H
#define MINI_TEST_H

// Custom testing framework.
// TEST_CASE registers a function, CHECK / CHECK_EQ record failures without stopping the test.

#include <cstdio>
#include <vector>

namespace minitest
{

  struct TestCase
  {
    const char* name;
    void (*function)();
  };

  inline std::vector<TestCase>& registry()
  {
    static std::vector<TestCase> tests;
    return tests;
  }

  inline int& failureCount()
  {
    static int count = 0;
    return count;
  }

  struct Registrar
  {
    Registrar(const char* name, void (*function)()) { registry().push_back({ name, function }); }
  };

  inline int runAll()
  {
    int failedTests = 0;
    for (const TestCase& test : registry())
    {
      const int failuresBefore = failureCount();
      test.function();
      const bool passed = failureCount() == failuresBefore;
      if (!passed)
        ++failedTests;
      std::printf("[%s] %s\n", passed ? "  OK  " : " FAIL ", test.name);
    }
    std::printf("\n%d of %zu tests failed\n", failedTests, registry().size());
    return failedTests == 0 ? 0 : 1;
  }

} // namespace minitest

#define TEST_CASE(name)                                                                                      \
  static void name();                                                                                        \
  static const minitest::Registrar name##Registrar(#name, &name);                                            \
  static void name()

#define CHECK(expr)                                                                                          \
  do                                                                                                         \
  {                                                                                                          \
    if (!(expr))                                                                                             \
    {                                                                                                        \
      ++minitest::failureCount();                                                                            \
      std::printf("%s(%d): CHECK failed: %s\n", __FILE__, __LINE__, #expr);                                  \
    }                                                                                                        \
  } while (false)

#define CHECK_EQ(actual, expected)                                                                           \
  do                                                                                                         \
  {                                                                                                          \
    const auto& actualValue = (actual);                                                                      \
    const auto& expectedValue = (expected);                                                                  \
    if (!(actualValue == expectedValue))                                                                     \
    {                                                                                                        \
      ++minitest::failureCount();                                                                            \
      std::printf("%s(%d): CHECK_EQ failed: %s\n    actual:   %s\n    expected: %s\n", __FILE__, __LINE__,   \
                  #actual, toString(actualValue), toString(expectedValue));                                  \
    }                                                                                                        \
  } while (false)

#endif // MINI_TEST_H