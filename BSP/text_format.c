#include "text_format.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
static void Text_Put(char *out, size_t n, size_t *used, char c)
{
    if (*used + 1 < n)
        out[*used] = c;
    ++*used;
}
size_t Text_Format(char *out, size_t n, const char *f, ...)
{
    if (!out || !n || !f)
        return 0;
    size_t used = 0;
    va_list args;
    va_start(args, f);
    while (*f)
    {
        if (*f != '%')
        {
            Text_Put(out, n, &used, *f++);
            continue;
        }
        ++f;
        if (*f == '%')
        {
            Text_Put(out, n, &used, *f++);
            continue;
        }
        bool zero = *f == '0';
        if (zero)
            ++f;
        unsigned width = 0;
        while (*f >= '0' && *f <= '9')
        {
            width = width * 10 + (unsigned)(*f++ - '0');
            if (width > 32)
                width = 32;
        }
        bool lng = *f == 'l';
        if (lng)
            ++f;
        char kind = *f;
        if (!kind)
            break;
        ++f;
        if (kind == 's')
        {
            const char *s = va_arg(args, const char *);
            if (s)
                while (*s)
                    Text_Put(out, n, &used, *s++);
            continue;
        }
        if (kind != 'd' && kind != 'u' && kind != 'x')
        {
            Text_Put(out, n, &used, '?');
            continue;
        }
        uint32_t v;
        bool negative = false;
        if (kind == 'd')
        {
            long s = lng ? va_arg(args, long) : (long)va_arg(args, int);
            negative = s < 0;
            v = negative ? (uint32_t)(-(s + 1)) + 1U : (uint32_t)s;
        }
        else
            v = lng ? (uint32_t)va_arg(args, unsigned long) : (uint32_t)va_arg(args, unsigned int);
        char digits[32];
        unsigned len = 0, base = kind == 'x' ? 16 : 10;
        do
        {
            digits[len++] = "0123456789abcdef"[v % base];
            v /= base;
        } while (v);
        if (negative)
            Text_Put(out, n, &used, '-');
        for (unsigned i = len + (negative ? 1U : 0U); i < width; ++i)
            Text_Put(out, n, &used, zero ? '0' : ' ');
        while (len)
            Text_Put(out, n, &used, digits[--len]);
    }
    va_end(args);
    out[used < n ? used : n - 1] = 0;
    return used;
}
