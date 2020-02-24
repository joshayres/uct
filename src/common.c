#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define CLAMP_MAX(x, max) MIN(x, max)
#define CLAMP_MIN(x, min) MAX(x, min)
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define ALIGN_DOWN(n, a) ((n) & ~((a) -1))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))

#define typeof __typeof__

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

const char* read_file(const char* filename)
{
    char* buffer = 0;
    long length;
    FILE* f= fopen(filename, "r");

    size_t read_length;

    if(f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = (char*)malloc(length + 1);
        if(buffer)
        {
            read_length = fread(buffer, 1, length, f);
        }
        fclose(f);

        buffer[length] = '\0';
    }
    else
    {
        printf("error: File not found: %s", filename);
        return false;
    }
    return buffer;
}

int str_len(const char* s)
{
    int i;
    for(i = 0; s[i] != '\0'; ++i);
    return i;
}
// Stretchy buffers, invented (?) by Sean Barrett

typedef struct BufHdr
{
    size_t len;
    size_t cap;
    char buf[];
} BufHdr;

#define buf__hdr(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_end(b) ((b) + buf_len(b))
#define buf_sizeof(b) ((b) ? buf_len(b)*sizeof(*b) : 0)

#define buf_free(b) ((b) ? (free(buf__hdr(b)), (b) = NULL) : 0)
#define buf_fit(b, n) ((n) <= buf_cap(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define buf_push(b, ...) (buf_fit((b), 1 + buf_len(b)), (b)[buf__hdr(b)->len++] = (__VA_ARGS__))
#define buf_printf(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define buf_clear(b) ((b) ? buf__hdr(b)->len = 0 : 0)

void* buf__grow(const void* buf, size_t new_len, size_t elem_size)
{
    assert(buf_cap(buf) <= (SIZE_MAX - 1)/2);
    size_t new_cap = CLAMP_MIN(2*buf_cap(buf), MAX(new_len, 16));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf))/elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap*elem_size;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = realloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = malloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

char* buf__printf(char* buf, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t cap = buf_cap(buf) - buf_len(buf);
    size_t n = 1 + vsnprintf(buf_end(buf), cap, fmt, args);
    va_end(args);
    if (n > cap) {
        buf_fit(buf, n + buf_len(buf));
        va_start(args, fmt);
        size_t new_cap = buf_cap(buf) - buf_len(buf);
        n = 1 + vsnprintf(buf_end(buf), new_cap, fmt, args);
        assert(n <= new_cap);
        va_end(args);
    }
    buf__hdr(buf)->len += n - 1;
    return buf;
}

#define each(item, buf) \
    (typeof(*(buf)) *p = (buf), item = *p; p < &((buf)[buf_len(buf)]); p++, (item) = *p)


typedef struct
{
    size_t len;
    const char* str;
} intern;

static intern* interns;

const char* str_intern_range(const char* start, const char* end)
{
    size_t len = end - start;
    for(intern *it = interns; it != buf_end(interns); it++)
    {
        if(it->len == len && strncmp(it->str, start, len) == 0)
        {
            return it->str;
        }
    }
    char *str = malloc(len + 1);
    memcpy(str, start, len);
    str[len] = 0;
    buf_push(interns, (intern){len, str});
    return str;
}

const char* str_intern(const char* str)
{
    return str_intern_range(str, str + strlen(str));
}

typedef struct
{
    char *ptr;
    char *end;
    char **blocks;
}arena;

#define ARENA_ALIGNMENT 8
#define ARENA_BLOCK_SIZE 1024

void arena_grow(arena* arena, size_t min_size)
{
    size_t size = ALIGN_UP(MAX(ARENA_BLOCK_SIZE, min_size), ARENA_ALIGNMENT);
    arena->ptr = malloc(size);
    assert(arena->ptr == ALIGN_DOWN_PTR(arena->ptr, ARENA_ALIGNMENT));
    arena->end = arena->ptr + size;
    buf_push(arena->blocks, arena->ptr);
}

void *arena_alloc(arena* arena, size_t size)
{
    if(size > (size_t)(arena->end - arena->ptr))
    {
        arena_grow(arena, size);
    }
    void* ptr = arena->ptr;
    arena->ptr = ALIGN_UP_PTR(arena->ptr + size, ARENA_ALIGNMENT);
    return ptr;
}

void syntax_error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("Syntax Error: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

void fatal_syntax_error(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("Syntax Error: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    exit(1);
}
