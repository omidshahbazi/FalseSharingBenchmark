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
const uint64_t Steps = 100'000'000;

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
	AlignAsUnpacked DataType Data17;
	AlignAsUnpacked DataType Data18;
	AlignAsUnpacked DataType Data19;
	AlignAsUnpacked DataType Data20;
	AlignAsUnpacked DataType Data21;
	AlignAsUnpacked DataType Data22;
	AlignAsUnpacked DataType Data23;
	AlignAsUnpacked DataType Data24;
	AlignAsUnpacked DataType Data25;
	AlignAsUnpacked DataType Data26;
	AlignAsUnpacked DataType Data27;
	AlignAsUnpacked DataType Data28;
	AlignAsUnpacked DataType Data29;
	AlignAsUnpacked DataType Data30;
	AlignAsUnpacked DataType Data31;
	AlignAsUnpacked DataType Data32;
};

const uint8_t MaxSharedDataCount = sizeof(MultiCacheLineSharedData) / MemberAlignment;

struct AlignAsPacked SingleCacheLineSharedData
{
public:
	DataType Data[MaxSharedDataCount];
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
	multiCacheLineSharedData.Data1 = 100;
	DataType* multiCacheLineSharedDataPtr[MaxSharedDataCount];
	for (uint8_t i = 0; i < MaxSharedDataCount; ++i)
		multiCacheLineSharedDataPtr[i] = &multiCacheLineSharedData.Data1 + (i * MemberAlignment);

	cout << "Multi Cache Line" << endl;
	nanoseconds multiTime = Run(multiCacheLineSharedDataPtr);

	cout << "Ratio: " << 1.0 * singleTime / multiTime << endl;
}