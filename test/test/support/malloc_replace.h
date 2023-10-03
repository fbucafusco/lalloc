#pragma once
#include <stdio.h>


void mem_din_set( void ( *malloc_fcn )( size_t size ),void free_fcn( void *ptr )  );