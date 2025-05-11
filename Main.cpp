#include <iomanip>
#include <thread>
#include <chrono>
#include <latch>
#include <mutex>
#include <functional>
#include <iostream>

using namespace std;
using namespace chrono;

#if __cpp_lib_hardware_interference_size
constexpr size_t StructAlignment = hardware_constructive_interference_size;
constexpr size_t MemberAlignment = hardware_destructive_interference_size;
#else
constexpr size_t StructAlignment = 64;
constexpr size_t MemberAlignment = 64;
#endif

#define AlignAsPacked alignas(StructAlignment)
#define AlignAsUnpacked alignas(MemberAlignment)

typedef uint8_t DataType;

//const uint64_t Steps = 1'000'000'000;
const uint64_t Steps = 10'000'000;

const uint8_t MaxSharedDataCount = 16;

struct AlignAsPacked SingleCacheLineSharedData
{
public:
	DataType Data[MaxSharedDataCount];
};

struct AlignAsPacked MultiCacheLineSharedData
{
public:
	AlignAsUnpacked DataType Data1;
	AlignAsUnpacked DataType Data2;
	AlignAsUnpacked DataType Data3;
	AlignAsUnpacked DataType Data4;
	AlignAsUnpacked DataType Data5;
	AlignAsUnpacked DataType Data6;
	AlignAsUnpacked DataType Data7;
	AlignAsUnpacked DataType Data8;
	AlignAsUnpacked DataType Data9;
	AlignAsUnpacked DataType Data10;
	AlignAsUnpacked DataType Data11;
	AlignAsUnpacked DataType Data12;
	AlignAsUnpacked DataType Data13;
	AlignAsUnpacked DataType Data14;
	AlignAsUnpacked DataType Data15;
	AlignAsUnpacked DataType Data16;
};

nanoseconds Run(DataType* volatile* Data)
{
	const uint32_t NumberOfThreads = min<uint32_t>(MaxSharedDataCount, thread::hardware_concurrency());

	latch syncLatch(NumberOfThreads);
	mutex coutMutex;

	nanoseconds sumElapsed = {};

	auto routine = [&syncLatch, &coutMutex, &sumElapsed](DataType volatile& Value)
		{
			syncLatch.arrive_and_wait();

			time_point start = high_resolution_clock::now();

			for (uint64_t i = 0; i < Steps; ++i)
				++Value;

			nanoseconds elapsed = high_resolution_clock::now() - start;
			sumElapsed += elapsed;

			lock_guard lock(coutMutex);

			cout << "Per Thread Average Time: " << elapsed / Steps << " Total: " << duration_cast<milliseconds>(elapsed) << endl;
		};

	vector<thread> threads;
	threads.reserve(NumberOfThreads);

	for (uint32_t i = 0; i < NumberOfThreads; ++i)
		threads.push_back(thread(routine, ref(*Data[i])));

	for (uint32_t i = 0; i < NumberOfThreads; ++i)
		threads[i].join();

	cout << "Average Time: " << sumElapsed / (Steps * NumberOfThreads) << " Total: " << duration_cast<milliseconds>(sumElapsed) << endl;

	cout << endl;

	return sumElapsed;
}

int main()
{
	SingleCacheLineSharedData singleCacheLineSharedData = {};
	DataType* singleCacheLineSharedDataPtr[MaxSharedDataCount];
	for (uint8_t i = 0; i < MaxSharedDataCount; ++i)
		singleCacheLineSharedDataPtr[i] = &singleCacheLineSharedData.Data[i];

	cout << "Single Cache Line" << endl;
	nanoseconds singleTime = Run(singleCacheLineSharedDataPtr);



	MultiCacheLineSharedData multiCacheLineSharedData = {};
	DataType* multiCacheLineSharedDataPtr[MaxSharedDataCount];
	for (uint8_t i = 0; i < MaxSharedDataCount; ++i)
		multiCacheLineSharedDataPtr[i] = &multiCacheLineSharedData.Data1 + (i * MemberAlignment);

	cout << "Multi Cache Line" << endl;
	nanoseconds multiTime = Run(multiCacheLineSharedDataPtr);

	cout << "Ratio: " << 1.0 * singleTime / multiTime << endl;
}