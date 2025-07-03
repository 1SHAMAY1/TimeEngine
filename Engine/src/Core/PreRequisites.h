#pragma once

// Standard Library Includes
#include <iostream>        // For basic I/O
#include <memory>          // For smart pointers
#include <utility>         // For std::move, std::swap
#include <algorithm>       // For std::sort, std::max, etc.
#include <functional>      // For std::function, std::bind
#include <string>          // For std::string
#include <vector>          // For std::vector
#include <unordered_map>   // For fast key-value maps
#include <map>             // For ordered maps
#include <set>             // For std::set
#include <unordered_set>   // For fast sets
#include <fstream>         // For file I/O
#include <sstream>         // For stringstream
#include <cassert>         // For assert macro
#include <chrono>          // For timing/profiling
#include <thread>          // For multithreading
#include <mutex>           // For thread safety
#include <atomic>          // For atomic variables

// You can add engine-specific utility includes here

// Platform Detection
#ifdef  TE_PLATFORM_WINDOWS
	#ifdef  TE_BUILD_DLL
		#define TE_API __declspec(dllexport)
	#else
		#define TE_API __declspec(dllimport)
	#endif
#else
	#error Not Supported for this platform!
#endif

// Bit manipulation macro
#define BIT(x) (1 << x)

