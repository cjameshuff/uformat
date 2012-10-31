//    Copyright (c) 2011, Christopher James Huff <cjameshuff@gmail.com>
//    All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are met:
//        * Redistributions of source code must retain the above copyright
//          notice, this list of conditions and the following disclaimer.
//        * Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in the
//          documentation and/or other materials provided with the distribution.
//        * Neither the name of the copyright holder nor the names of contributors
//          may be used to endorse or promote products derived from this software
//          without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
//    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
//************************************************************************************************

#ifndef UPRINTF_H
#define UPRINTF_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

// #define UPRINTF_LONG_INT   0 // Not implemented yet
// #define UPRINTF_FLOAT      0 // Not implemented yet
// #define UPRINTF_FIXPOINT   0 // Not implemented yet

// No formatting, only %d, %u, %x, %X, %N
// #define UPRINTF_TINY       0

// Usage is very similar to snprintf(), with some important differences.
// Prints format string, and returns number of chars in output string.
// *DOES NOT APPEND NULL CHARACTERS.*
// Differences from standard format strings:
// %width.dividerF: decimal fixpoint (TODO)
// %widthCchar: character run
// %N: explicit null character
// # flag and * for width/precision by arguments are unimplemented
// %x and %X both use upper case letters
int bfrprintf(char * bfr, int len, const char * format, ...);


int vprintf_chars(int (*putc_f)(char, void*), void * optdata, int len, const char * format, va_list args);


#ifdef __cplusplus
}
#endif

#endif // UPRINTF_H
