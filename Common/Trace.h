#pragma once

#include <Windows.h>

bool _trace(TCHAR *format, ...);

#ifdef _DEBUG
#define TRACE _trace
#else
#define TRACE __noop
#endif
