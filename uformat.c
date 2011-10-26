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

#include "uformat.h"
#include <stdint.h>
#include <stdbool.h>

#define isdigit(x)  ((x) >= '0' && (x) <= '9')

#ifdef TEST_UFORMAT
// gcc uformat.c -std=gnu99 -DTEST_UFORMAT -o uformat_test
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif


struct FormatOpts {
    char * buffer;
    char * bufferEnd;
    char pre;
#ifndef UPRINTF_TINY
    char pad;
    bool ljust;
    int width;
    int precision;
#endif // UPRINTF_TINY
};

// Print integer, return address of character after the end
char * bfrprint_integer(struct FormatOpts * opts, uint32_t val, int base)
{
    int digits = 1, maxval = 1;
    while(digits < 16) {
        int x = maxval*base;
        if(x > val || x < maxval)
            break;
        ++digits;
        maxval = x;
    }
    
    // Check that room remains, write #'s if insufficient room for complete number.
    int len = (opts->pre)? (digits + 1) : (digits);
    if(len > (opts->bufferEnd - opts->buffer)) {
        while(opts->buffer != opts->bufferEnd)
            *opts->buffer++ = '#';
        return opts->buffer;
    }
    
#ifndef UPRINTF_TINY
    if(opts->pre)
    {
        opts->width -= 1;
        if(opts->pad == '0')
            *opts->buffer++ = opts->pre;
    }
    
    if(!opts->ljust) {
        for(int j = 0; j < opts->width - digits; ++j)
            *opts->buffer++ = opts->pad;
    }
    
    if(opts->pre && opts->pad == ' ')
        *opts->buffer++ = opts->pre;
#else
    if(opts->pre)
        *opts->buffer++ = opts->pre;
#endif // UPRINTF_TINY
    
    for(int d = 0; d < digits; ++d) {
        int x = val/maxval;
        val -= x*maxval;
        maxval /= base;
        *opts->buffer++ = (x < 10)? ('0' + x) : ('A' + x - 10);
    }
    
#ifndef UPRINTF_TINY
    if(opts->ljust) {
        for(int j = 0; j < opts->width - digits; ++j)
            *opts->buffer++ = ' ';
    }
#endif // UPRINTF_TINY
    
    return opts->buffer;
}

char * vbfrprintf(char * bfr, int len, const char * format, va_list args)
{
    struct FormatOpts opts;
    opts.buffer = bfr;
    opts.bufferEnd = bfr + len;
    
    while(*format != '\0' && opts.buffer != opts.bufferEnd)
    {
        if(*format != '%') {
            *opts.buffer++ = *format++;
            continue;
        }
        
        format++;// skip over '%'
        opts.pre = '\0';
#ifndef UPRINTF_TINY
        opts.pad = ' ';
        opts.ljust = false;
        
        // parse flags
        while(*format != '\0')
        {
            if(*format == '0') {
                opts.pad = '0';
                ++format;
            }
            else if(*format == ' ') {
                opts.pad = ' ';
                ++format;
            }
            else if(*format == '-') {
                opts.ljust = true;
                ++format;
            }
            else if(*format == '+') {
                opts.pre = '+';
                ++format;
            }
            else break;// not a flag character
        }
        
        // parse minimum width
        opts.width = 0;
        while(isdigit(*format))
            opts.width = opts.width*10 + *format++ - '0';
        
        // parse precision (only applicable for floats/fixpoint)
        opts.precision = 8;
        if(*format == '.') {
            opts.precision = 0;
            ++format;
            while(isdigit(*format))
                opts.precision = opts.precision*10 + *format++ - '0';
        }
#endif // UPRINTF_TINY
        
        char size = '\0';
        if(*format == 'h' || *format == 'l' || *format == 'L')
            size = *format++;
        
        if(*format == 'd' || *format == 'i') {
            int32_t val = va_arg(args, int);
            if(val < 0) {
                opts.pre = '-';
                val = -val;
            }
            // Longs not implemented (yet)
            bfrprint_integer(&opts, val, 10);
            ++format;
        }
        else if(*format == 'u') {
            // Longs not implemented (yet)
            bfrprint_integer(&opts, va_arg(args, unsigned int), 10);
            ++format;
        }
        else if(*format == 'x' || *format == 'X') {
            bfrprint_integer(&opts, va_arg(args, unsigned int), 16);
            ++format;
        }
        else if(*format == 'c') {
            *opts.buffer++ = va_arg(args, int);
            ++format;
        }
        else if(*format == '%') {
            *opts.buffer++ = '%';
            ++format;
        }
        else if(*format == 'N') {
            *opts.buffer++ = '\0';
            ++format;
        }
        else if(*format == 's') {
            char * src = va_arg(args, char *);
            while(*src && opts.buffer != opts.bufferEnd)
                *opts.buffer++ = *src++;
            ++format;
        }
#ifdef UPRINTF_TINY
        else if(*format == 'C') {
            ++format;
            while(opts.buffer != opts.bufferEnd)
                *opts.buffer++ = *format;
            ++format;
        }
#else
        else if(*format == 'C') {
            ++format;
            if(opts.width > 0) {
                for(int j = 0; j < opts.width && opts.buffer != opts.bufferEnd; ++j)
                    *opts.buffer++ = *format;
            }
            else {
                while(opts.buffer != opts.bufferEnd)
                    *opts.buffer++ = *format;
            }
            ++format;
        }
#endif // UPRINTF_TINY
    }
    
    return opts.buffer;
}

char * bfrprintf(char * bfr, int len, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    char * end = vbfrprintf(bfr, len, format, args);
    va_end(args);
    return end;
}


#ifdef TEST_UFORMAT

void CheckOverrun(char * buffer, int len, char * bEnd)
{
    if(bEnd > buffer + len)
    {
        printf("Overran end of buffer!\n");
        exit(EXIT_FAILURE);
    }
}

void CheckResult(char * buffer, char * bEnd, char * expected)
{
    CheckOverrun(buffer, strlen(expected) + 1, bEnd);
    if(strncmp(buffer, expected, strlen(expected)) == 0)
    {
        printf("got expected result: \"%s\"\n", expected);
    }
    else
    {
        printf("did not get expected result: \"%s\"\n", expected);
        printf("instead: \"%s\"\n", buffer);
        // exit(EXIT_FAILURE);
    }
}

int main(int argc, const char * argv[])
{
    char buffer[257];
    char * bEnd = NULL;
    
    bEnd = bfrprintf(buffer, 256, "Foo%%%N");
    CheckResult(buffer, bEnd, "Foo%\0");
    
    
    bEnd = bfrprintf(buffer, 256, "Foo%d%N", 1);
    CheckResult(buffer, bEnd, "Foo1\0");
    bEnd = bfrprintf(buffer, 256, "Foo%d%N", 9);
    CheckResult(buffer, bEnd, "Foo9\0");
    bEnd = bfrprintf(buffer, 256, "Foo%d%N", 123456789);
    CheckResult(buffer, bEnd, "Foo123456789\0");
    bEnd = bfrprintf(buffer, 256, "Foo%d%N", 10);
    CheckResult(buffer, bEnd, "Foo10\0");
    bEnd = bfrprintf(buffer, 256, "Foo%4d%N", 10);
    CheckResult(buffer, bEnd, "Foo  10\0");
    bEnd = bfrprintf(buffer, 256, "Foo%04d%N", 10);
    CheckResult(buffer, bEnd, "Foo0010\0");
    bEnd = bfrprintf(buffer, 256, "Foo%-4d%N", 10);
    CheckResult(buffer, bEnd, "Foo10  \0");
    bEnd = bfrprintf(buffer, 256, "Foo%-04d%N", 10);
    CheckResult(buffer, bEnd, "Foo10  \0");
    bEnd = bfrprintf(buffer, 256, "Foo%-+4d%N", 10);
    CheckResult(buffer, bEnd, "Foo+10 \0");
    
    bEnd = bfrprintf(buffer, 256, "Foo%d%N", -1);
    CheckResult(buffer, bEnd, "Foo-1\0");
    bEnd = bfrprintf(buffer, 256, "Foo%d%N", -9);
    CheckResult(buffer, bEnd, "Foo-9\0");
    bEnd = bfrprintf(buffer, 256, "Foo%d%N", -10);
    CheckResult(buffer, bEnd, "Foo-10\0");
    bEnd = bfrprintf(buffer, 256, "Foo%4d%N", -10);
    CheckResult(buffer, bEnd, "Foo -10\0");
    bEnd = bfrprintf(buffer, 256, "Foo%04d%N", -10);
    CheckResult(buffer, bEnd, "Foo-010\0");
    bEnd = bfrprintf(buffer, 256, "Foo%-4d%N", -10);
    CheckResult(buffer, bEnd, "Foo-10 \0");
    bEnd = bfrprintf(buffer, 256, "Foo%-+4d%N", -10);
    CheckResult(buffer, bEnd, "Foo-10 \0");
    
    bEnd = bfrprintf(buffer, 256, "Foo%X%N", 0x12345678);
    CheckResult(buffer, bEnd, "Foo12345678\0");
    bEnd = bfrprintf(buffer, 256, "Foo%X%N", 0x9ABCDEF0);
    CheckResult(buffer, bEnd, "Foo9ABCDEF0\0");
    
    bEnd = bfrprintf(buffer, 256, "Foo%4C&%N", 10);
    CheckResult(buffer, bEnd, "Foo&&&&\0");
    
    bEnd = bfrprintf(buffer, 256, "%s %s%N", "red", "truck");
    CheckResult(buffer, bEnd, "red truck\0");
    
    return EXIT_SUCCESS;
}

#endif

