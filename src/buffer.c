#include <collections/buffer.h>
#include <collections/memmove_safe.h>

GENERIC_BUFFER_DEFINITION(i, int, 0)
GENERIC_BUFFER_DEFINITION(ui, unsigned int, 0)
GENERIC_BUFFER_DEFINITION(f, float, 0.0f)
GENERIC_BUFFER_DEFINITION(b, unsigned char, '\0')
GENERIC_BUFFER_DEFINITION(d, double, 0.0)
GENERIC_BUFFER_DEFINITION(l, long long int, 0L)
GENERIC_BUFFER_DEFINITION(s, short int, 0)
GENERIC_BUFFER_DEFINITION(ptr, memory_t, nullptr)
GENERIC_BUFFER_DEFINITION(c, char, '\0')
GENERIC_BUFFER_DEFINITION(ul, unsigned long long int, 0L)
GENERIC_BUFFER_DEFINITION(us, unsigned short int, 0)
GENERIC_BUFFER_DEFINITION(z, size_t, 0)
GENERIC_BUFFER_DEFINITION(str, char *, "")