#ifndef __MEMSTREAM_HEADER_H__
#define __MEMSTREAM_HEADER_H__

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef unsigned long long qword;

#ifdef _WIN32
  #pragma warning (push)
  #pragma warning (disable : 4035)
#endif

inline long long _rdtsc() { return 1; /* __asm { rdtsc } */ }

#ifdef _WIN32
  #pragma warning(pop)
#endif
#endif

