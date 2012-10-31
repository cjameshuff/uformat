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

//#define UPRINTF_FLOATS


struct FormatOpts {
    char pre;
#ifndef UPRINTF_TINY
    char pad;
    bool ljust;
    uint8_t width;
#ifdef UPRINTF_FLOATS
    int8_t precision;
#endif // UPRINTF_FLOATS
#endif // UPRINTF_TINY
    
    int nchars;
    int maxlen;
    
    int (*putc_f)(char, void*);
    void * userdata;
};


int putc_linbuf(char c, void * userdata)
{
    char ** buffer = (char **)userdata;
    *(*buffer)++ = c;
    return 0;
}


// Print integer, return address of character after the end
void bfrprint_integer(struct FormatOpts * opts, uint32_t val, int base)
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
    if(len > (opts->maxlen - opts->nchars)) {
        while(opts->nchars < opts->maxlen) {
            opts->putc_f('#', opts->userdata);
            ++(opts->nchars);
        }
        return;
    }
    
#ifndef UPRINTF_TINY
    // if +/- prefix specified and pad character is zero, output prefix immediately
    // START-000DIGITS
    if(opts->pre)
    {
        if(opts->width)
            opts->width -= 1;
        
        if(opts->pad == '0') {
            opts->putc_f(opts->pre, opts->userdata);
            ++(opts->nchars);
        }
    }
    
    // output padding if right-justified
    if(!opts->ljust) {
        for(int j = 0; j < opts->width - digits; ++j) {
            opts->putc_f(opts->pad, opts->userdata);
            ++(opts->nchars);
        }
    }
    
    // if +/- prefix specified and pad character is space, output prefix here
    // START   -DIGITS
    if(opts->pre && opts->pad == ' ') {
        opts->putc_f(opts->pre, opts->userdata);
        ++(opts->nchars);
    }
#else
    // output +/- prefix if specified
    if(opts->pre) {
        opts->putc_f(opts->pre, opts->userdata);
        ++(opts->nchars);
    }
#endif // UPRINTF_TINY
    
    // Output integer digits
    for(int d = 0; d < digits; ++d) {
        int x = val/maxval;
        val -= x*maxval;
        maxval /= base;
        opts->putc_f((x < 10)? ('0' + x) : ('A' + x - 10), opts->userdata);
        ++(opts->nchars);
    }
    
#ifndef UPRINTF_TINY
    // If left-justified, emit trailing fill characters
    if(opts->ljust) {
        for(int j = 0; j < opts->width - digits; ++j) {
            opts->putc_f(' ', opts->userdata);
            ++(opts->nchars);
        }
    }
#endif // UPRINTF_TINY
}


int vprintf_chars(int (*putc_f)(char, void*), void * userdata, int len, const char * format, va_list args)
{
    struct FormatOpts opts;
    opts.putc_f = putc_f;
    opts.userdata = userdata;
    opts.nchars = 0;
    opts.maxlen = len;
    
    while(*format != '\0' && opts.nchars < len)
    {
        if(*format != '%') {
            putc_f(*format++, userdata);
            ++(opts.nchars);
            continue;
        }
        
        format++;// skip over '%'
        opts.pre = '\0';
#ifndef UPRINTF_TINY
        // Parse formatting options if not compiled in tiny mode
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
        
#ifdef UPRINTF_FLOATS
        // parse precision (only applicable for floats/fixpoint)
        opts.precision = 8;
        if(*format == '.') {
            opts.precision = 0;
            ++format;
            while(isdigit(*format))
                opts.precision = opts.precision*10 + *format++ - '0';
        }
#endif // UPRINTF_FLOATS
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
#ifdef UPRINTF_FLOATS
        else if(*format == 'e') {
            // Longs not implemented (yet)
            ++format;
        }
        else if(*format == 'f') {
            // Longs not implemented (yet)
            ++format;
        }
#endif // UPRINTF_FLOATS
        else if(*format == 'c') {
            putc_f(va_arg(args, int), userdata);
            ++(opts.nchars);
            ++format;
        }
        else if(*format == '%') {
            putc_f('%', userdata);
            ++(opts.nchars);
            ++format;
        }
        else if(*format == 'N') {
            putc_f('\0', userdata);
            ++(opts.nchars);
            ++format;
        }
        else if(*format == 's') {
            char * src = va_arg(args, char *);
            while(*src && opts.nchars < len) {
                putc_f(*src++, userdata);
                ++(opts.nchars);
            }
            ++format;
        }
#ifdef UPRINTF_TINY
        else if(*format == 'C') {
            ++format;
            while(opts.nchars < len) {
                putc_f(*format, userdata);
                ++(opts.nchars);
            }
            ++format;
        }
#else
        else if(*format == 'C')
        {
            ++format;
            if(opts.width > 0)
            {
                for(int j = 0; j < opts.width && opts.nchars < len; ++j) {
                    putc_f(*format, userdata);
                    ++(opts.nchars);
                }
            }
            else
            {
                while(opts.nchars < len) {
                    putc_f(*format, userdata);
                    ++(opts.nchars);
                }
            }
            ++format;
        }
#endif // UPRINTF_TINY
    }
    return opts.nchars;
}

int bfrprintf(char * bfr, int len, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    int nchars = vprintf_chars(putc_linbuf, (void*)&bfr, len, format, args);
    va_end(args);
    return nchars;
}


#ifdef TEST_UFORMAT

void CheckOverrun(char * buffer, int len, int nchars)
{
    if(nchars > len)
    {
        printf("Overran end of buffer!\n");
        exit(EXIT_FAILURE);
    }
}

void CheckResult(char * buffer, int nchars, char * expected)
{
    CheckOverrun(buffer, strlen(expected) + 1, nchars);
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
    int nchars = 0;
    
    nchars = bfrprintf(buffer, 256, "Foo%%%N");
    CheckResult(buffer, nchars, "Foo%\0");
    
    
    nchars = bfrprintf(buffer, 256, "Foo%d%N", 1);
    CheckResult(buffer, nchars, "Foo1\0");
    nchars = bfrprintf(buffer, 256, "Foo%d%N", 9);
    CheckResult(buffer, nchars, "Foo9\0");
    nchars = bfrprintf(buffer, 256, "Foo%d%N", 123456789);
    CheckResult(buffer, nchars, "Foo123456789\0");
    nchars = bfrprintf(buffer, 256, "Foo%d%N", 10);
    CheckResult(buffer, nchars, "Foo10\0");
    nchars = bfrprintf(buffer, 256, "Foo%4d%N", 10);
    CheckResult(buffer, nchars, "Foo  10\0");
    nchars = bfrprintf(buffer, 256, "Foo%04d%N", 10);
    CheckResult(buffer, nchars, "Foo0010\0");
    nchars = bfrprintf(buffer, 256, "Foo%-4d%N", 10);
    CheckResult(buffer, nchars, "Foo10  \0");
    nchars = bfrprintf(buffer, 256, "Foo%-04d%N", 10);
    CheckResult(buffer, nchars, "Foo10  \0");
    nchars = bfrprintf(buffer, 256, "Foo%-+4d%N", 10);
    CheckResult(buffer, nchars, "Foo+10 \0");
    
    nchars = bfrprintf(buffer, 256, "Foo%d%N", -1);
    CheckResult(buffer, nchars, "Foo-1\0");
    nchars = bfrprintf(buffer, 256, "Foo%d%N", -9);
    CheckResult(buffer, nchars, "Foo-9\0");
    nchars = bfrprintf(buffer, 256, "Foo%d%N", -10);
    CheckResult(buffer, nchars, "Foo-10\0");
    nchars = bfrprintf(buffer, 256, "Foo%4d%N", -10);
    CheckResult(buffer, nchars, "Foo -10\0");
    nchars = bfrprintf(buffer, 256, "Foo%04d%N", -10);
    CheckResult(buffer, nchars, "Foo-010\0");
    nchars = bfrprintf(buffer, 256, "Foo%-4d%N", -10);
    CheckResult(buffer, nchars, "Foo-10 \0");
    nchars = bfrprintf(buffer, 256, "Foo%-+4d%N", -10);
    CheckResult(buffer, nchars, "Foo-10 \0");
    
    nchars = bfrprintf(buffer, 256, "Foo%X%N", 0x12345678);
    CheckResult(buffer, nchars, "Foo12345678\0");
    nchars = bfrprintf(buffer, 256, "Foo%X%N", 0x9ABCDEF0);
    CheckResult(buffer, nchars, "Foo9ABCDEF0\0");
    
    nchars = bfrprintf(buffer, 256, "Foo%4C&%N", 10);
    CheckResult(buffer, nchars, "Foo&&&&\0");
    
    nchars = bfrprintf(buffer, 256, "%s %s%N", "red", "truck");
    CheckResult(buffer, nchars, "red truck\0");
    
    return EXIT_SUCCESS;
}

#endif

