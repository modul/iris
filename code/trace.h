#ifndef _TRACE_
#define _TRACE_

#include <stdio.h>

#define TRACE_LEVEL_DEBUG      5
#define TRACE_LEVEL_INFO       4
#define TRACE_LEVEL_WARNING    3
#define TRACE_LEVEL_ERROR      2
#define TRACE_LEVEL_FATAL      1
#define TRACE_LEVEL_NO_TRACE   0

#ifndef TRACE_OUT
#define TRACE_OUT stderr
#endif

/* By default, all traces are output except the debug one. */
#if !defined(TRACE_LEVEL)
#define TRACE_LEVEL TRACE_LEVEL_INFO
#endif

/* By default, trace level is static (not dynamic) */
#if !defined(DYN_TRACES)
#define DYN_TRACES 0
#endif

#if defined(NOTRACE)
#error "Error: NOTRACE has to be not defined !"
#endif

#undef NOTRACE
#if (DYN_TRACES==0)
    #if (TRACE_LEVEL == TRACE_LEVEL_NO_TRACE)
        #define NOTRACE
    #endif
#endif



/* ------------------------------------------------------------------------------
 *         Global Macros
 * ------------------------------------------------------------------------------
 */

/**
 *  Outputs a formatted string using 'printf' if the log level is high
 *  enough. Can be disabled by defining TRACE_LEVEL=0 during compilation.
 *  \param ...  Additional parameters depending on formatted string.
 */
#if defined(NOTRACE)

/* Empty macro */
#define TRACE_DEBUG(...)      { }
#define TRACE_INFO(...)       { }
#define TRACE_WARNING(...)    { }
#define TRACE_ERROR(...)      { }
#define TRACE_FATAL(...)      { while(1); }

#define TRACE_DEBUG_WP(...)   { }
#define TRACE_INFO_WP(...)    { }
#define TRACE_WARNING_WP(...) { }
#define TRACE_ERROR_WP(...)   { }
#define TRACE_FATAL_WP(...)   { while(1); }

#elif (DYN_TRACES == 1)

/* Trace output depends on dwTraceLevel value */
#define TRACE_DEBUG(...)      { if (dwTraceLevel >= TRACE_LEVEL_DEBUG)   { fprintf(TRACE_OUT, "-D- " __VA_ARGS__); } }
#define TRACE_INFO(...)       { if (dwTraceLevel >= TRACE_LEVEL_INFO)    { fprintf(TRACE_OUT, "-I- " __VA_ARGS__); } }
#define TRACE_WARNING(...)    { if (dwTraceLevel >= TRACE_LEVEL_WARNING) { fprintf(TRACE_OUT, "-W- " __VA_ARGS__); } }
#define TRACE_ERROR(...)      { if (dwTraceLevel >= TRACE_LEVEL_ERROR)   { fprintf(TRACE_OUT, "-E- " __VA_ARGS__); } }
#define TRACE_FATAL(...)      { if (dwTraceLevel >= TRACE_LEVEL_FATAL)   { fprintf(TRACE_OUT, "-F- " __VA_ARGS__); while(1); } }

#define TRACE_DEBUG_WP(...)   { if (dwTraceLevel >= TRACE_LEVEL_DEBUG)   { fprintf(TRACE_OUT, __VA_ARGS__); } }
#define TRACE_INFO_WP(...)    { if (dwTraceLevel >= TRACE_LEVEL_INFO)    { fprintf(TRACE_OUT, __VA_ARGS__); } }
#define TRACE_WARNING_WP(...) { if (dwTraceLevel >= TRACE_LEVEL_WARNING) { fprintf(TRACE_OUT, __VA_ARGS__); } }
#define TRACE_ERROR_WP(...)   { if (dwTraceLevel >= TRACE_LEVEL_ERROR)   { fprintf(TRACE_OUT, __VA_ARGS__); } }
#define TRACE_FATAL_WP(...)   { if (dwTraceLevel >= TRACE_LEVEL_FATAL)   { fprintf(TRACE_OUT, __VA_ARGS__); while(1); } }

#else

/* Trace compilation depends on TRACE_LEVEL value */
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
#define TRACE_DEBUG(...)      { fprintf(TRACE_OUT, "-D- " __VA_ARGS__); }
#define TRACE_DEBUG_WP(...)   { fprintf(TRACE_OUT, __VA_ARGS__); }
#else
#define TRACE_DEBUG(...)      { }
#define TRACE_DEBUG_WP(...)   { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_INFO)
#define TRACE_INFO(...)       { fprintf(TRACE_OUT, "-I- " __VA_ARGS__); }
#define TRACE_INFO_WP(...)    { fprintf(TRACE_OUT, __VA_ARGS__); }
#else
#define TRACE_INFO(...)       { }
#define TRACE_INFO_WP(...)    { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
#define TRACE_WARNING(...)    { fprintf(TRACE_OUT, "-W- " __VA_ARGS__); }
#define TRACE_WARNING_WP(...) { fprintf(TRACE_OUT, __VA_ARGS__); }
#else
#define TRACE_WARNING(...)    { }
#define TRACE_WARNING_WP(...) { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_ERROR)
#define TRACE_ERROR(...)      { fprintf(TRACE_OUT, "-E- " __VA_ARGS__); }
#define TRACE_ERROR_WP(...)   { fprintf(TRACE_OUT, __VA_ARGS__); }
#else
#define TRACE_ERROR(...)      { }
#define TRACE_ERROR_WP(...)   { }
#endif

#if (TRACE_LEVEL >= TRACE_LEVEL_FATAL)
#define TRACE_FATAL(...)      { fprintf(TRACE_OUT, "-F- " __VA_ARGS__); while(1); }
#define TRACE_FATAL_WP(...)   { fprintf(TRACE_OUT, __VA_ARGS__); while(1); }
#else
#define TRACE_FATAL(...)      { while(1); }
#define TRACE_FATAL_WP(...)   { while(1); }
#endif

#endif


/**
 *        Exported variables
 */
/** Depending on DYN_TRACES, dwTraceLevel is a modifable runtime variable or a define */
#if !defined(NOTRACE) && (DYN_TRACES == 1)
    extern uint32_t dwTraceLevel ;
#endif

#undef NOTRACE

#endif //#ifndef TRACE_H

