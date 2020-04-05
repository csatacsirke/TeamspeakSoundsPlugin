#include "stdafx.h"
#include "CppUnitTest.h"

#include <Plugin/Util/Util.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{		
	using namespace TSPlugin;

	TEST_CLASS(PluginUtilTest)
	{
	public:
		
		TEST_METHOD(CircularDistanceTest)
		{
			Assert::IsTrue(circular_distance(0, 0, 1) == 0);

			Assert::IsTrue(circular_distance(0, 0, 2) == 0);
			Assert::IsTrue(circular_distance(0, 1, 2) == 1);
			Assert::IsTrue(circular_distance(1, 0, 2) == 1);
			Assert::IsTrue(circular_distance(1, 1, 2) == 0);


			Assert::IsTrue(circular_distance(0, 1, 3) == 1);
			Assert::IsTrue(circular_distance(0, 2, 3) == 1);

			Assert::IsTrue(circular_distance(1, 0, 3) == 1);
			Assert::IsTrue(circular_distance(1, 2, 3) == 1);


			Assert::IsTrue(circular_distance(0, 3, 4) == 1);
			Assert::IsTrue(circular_distance(1, 2, 4) == 1);
			Assert::IsTrue(circular_distance(0, 2, 4) == 2);
			Assert::IsTrue(circular_distance(1, 3, 4) == 2);

			Assert::IsTrue(circular_distance(0, 4, 5) == 1);
			Assert::IsTrue(circular_distance(0, 1, 5) == 1);
			Assert::IsTrue(circular_distance(0, 3, 5) == 2);
			Assert::IsTrue(circular_distance(1, 3, 5) == 2);

			Assert::IsTrue(circular_distance(0, 9, 10) == 1);
			Assert::IsTrue(circular_distance(2, 8, 10) == 4);


		}

	};
}
