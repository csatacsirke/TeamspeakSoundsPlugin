#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include <Plugin/Wave/AudioProcessor.h>
#include <Plugin/Wave/AudioProcessing.h>

using namespace AudioProcessing;


namespace SoundPlayerTests
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			// TODO: Your test code here
			Filter filter;


			std::vector<short> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

			filter.ProcessData(AudioData{ data.data(), 5, 2 });
			
			int a = 42;
			Assert::AreEqual(a, 42);


		}

	};
}