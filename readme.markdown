Micro format strings implementation.
As compiled for Cortex-M3 ARM with 4.5.0, it currently comes out to 450 B with formatting, 262 B in minimal form.

    char * bfrprintf(char * buffer, int len, const char * format, ...);

Prints format string to buffer, and returns address of character after the end.
Usage is very similar to snprintf(), but does not append a null character...only the contents of the format string are written, allowing it to be used to write into a buffer without overwriting bits with null characters.

To allow null characters to be inserted explicitly, the %N format operator has been added.

Another addition is a %C operator, used to fill a span with a particular character. If no width is specified, the %C operator will fill to the end of the buffer, making it convenient for blanking the remainder of a line. For example:

    bfrprintf(lcdBfr+16, 16, "ST: %u%C ", systicks);

Some basic tests are embedded in uformat.c, and a test application can be built as follows:

    gcc uformat.c -std=gnu99 -DTEST_UFORMAT -o uformat_test
