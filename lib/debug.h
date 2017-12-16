#ifndef INCLUDED_DEBUG_H
#define INCLUDED_DEBUG_H

#include <stdio.h>
#include <stdarg.h>

#define DEBUG_LEVEL 1
#define TRACE_LEVEL 2
#define INFO_LEVEL 3
#define WARN_LEVEL 4
#define ERROR_LEVEL 5
#define FATAL_LEVEL 6

/**
 * I believe this implementation to be somewhat speedy.
 * Calls are only made if DEBUG is less than the
 * requested level to be printed
 */

/** From the  GCC manual on the use of ##
 *
 * if the variable arguments are omitted or empty,
 *  the `##' operator causes the preprocessor to remove the comma before it.
 */

#ifdef MIN_DEBUG_LEVEL
#define DEBUG
#ifndef MAX_DEBUG_LEVEL
#define MAX_DEBUG_LEVEL MIN_DEBUG_LEVEL
#endif // MAX_DEBUG_LEVEL
#endif // MIN_DEBUG_LEVEL

#ifdef MAX_DEBUG_LEVEL
#define DEBUG
#ifndef MIN_DEBUG_LEVEL
#define MIN_DEBUG_LEVEL MAX_DEBUG_LEVEL
#endif // MIN_DEBUG_LEVEL
#endif // MAX_DEBUG_LEVEL

#ifdef DEBUG
extern void debugx(const char *file, const char *function, const int line, int level, const char *message, ...);
#define debug(level, message, ...) debugx(__FILE__, __FUNCTION__, __LINE__, level, message, ##__VA_ARGS__)
#else
#define debugx(level, message, ...)
#endif // DEBUG

#if MIN_DEBUG_LEVEL <= DEBUG_LEVEL && MAX_DEBUG_LEVEL >= DEBUG_LEVEL
#define dDebug(message, ...)  debug(DEBUG_LEVEL, message, ## __VA_ARGS__)
#else
#define dDebug(message, ...)
#endif

#if MIN_DEBUG_LEVEL <= TRACE_LEVEL && MAX_DEBUG_LEVEL >= TRACE_LEVEL
#define dTrace(message, ...)  debug(TRACE_LEVEL, message, ## __VA_ARGS__)
#else
#define dTrace(message, ...)
#endif

#if MIN_DEBUG_LEVEL <= INFO_LEVEL && MAX_DEBUG_LEVEL >= INFO_LEVEL
#define dInfo(message, ...)   debug(INFO_LEVEL, message,  ##__VA_ARGS__)
#else
#define dInfo(message, ...)
#endif

#if MIN_DEBUG_LEVEL <= WARN_LEVEL && MAX_DEBUG_LEVEL >= WARN_LEVEL
#define dWarn(message, ...)   debug(WARN_LEVEL, message,  ##__VA_ARGS__)
#else
#define dWarn(message, ...)
#endif

#if MIN_DEBUG_LEVEL <= ERROR_LEVEL && MAX_DEBUG_LEVEL >= ERROR_LEVEL
#define dError(message, ...)  debug(ERROR_LEVEL, message, ##__VA_ARGS__)
#else
#define dError(message, ...)
#endif

#if MIN_DEBUG_LEVEL <= FATAL_LEVEL && MAX_FATAL_LEVEL >= ERROR_LEVEL
#define dFatal(message, ...)  debug(FATAL_LEVEL, message, ##__VA_ARGS__)
#else
#define dFatal(message, ...)
#endif

#endif	/* INCLUDED_DEBUG_H */
