// This file is autogenerated. See look at the .lzz files in the src/ directory for a more human-friendly version
#ifndef LZZ_OVERRIDE
	#define LZZ_OVERRIDE override
#endif
// example.hh
//

#ifndef LZZ_example_hh
#define LZZ_example_hh
#define STT_STL_DEBUG 1
#define STT_STL_DEBUG_MEMORY 1
#include "stt-stl.h"
#define STT_STL_IMPL 1 // needed here for .lzz to include #src
#define LZZ_INLINE inline
int main (int argc, char * * argv);
#undef LZZ_INLINE
#endif
#undef LZZ_OVERRIDE

////////////////////////////////////////////////////////////////////////

#ifdef STT_STL_IMPL
#ifndef STT_STL_IMPL_DOUBLE_GUARD_example
#define STT_STL_IMPL_DOUBLE_GUARD_example
#define LZZ_OVERRIDE
// example.cpp
//

#include <cstdio>
#include <iostream>
#include <type_traits>

#define STT_STL_IMPL 1
#include "stt-stl.h"

#include <string>
#include <vector>

#define ANKERL_NANOBENCH_IMPLEMENT
#include "3rd_party/nanobench.h"
#include "benchmark_tests.h"

namespace stt {
	template <typename T>
	using small_vector64 = small_vector<T, 64, uint16_t>;
	}
#define LZZ_INLINE inline
int main (int argc, char * * argv)
                                  {
	//printf("Hello\n");
			
	#if 1
	for (int testId = 0; testId < 64; ++testId) {
		ankerl::nanobench::Bench b;
		//b.clockResolutionMultiple(10); // fast
		b.clockResolutionMultiple(10000000); // accurate
		b.relative(true);
		b.performanceCounters(true);
		
		b.title(stt_benchmarks::vectorTests<stt_benchmarks::nullContainer, std::vector>(&b, NULL, testId));
		stt_benchmarks::vectorTests<void, std::vector>(&b, "std::vector", testId);
		stt_benchmarks::vectorTests<void, stt::vector24>(&b, "stt::vector24", testId);
		
		stt_benchmarks::vectorTests<void, stt::small_vector64>(&b, "stt::small_vector<T, 64, uint16_t>", testId);
		}
	#endif
	
	#if 1
	for (int testId = 0; testId < 64; ++testId) {
		ankerl::nanobench::Bench b;
		//b.clockResolutionMultiple(10); // fast
		b.clockResolutionMultiple(10000000); // accurate
		b.relative(true);
		b.performanceCounters(true);
		
		b.title(stt_benchmarks::stringTests<stt_benchmarks::nullContainer, std::string>(&b, NULL, testId));
		stt_benchmarks::stringTests<void, std::string>(&b, "std::string", testId);
		stt_benchmarks::stringTests<void, stt::string24>(&b, "stt::string24", testId);
		stt_benchmarks::stringTests<void, stt::string64>(&b, "stt::string64", testId);
		}
	#endif
	
	return 0;
	}
#undef LZZ_INLINE
#undef LZZ_OVERRIDE
#endif //STT_STL_IMPL_DOUBLE_GUARD_example
#endif //STT_STL_IMPL_IMPL
