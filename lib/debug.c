#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

/*
 * In debug.h, there are several debugging levels
 * calling debug() with a level higher than the MAX (DEBUG)
 * causes it to be displayed. the max is usually compiled
 * in during the build process.
 */

#ifdef MIN_DEBUG_LEVEL
#define DEBUG
#endif

#ifdef MAX_DEBUG_LEVEL
#define DEBUG
#endif

#ifdef DEBUG
void
debugx(const char *file, const char *function, int line, int level, const char *message, ...)
{
	va_list ap;

	fprintf(stderr, "%d: %s:%s:%u ", level, file, function, line);
	va_start(ap, message);
	vfprintf(stderr, message, ap);
	va_end(ap);
}
#endif
