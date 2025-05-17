#pragma once

#ifdef  TE_PLATFORM_WINDOWS
	#ifdef  TE_BUILD_DLL
		#define TIMEENGINE_API __declspec(dllexport)
	#else
		#define TIMEENGINE_API __declspec(dllimport)
	#endif
#else
	#error Not Supported for this platform!
#endif


# define BIT(x) (1 << x)