/*
BSD 3-Clause License

Copyright (c) 2018, Franco Bucafusco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Project dependant configuration : the user should create this file in its project */
#include "lalloc_config.h"

#define LALLOC_VERSION           020

/* DEFAULT VALUES: can be changed in  lalloc_config.h =========================================================================== */

#ifndef LALLOC_ASSERT
#define LALLOC_ASSERT(a)
#endif

/**
   @brief it defines the alignment required to access the data.
          e.g 2 -> the first byte of any block will be aligned with an address multiple of 2
          options are 1, 2, 4 and 8.
 */
#ifndef LALLOC_ALIGNMENT
#define LALLOC_ALIGNMENT         4
#endif

/**
   @brief defines the maximum ammount of byte of the pool for each instance.
 */
#ifndef LALLOC_MAX_BYTES
#define LALLOC_MAX_BYTES         0xFFFF
#endif

/* CONDITIONALS ========================================================================================================== */

/**
   @brief   If lalloc_config.h defines LALLOC_CRITICAL_START, LALLOC_CRITICAL_END
            LALLOC_THREAD_SAFE is defined as 2, meaning that the critical section mechanism will be based on other mechanism than mutex ( disable/enable isr, e.g. )
            In this case, the RAM footprint will include the mutex object handle.
 */
#if defined(LALLOC_CRITICAL_START) && defined(LALLOC_CRITICAL_END)
#define LALLOC_THREAD_SAFE       2
#endif

/**
   @brief   If lalloc_config.h defines LALLOC_MUTEX_INIT, LALLOC_MUTEX_LOCK, LALLOC_MUTEX_UNLOCK
            LALLOC_THREAD_SAFE is defined as 1, meaning that the critical section mechanism will be based on mutex.
            In this case, the RAM footptinf will include the mutex object handle.
 */
#if defined(LALLOC_MUTEX_INIT)&&defined(LALLOC_MUTEX_LOCK)&&defined(LALLOC_MUTEX_UNLOCK)&&defined(LALLOC_MUTEX_TYPE)
#define LALLOC_THREAD_SAFE       1
#define LALLOC_CRITICAL_INIT     LALLOC_MUTEX_INIT(obj->din.mutex)
#define LALLOC_CRITICAL_START    LALLOC_MUTEX_LOCK(obj->din.mutex)
#define LALLOC_CRITICAL_END      LALLOC_MUTEX_UNLOCK(obj->din.mutex)
#endif

/**
   @brief   LALLOC_NO_FLAGS
            Default value for passing to the LALLOC_DECLARE macro if no behaviors are needed for a certain instance.
 */
#define LALLOC_NO_FLAGS          0

/**
   @brief   Based on LALLOC_MAX_BYTES it defines the data type for the indexing of bytes and blocks
 */
#if defined(LALLOC_MAX_BYTES) && !defined(LALLOC_IDX_TYPE)
#if( LALLOC_MAX_BYTES<=0xFF )
#define LALLOC_IDX_TYPE                uint8_t
#elif( LALLOC_MAX_BYTES<=0xFFFF )
#define LALLOC_IDX_TYPE                uint16_t
#elif( LALLOC_MAX_BYTES<=0xFFFFFFFF )
#define LALLOC_IDX_TYPE                uint32_t
#endif
#endif

/**
   @brief   LALLOC_IDX_INVALID
            defines the invalid value for all the variables or members of type LALLOC_IDX_TYPE
*/
#define LALLOC_IDX_INVALID              ((LALLOC_IDX_TYPE)(~((LALLOC_IDX_TYPE)0)))

#if LALLOC_ALIGNMENT==1
#define LALLOC_ALIGN_TYPE                uint8_t
#elif LALLOC_ALIGNMENT==2
#define LALLOC_ALIGN_TYPE                uint16_t
#elif LALLOC_ALIGNMENT==4
#define LALLOC_ALIGN_TYPE                uint32_t
#elif LALLOC_ALIGNMENT==8
#define LALLOC_ALIGN_TYPE                uint64_t
#else
#error "LALLOC_ALIGN_TYPE: ALIGNMENT not supported"
#endif

/**
   @brief   LALLOC_SIZE_ROUND_UP
            rounds up the size to the next multiple of the size of the type passed as parameter
 */
#define LALLOC_SIZE_ROUND_UP( TYPE, SIZE) ((sizeof(TYPE) == 1) ? (SIZE) : ( (SIZE) + (sizeof(TYPE) - 1) ) & ~(sizeof(TYPE) - 1))

/**
   @brief General macros for adjusting sizes and addresses
*/
#define LALLOC_ALIGN_ROUND_UP(SIZE)     LALLOC_SIZE_ROUND_UP( LALLOC_ALIGN_TYPE , SIZE  )

/* STRUCTURES ============================================================================================================ */
typedef struct
{
    LALLOC_IDX_TYPE flist;              // Index (in bytes) to the first block to be freed (1st byte of the header). Points to the block with the largest size.
    LALLOC_IDX_TYPE alist;              // Index (in bytes) to the first block to be allocated (1st byte of the header).
    LALLOC_IDX_TYPE alloc_block;        // Allocated block, which can be calculated by looking at flist to see if it has the "free" bit or not.
    LALLOC_IDX_TYPE allocated_blocks;   // Count of allocated blocks. It avoids having to iterate through the alist elements.

#if LALLOC_THREAD_SAFE==1
    LALLOC_MUTEX_TYPE mutex;            // Mutex to ensure thread safety, if enabled.
#endif
} lalloc_dyn_t;

typedef struct
{
    uint8_t*            pool;       // Points to the RAM memory area where data will be stored.
    LALLOC_IDX_TYPE     size;       // Size of the memory area pointed to by "pool."
    lalloc_dyn_t*       dyn;        // Pointer to the RAM area to store dynamic variables of the queue.
} lalloc_t;

/* FUNCTIONAL MACROS ===================================================================================================== */
#ifndef LALLOC_RAM_ATTRIBUTES
#define LALLOC_RAM_ATTRIBUTES
#endif

#ifndef LALLOC_ROM_ATTRIBUTES
#define LALLOC_ROM_ATTRIBUTES           const
#endif

#ifndef LALLOC_CONST_OBJ_ATTRIBUTES
#define LALLOC_CONST_OBJ_ATTRIBUTES     LALLOC_ROM_ATTRIBUTES
#endif

#ifndef LALLOC_T
#define LALLOC_T LALLOC_CONST_OBJ_ATTRIBUTES lalloc_t
#endif

#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define ct_assert(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }

/**
   @brief declares a static object that can be declared in any scope of execution
 */
#define LALLOC_DECLARE(NAME,SIZE  )                                                                   \
lalloc_dyn_t         NAME##_Data;                                                                     \
LALLOC_ALIGN_TYPE    NAME##_pool[LALLOC_SIZE_ROUND_UP(LALLOC_ALIGN_TYPE,SIZE) / LALLOC_ALIGNMENT ];   \
lalloc_t             LALLOC_ROM_ATTRIBUTES NAME =                                                     \
{                                                                                                     \
    .pool     = (uint8_t*) NAME##_pool,                                                               \
    .size     = sizeof(NAME##_pool),                                                                  \
    .dyn      = &(NAME##_Data),                                                                       \
};

/** methods  ---------------------------------------------------------------------------  **/

/* User interfaces */
void lalloc_init( LALLOC_T * obj );
void lalloc_alloc( LALLOC_T * obj, void **addr, LALLOC_IDX_TYPE *size );
void lalloc_alloc_revert( LALLOC_T * obj );
bool lalloc_commit( LALLOC_T * obj, LALLOC_IDX_TYPE size );
bool lalloc_free_first( LALLOC_T * obj ) ;
bool lalloc_free( LALLOC_T * obj, void *addr );
bool lalloc_free_last( LALLOC_T * obj );
void lalloc_get_first( LALLOC_T * obj, void **addr, LALLOC_IDX_TYPE *size );
void lalloc_get_n( LALLOC_T * obj, void **addr, LALLOC_IDX_TYPE *size, LALLOC_IDX_TYPE n );
void lalloc_get_last( LALLOC_T * obj, void **addr, LALLOC_IDX_TYPE *size );
bool lalloc_is_full( LALLOC_T * obj );
bool lalloc_is_empty( LALLOC_T * obj );
LALLOC_IDX_TYPE lalloc_get_free_space ( LALLOC_T * obj );
char lalloc_dest_belongs( LALLOC_T * obj, void *addr );
LALLOC_IDX_TYPE lalloc_get_alloc_count ( LALLOC_T * obj );

void* lalloc_ctor( LALLOC_IDX_TYPE size );
void lalloc_dtor( void* this_ );

#ifdef __cplusplus
}
#endif



/* v0.20 */


