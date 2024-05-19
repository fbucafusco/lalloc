/*
BSD 3-Clause License

Copyright (c) 2024, Franco Bucafusco
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

/**
   @brief This file defines private defiitions that are use by the main module of the library and also by the unit tests.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lalloc.h"

#define DEPRECATED

/* ==PRIVATE MACROS==CONFIGURATION===================================================================== */
/**
   @brief LALLOC_FREE_ANY
          The user can define it in lalloc_config.h
          Defines the behavior of lalloc_free.
          1:  addr field could be ANY address of memory granted to the user with a range from  the start or the block to start + committed size.
              Is handy when a pointer to an allocated space is passed to other layer of processing
              and then these layers need to deallocate the space, without having the start of the original
              allocated area.
              Slows down the lalloc_free function.
          0:  addr MUST be the starting area of the memory area given to the user with lalloc_alloc
 */
#ifndef LALLOC_FREE_ANY
#define LALLOC_FREE_ANY 0
#endif

/**
   @brief LALLOC_ALLOW_QUEUED_FREES
          1: lalloc_free_first and lalloc_free_last are implemeted to remove allocations in the order they were commited
          0: The only free function available is lalloc_free where the used must provide an address.
 */
#ifndef LALLOC_ALLOW_QUEUED_FREES
#define LALLOC_ALLOW_QUEUED_FREES 0
#endif

/**
   @brief   LALLOC_MIN_PAYLOAD_SIZE
            the user can define it in lalloc_config.h in order to avoid small allocations.
 */
#ifndef LALLOC_MIN_PAYLOAD_SIZE
#define LALLOC_MIN_PAYLOAD_SIZE             0
#endif


#ifndef LALLOC_INLINE
#define LALLOC_INLINE inline
#endif

/* ==PRIVATE MACROS==CONDITIONAL===================================================================== */
#ifndef LALLOC_CRITICAL_START
#define LALLOC_CRITICAL_START
#endif

#ifndef LALLOC_CRITICAL_END
#define LALLOC_CRITICAL_END
#endif

/* ==PRIVATE MACROS==FUNCTIONAL====================================================================== */
#ifdef LALLOC_TEST
#define LALLOC_STATIC
#else
#define LALLOC_STATIC   static
#endif //LALLOC_TEST

#define LALLOC_NEXT_BLOCK_IDX(idx,size)               ((idx)+(size)+lalloc_b_overhead_size)

#define LALLOC_BLOCK_HEADER_SIZE                      LALLOC_ALIGN_ROUND_UP( sizeof(lalloc_block_t) )

#define LALLOC_BLOCK(POOL, INDEX)                     ( ( lalloc_block_t* ) &(POOL)[(INDEX)] )
#define LALLOC_BLOCK_DATA(POOL, INDEX)                &(POOL)[((INDEX)+lalloc_b_overhead_size)]
#define LALLOC_BLOCK_FLAGS(POOL, INDEX)               ( LALLOC_BLOCK(POOL, INDEX)->flags )
#define LALLOC_BLOCK_SIZE(POOL, INDEX)                ( LALLOC_BLOCK(POOL, INDEX)->blk_size )
#define LALLOC_BLOCK_NEXT(POOL, INDEX)                ( LALLOC_BLOCK(POOL, INDEX)->next )
#define LALLOC_BLOCK_PREV(POOL, INDEX)                ( LALLOC_BLOCK(POOL, INDEX)->prev )
#define LALLOC_BLOCK_PREVPHYS(POOL, INDEX)            ( LALLOC_BLOCK(POOL, INDEX)->prev_phys )

/* operations */
#define LALLOC_GET_BLOCK_DATA(POOL,INDEX, DATAPTR)    (DATAPTR) = LALLOC_BLOCK_DATA( (POOL), (INDEX) )
#define LALLOC_GET_BLOCK_SIZE(POOL,INDEX, SIZE)       (SIZE) = LALLOC_BLOCK_SIZE( (POOL), (INDEX) )
#define LALLOC_GET_BLOCK_FLAGS(POOL,INDEX, SIZE)      (SIZE) = LALLOC_BLOCK_FLAGS( (POOL), (INDEX) )
#define LALLOC_GET_BLOCK_NEXT(POOL,INDEX, NEXT)       (NEXT) = LALLOC_BLOCK_NEXT( (POOL), (INDEX) )
#define LALLOC_GET_BLOCK_PREV(POOL,INDEX, PREV)       (PREV) = LALLOC_BLOCK_PREV(POOL,INDEX);
#define LALLOC_GET_BLOCK_PREVPHYS(POOL, INDEX, PREV)  (PREV) = LALLOC_BLOCK_PREVPHYS(POOL,INDEX);
#define LALLOC_SET_BLOCK_SIZE(POOL,INDEX, SIZE)       LALLOC_BLOCK_SIZE( (POOL), (INDEX) ) = (SIZE)
#define LALLOC_SET_BLOCK_FLAGS(POOL,INDEX, SIZE)      LALLOC_BLOCK_FLAGS( (POOL), (INDEX) ) = (SIZE)
#define LALLOC_SET_BLOCK_NEXT(POOL,INDEX, NEXT)       LALLOC_BLOCK_NEXT( (POOL), (INDEX) ) = (NEXT)
#define LALLOC_SET_BLOCK_PREV(POOL,INDEX, PREV)       LALLOC_BLOCK_PREV( (POOL), (INDEX) ) = (PREV)
#define LALLOC_SET_BLOCK_PREVPHYS(POOL, INDEX, PREV)  LALLOC_BLOCK_PREVPHYS(POOL,INDEX) = (PREV)

/**
   @brief structure for each block's block
 */
#pragma pack(1)
typedef struct
{
    LALLOC_IDX_TYPE prev;       /* Logical index to the previous block in the list     */
    LALLOC_IDX_TYPE next;       /* Logical index to the next block in the list         */
    LALLOC_IDX_TYPE prev_phys;  /* Physical index to the previous block in the pool    */
    LALLOC_IDX_TYPE blk_size;   /* playload block's size                               */
#if LALLOC_ALIGNMENT==1
    LALLOC_IDX_TYPE flags;
#endif
} lalloc_block_t;
#pragma pack()

/**
   @brief   LALLOC_FREE_BLOCK_MASK
            defines the bit within the blk_size field of lalloc_block_t that will mark the block as free
            NOTE: this is done for avoiding move through the free list when joining free blocks.
 */
#define LALLOC_FREE_BLOCK_MASK       1
#define LALLOC_USED_BLOCK_MASK       0

/* private functions exposed to tests */
void _block_list_add_before ( uint8_t* pool, LALLOC_IDX_TYPE* list_idx, LALLOC_IDX_TYPE block_idx );
void _block_set_data ( uint8_t* pool, LALLOC_IDX_TYPE  block_idx, uint8_t* addr, LALLOC_IDX_TYPE size );
void _block_list_add_sorted ( uint8_t* pool, LALLOC_IDX_TYPE* list_idx, LALLOC_IDX_TYPE block_idx );
void _block_list_get_n ( uint8_t* pool, LALLOC_IDX_TYPE list_idx,  LALLOC_IDX_TYPE n, uint8_t** addr, LALLOC_IDX_TYPE* size );
LALLOC_IDX_TYPE _block_list_remove ( uint8_t* pool, LALLOC_IDX_TYPE* block_idx );
void _block_set( uint8_t* pool, LALLOC_IDX_TYPE idx, LALLOC_IDX_TYPE size, LALLOC_IDX_TYPE next, LALLOC_IDX_TYPE prev, LALLOC_IDX_TYPE flags );
LALLOC_IDX_TYPE _block_remove( uint8_t *pool, LALLOC_IDX_TYPE *idx );
LALLOC_INLINE LALLOC_IDX_TYPE _block_get_next_phy( uint8_t *pool, LALLOC_IDX_TYPE block_idx );

#ifdef __cplusplus
}
#endif

/* v1.00 */