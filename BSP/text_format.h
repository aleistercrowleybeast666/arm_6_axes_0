#ifndef __TEXT_FORMAT_H
#define __TEXT_FORMAT_H
#include <stddef.h>
/* Bounded integer/string formatter; supports %s %u %d %x, optional l and zero padding.
 * No stdio state, floating point conversion or allocator. Returns full required length. */
size_t Text_Format(char *out, size_t capacity, const char *format, ...);
#endif
