#include "stdafx.h"
#include "CppUnitTest.h"

#include <Plugin/stdafx.h>

#include <Plugin/Wave/AudioProcessor.h>
#include <Plugin/Wave/AudioProcessing.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


using namespace TSPlugin;

namespace SoundPlayerTests
{		
	//TEST_CLASS(UnitTest1)
	//{
	//public:

	//	static inline auto GenerateData(size_t count) {
	//		std::vector<short> result;

	//		for (int i = 0; i < count; ++i) {
	//			result.push_back(1000 + i*10);
	//			result.push_back(i*20);
	//		}

	//		return result;
	//	}
	//	
	//	TEST_METHOD(TestMethod1)
	//	{
	//		// TODO: Your test code here
	//		Filter filter;


	//		std::vector<short> data = GenerateData(40);

	//		short* ptr = data.data();
	//		size_t delta = 5 * 2;

	//		filter.ProcessData(AudioData{ ptr, 5, 2 });
	//		ptr += delta;
	//		filter.ProcessData(AudioData{ ptr, 5, 2 });
	//		ptr += delta;
	//		filter.ProcessData(AudioData{ ptr, 5, 2 });
	//		ptr += delta;
	//		
	//		int a = 42;
	//		Assert::AreEqual(a, 42);


	//	}



	//	TEST_METHOD(TestSplitChannels) {
	//		// TODO: Your test code here
	//		Filter filter;


	//		std::vector<short> data = GenerateData(20);

	//		data = SplitChannels({ data.data(), 20, 2 });

	//		int a = 42;
	//		Assert::AreEqual(a, 42);


	//	}


	//};
}