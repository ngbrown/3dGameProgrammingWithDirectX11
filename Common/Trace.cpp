#include "Trace.h"
#include <Strsafe.h>

bool _trace(TCHAR *format, ...)
{
	TCHAR buffer[1000];

	va_list argptr;
	va_start(argptr, format);
	StringCchVPrintfW(buffer, 1000, format, argptr);
	va_end(argptr);

	OutputDebugString(buffer);

	return true;
}