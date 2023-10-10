#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>

static void* ( *real_malloc )( size_t size );
static void  ( *real_free )( void *ptr );


__attribute__( ( constructor ) )
static void init()
{
    real_malloc = dlsym( RTLD_NEXT, "malloc" );
    real_free   = dlsym( RTLD_NEXT, "free" );
}

void mem_din_set( void ( *malloc_fcn )( size_t size ),void free_fcn( void *ptr )  )
{
    if ( malloc_fcn != NULL )
    {
        real_malloc = malloc_fcn;
    }
    else
    {
        real_malloc = dlsym( RTLD_NEXT, "malloc" );
    }

    if ( free_fcn != NULL )
    {
        real_free = free_fcn;
    }
    else
    {
        real_free = dlsym( RTLD_NEXT, "free" );
    }
}

void *malloc( size_t size )
{
    void *ptr = real_malloc( size );
    // fprintf(stderr, "malloc(%zd) = %p\n", size, ptr);
    return ptr;
}

void free( void *ptr )
{
    real_free( ptr );
    // fprintf(stderr, "free(%p)\n", ptr);
}
