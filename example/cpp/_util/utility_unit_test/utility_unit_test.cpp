#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// [Write unit tests for C/C++ in Visual Studio](https://docs.microsoft.com/en-us/visualstudio/test/writing-unit-tests-for-c-cpp?view=vs-2019)

namespace utilityunittest
{
    TEST_CLASS(utilityunittest)
    {
    private:

        bool var = true;

    public:

        TEST_METHOD(TestMethod1)
        {
            Assert::AreEqual(true, var);
            Assert::AreNotEqual(false, var);
        }
    };
}
