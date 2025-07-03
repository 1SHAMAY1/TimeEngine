#pragma once

#ifdef _WIN32
  #ifdef TE_BUILD_DLL       
    #define LOGGER_API __declspec(dllexport)
  #else
    #define LOGGER_API __declspec(dllimport)
  #endif
#else
  #define LOGGER_API
#endif
