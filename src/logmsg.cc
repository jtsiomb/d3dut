#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "logmsg.h"

void fatal_error(const char *fmt, ...)
{
	fputs("D3DUT FATAL ERROR: ", stderr);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	abort();
}

void warning(const char *fmt, ...)
{
	fputs("D3DUT WARNING: ", stderr);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}