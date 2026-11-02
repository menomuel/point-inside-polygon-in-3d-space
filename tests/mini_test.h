#ifndef MINI_TEST_H
#define MINI_TEST_H

// Custom testing framework

#include <iostream>
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

  inline unsigned& failureCount()
  {
    static unsigned count = 0;
    return count;
  }

  struct Registrar
  {
    Registrar(const char* name, void (*function)()) { registry().push_back({ name, function }); }
  };

  inline int runAll()
  {
    std::size_t failedTests = 0;
    for (const TestCase& test : registry())
    {
      const unsigned failuresBefore = failureCount();
      test.function();
      const bool passed = failureCount() == failuresBefore;
      if (!passed)
        ++failedTests;
      std::cout << "[" << (passed ? "  OK  " : " FAIL ") << "] " << test.name << std::endl;
    }
    std::size_t total = registry().size();
    std::cout << (total - failedTests) << " of " << total << " tests passed" << std::endl;
    return failedTests == 0 ? 0 : 1;
  }

} // namespace minitest

#define TEST_CASE(name)                                                                                      \
  static void name();                                                                                        \
  static const minitest::Registrar name##Registrar(#name, &name);                                            \
  static void name()

#define CHECK_EQ(actual, expected)                                                                           \
  do                                                                                                         \
  {                                                                                                          \
    const auto& actualValue = (actual);                                                                      \
    const auto& expectedValue = (expected);                                                                  \
    if (!(actualValue == expectedValue))                                                                     \
    {                                                                                                        \
      ++minitest::failureCount();                                                                            \
      std::cout << __FILE__ << "(" << __LINE__ << "): " << "CHECK_EQ failed: " << #actual << std::endl       \
                << "    actual: " << toString(actualValue) << std::endl                                      \
                << "    expected: " << toString(expectedValue) << std::endl;                                 \
    }                                                                                                        \
  } while (false)

#endif // MINI_TEST_H
