Micro format strings implementation.

Written to be very lightweight. As compiled for Cortex-M3 with GCC 4.6.2 from summon-arm-toolchain, it currently comes out to 470 B with formatting, 292 B in minimal form.

All functions are reentrant.

# Functions

## bfrprintf()

    int bfrprintf(char * buffer, int maxlen, const char * format, ...);

Prints format string to buffer, and returns number of characters written.
Usage is very similar to snprintf(), but does not append a null character...only the contents of the format string are written, allowing it to be used to write into a buffer without overwriting bits with null characters.

## vprintf_chars()

    int vprintf_chars(int (*putc_f)(char, void*), void * optdata, int maxlen, const char * format, va_list args);

Generic formatted string implementation, prints up to maxlen characters using specified putc function and returns number of characters actually written. optdata is passed as second parameter to given putc function. Takes a va_list to allow integration with user code.


# Formatting


## Supported format operators:

* `%d`, `%i`: signed decimal integer
* `%u`: unsigned decimal integer
* `%x`, `%X`: unsigned hexadecimal integer
* `%c`: character
* `%s`: string
* `%%`: percent character

And a couple non-standard additions:

* `%N`: explicit null character
* `%C` Character span.

If formatting is enabled, field width, justification, and padding are implemented. Width must be less than 256.

If no width is specified, the %C operator will fill to the end of the buffer, making it convenient for blanking the remainder of a line. For example:

    bfrprintf(lcdBfr+16, 16, "ST: %u%C ", systicks);


# Tests

Some basic tests are embedded in uformat.c, and a test application can be built as follows:

    gcc uformat.c -std=gnu99 -DTEST_UFORMAT -o uformat_test
