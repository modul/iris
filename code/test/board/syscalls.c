/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2009, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
  * \file syscalls.c
  *
  * Implementation of newlib syscall.
  *
  */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/


#include "board.h"

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

/*----------------------------------------------------------------------------
 *        Exported variables
 *----------------------------------------------------------------------------*/

#undef errno
extern int errno ;
extern int  _end ;

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/
extern int _read(int file, char *ptr, int len)
{
	switch (file)
	{
#ifdef read_stdin
		case STDIN_FILENO:
			len = read_stdin(ptr, len);
		break;
#endif
#ifdef read_stderr
		case STDERR_FILENO: // For convenience, use stderr both ways
			len = read_stderr(ptr, len);
		break;
#endif
		default:
			errno = EBADF;
			return -1;
	}
	return len;
}

extern int _write( int file, char *ptr, int len )
{
	switch (file)
	{
#ifdef write_stdout
		case STDOUT_FILENO:
			len = write_stdout(ptr, len);
		break;
#endif
#ifdef write_stderr
		case STDERR_FILENO:
			len = write_stderr(ptr, len);
		break;
#endif
		default:
			errno = EBADF;
			return -1;
	}
	return len;
}

/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 */
extern caddr_t _sbrk ( int incr )
{
    static unsigned char *heap = NULL ;
    unsigned char *prev_heap ;

    if ( heap == NULL )
    {
        heap = (unsigned char *)&_end ;
    }
    prev_heap = heap;

    heap += incr ;

    return (caddr_t) prev_heap ;
}


/*
 isatty
 Query whether output stream is a terminal. 
 For consistency with the other minimal implementations:
 */
extern int _isatty( int file )
{
	switch (file) {
#if defined(read_stdin)
		case STDIN_FILENO:
#elif defined(read_stderr) | defined(write_stderr)
		case STDERR_FILENO:
#elif defined(write_stdout)
		case STDOUT_FILENO:
			return 1;
		break;
#endif
		default:
			errno = EBADF;
			return 0;
	}
}

/*
 exit
 Leave the program.
 */
extern void _exit( int status )
{
    printf( "Exiting with status %d.\n", status ) ;

    for ( ; ; ) ;
}

/*
 lseek
 Set position in a file. Minimal implementation:
 */
extern int _lseek( int file, int ptr, int dir )
{
    return 0 ;
}

/*
 kill
 Send a signal. Minimal implementation:
 */
extern int _kill( int pid, int sig )
{
	errno = EINVAL;
    return -1; 
}

/*
 getpid
 Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
 */
extern int _getpid ( void )
{
    return -1 ;
}

extern int _link( char *old, char *new )
{
    return -1 ;
}

extern int _close( int file )
{
    return -1 ;
}

extern int _fstat( int file, struct stat *st )
{
    st->st_mode = S_IFCHR ;

    return 0 ;
}
