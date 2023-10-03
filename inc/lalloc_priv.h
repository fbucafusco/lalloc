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

/**
   @brief This file defines private defiitions that are use by the main module of the library and also by the unit tests.
 */

#pragma once

#include "lalloc.h"

#define DEPRECATED

/* ==PRIVATE MACROS==CONFIGURATION===================================================================== */
 
/**
   @brief LALLOC_FREE_ANY
          Defines the behavior of lalloc_free.
          1: addr field could be ANY address of the user granded memory area with a range from  the start or the block to start + committed size.
          0: addr MUST be the starting area of the memory area given to the user with lalloc_alloc
 */
#ifndef LALLOC_FREE_ANY
#define LALLOC_FREE_ANY 1
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
 
/* FOR TESTING PURPOSES */
#ifndef LALLOC_VALIDATE_INDEX
#define LALLOC_VALIDATE_INDEX(IDX)
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
#endif

#define LALLOC_BLOCK_FIELD_OFFSET_NEXT                  offsetof(lalloc_block_t , next)
#define LALLOC_BLOCK_FIELD_OFFSET_SIZE                  offsetof(lalloc_block_t , blk_size)

#define LALLOC_NEXT_BLOCK_IDX(idx,size)                 (idx+size+lalloc_b_overhead_size)

#define LALLOC_BLOCK_DATA_FROMPOOL__(POOL, INDEX)       &(POOL)[(INDEX+lalloc_block_size)]
#define LALLOC_BLOCK_SIZE_FROMPOOL__(POOL, INDEX)       (( ( lalloc_block_t* ) &(POOL)[(INDEX)] )->blk_size)
#define LALLOC_BLOCK_NEXT_FROMPOOL__(POOL, INDEX)       (( ( lalloc_block_t* ) &(POOL)[(INDEX)] )->next)
#define LALLOC_BLOCK_PREV_FROMPOOL__(POOL, INDEX)       (( ( lalloc_block_t* ) &(POOL)[(INDEX)] )->prev)
#define LALLOC_BLOCK_NEXTPHYS_FROMPOOL__(POOL, INDEX)   (( ( lalloc_block_t* ) &(POOL)[(INDEX)] )->next_phys)
#define LALLOC_BLOCK_PREVPHYS_FROMPOOL__(POOL, INDEX)   (( ( lalloc_block_t* ) &(POOL)[(INDEX)] )->prev_phys)


#define LALLOC_NEXT_HEAD(CURRENT_BLOCK_SIZE)            ( obj->dyn->Head + lalloc_block_size + (CURRENT_BLOCK_SIZE) )
#define LALLOC_NEXT_HEAD_BK                             LALLOC_NEXT_HEAD( ( (lalloc_block_t*) &obj->pool[obj->dyn->Head] )->blk_size )

#define LALLOC_BLOCK_PAYLOAD_REF(INDEX)                 ( &obj->pool[(INDEX) + lalloc_block_size] )
#define LALLOC_NEXT_BLOCK_INDEX_TEST(POOL,INDEX)        (( ( lalloc_block_t* ) &(POOL)[(INDEX)] )->next)

/* OPERACIONES */
#define LALLOC_GET_BLOCK_DATA_FROMPOOL_(POOL,INDEX, DATAPTR)        (DATAPTR) = LALLOC_BLOCK_DATA_FROMPOOL__( (POOL), (INDEX) )
#define LALLOC_GET_BLOCK_SIZE_FROMPOOL_(POOL,INDEX, SIZE)           (SIZE) = LALLOC_BLOCK_SIZE_FROMPOOL__( (POOL), (INDEX) )
#define LALLOC_GET_BLOCK_NEXT_FROMPOOL_(POOL,INDEX, NEXT)           (NEXT) = LALLOC_BLOCK_NEXT_FROMPOOL__( (POOL), (INDEX) )
#define LALLOC_GET_BLOCK_PREV_FROMPOOL_(POOL,INDEX, PREV)           (PREV) = LALLOC_BLOCK_PREV_FROMPOOL__(POOL,INDEX);
#define LALLOC_GET_BLOCK_NEXTPHYS_FROMPOOL_(POOL, INDEX, NEXT)      (NEXT) = LALLOC_BLOCK_NEXTPHYS_FROMPOOL__(POOL,INDEX);
#define LALLOC_GET_BLOCK_PREVPHYS_FROMPOOL_(POOL, INDEX, PREV)      (PREV) = LALLOC_BLOCK_PREVPHYS_FROMPOOL__(POOL,INDEX);
#define LALLOC_SET_BLOCK_SIZE_FROMPOOL_(POOL,INDEX, SIZE)           LALLOC_BLOCK_SIZE_FROMPOOL__( (POOL), (INDEX) ) = (SIZE)
#define LALLOC_SET_BLOCK_NEXT_FROMPOOL_(POOL,INDEX, NEXT)           LALLOC_BLOCK_NEXT_FROMPOOL__( (POOL), (INDEX) ) = (NEXT)
#define LALLOC_SET_BLOCK_PREV_FROMPOOL_(POOL,INDEX, PREV)           LALLOC_BLOCK_PREV_FROMPOOL__( (POOL), (INDEX) ) = (PREV)
#define LALLOC_SET_BLOCK_NEXTPHYS_FROMPOOL_(POOL, INDEX, NEXT)      LALLOC_BLOCK_NEXTPHYS_FROMPOOL__(POOL,INDEX) = (NEXT)
#define LALLOC_SET_BLOCK_PREVPHYS_FROMPOOL_(POOL, INDEX, PREV)      LALLOC_BLOCK_PREVPHYS_FROMPOOL__(POOL,INDEX) = (PREV)

/* OPERACIONES + VALIDACION */
#define LALLOC_GET_BLOCK_DATA_FROMPOOL(POOL,INDEX, DATAPTR)         LALLOC_VALIDATE_INDEX(INDEX); LALLOC_GET_BLOCK_DATA_FROMPOOL_(POOL,INDEX, DATAPTR)
#define LALLOC_GET_BLOCK_SIZE_FROMPOOL(POOL,INDEX, SIZE)            LALLOC_VALIDATE_INDEX(INDEX); LALLOC_GET_BLOCK_SIZE_FROMPOOL_(POOL,INDEX, SIZE)
#define LALLOC_GET_BLOCK_NEXT_FROMPOOL(POOL,INDEX, NEXT)            LALLOC_VALIDATE_INDEX(INDEX); LALLOC_GET_BLOCK_NEXT_FROMPOOL_(POOL,INDEX, NEXT)
#define LALLOC_GET_BLOCK_PREV_FROMPOOL(POOL,INDEX, PREV)            LALLOC_VALIDATE_INDEX(INDEX); LALLOC_GET_BLOCK_PREROMPOOL_(POOL,INDEX, DATAPTR)
#define LALLOC_GET_BLOCK_NEXTPHYS_FROMPOOL(POOL, INDEX, NEXT)       LALLOC_VALIDATE_INDEX(INDEX); LALLOC_GET_BLOCK_NEXTPHYS_FROMPOOL_(POOL, INDEX, NEXT)
#define LALLOC_GET_BLOCK_PREVPHYS_FROMPOOL(POOL, INDEX, PREV)       LALLOC_VALIDATE_INDEX(INDEX); LALLOC_GET_BLOCK_PREVPHYS_FROMPOOL_(POOL, INDEX, PREV)
#define LALLOC_SET_BLOCK_SIZE_FROMPOOL(POOL,INDEX, SIZE)            LALLOC_VALIDATE_INDEX(INDEX); LALLOC_SET_BLOCK_SIZE_FROMPOOL_(POOL,INDEX, SIZE)
#define LALLOC_SET_BLOCK_NEXT_FROMPOOL(POOL,INDEX, NEXT)            LALLOC_VALIDATE_INDEX(INDEX); LALLOC_SET_BLOCK_NEXT_FROMPOOL_(POOL,INDEX, NEXT)
#define LALLOC_SET_BLOCK_PREV_FROMPOOL(POOL,INDEX, PREV)            LALLOC_VALIDATE_INDEX(INDEX); LALLOC_SET_BLOCK_PREV_FROMPOOL_(POOL,INDEX, PREV)
#define LALLOC_SET_BLOCK_NEXTPHYS_FROMPOOL(POOL, INDEX, NEXT)       LALLOC_VALIDATE_INDEX(INDEX); LALLOC_SET_BLOCK_NEXTPHYS_FROMPOOL_(POOL, INDEX, NEXT)
#define LALLOC_SET_BLOCK_PREVPHYS_FROMPOOL(POOL, INDEX, PREV)       LALLOC_VALIDATE_INDEX(INDEX); LALLOC_SET_BLOCK_PREVPHYS_FROMPOOL_(POOL, INDEX, PREV)

#define LALLOC_GET_BLOCK_SIZE(INDEX, SIZE)                          LALLOC_GET_BLOCK_SIZE_FROMPOOL( obj->pool , (INDEX) , (SIZE) )
#define LALLOC_GET_BLOCK_NEXT(INDEX, NEXT)                          LALLOC_GET_BLOCK_NEXT_FROMPOOL( obj->pool , (INDEX) , (NEXT) )
#define LALLOC_GET_BLOCK_PREV(INDEX, PREV)                          LALLOC_GET_BLOCK_PREV_FROMPOOL( obj->pool , (INDEX) , (PREV) )

#define LALLOC_SET_BLOCK_SIZE(INDEX, SIZE)                          LALLOC_SET_BLOCK_SIZE_FROMPOOL( obj->pool , (INDEX) , (SIZE) )
#define LALLOC_SET_BLOCK_NEXT(INDEX, NEXT)                          LALLOC_SET_BLOCK_NEXT_FROMPOOL( obj->pool , (INDEX) , (NEXT) )
#define LALLOC_SET_BLOCK_PREV(INDEX, PREV)                          LALLOC_SET_BLOCK_PREV_FROMPOOL( obj->pool , (INDEX) , (PREV) )


#define LALLOC_NODE_HEAD_SIZE               LALLOC_ALIGN_ROUND_UP( sizeof(lalloc_block_t) )

/* private functions exposed to tests */
void _block_list_add_first ( uint8_t* pool, LALLOC_IDX_TYPE* list_idx, LALLOC_IDX_TYPE block_idx );
void _block_set_data ( uint8_t* pool, LALLOC_IDX_TYPE  block_idx, uint8_t* addr, LALLOC_IDX_TYPE size );
void _block_list_add_sorted ( uint8_t* pool, LALLOC_IDX_TYPE* list_idx, LALLOC_IDX_TYPE block_idx );
void _block_list_get_n ( uint8_t* pool, LALLOC_IDX_TYPE list_idx,  LALLOC_IDX_TYPE n, uint8_t** addr, LALLOC_IDX_TYPE* size );
LALLOC_IDX_TYPE _block_list_remove ( uint8_t* pool, LALLOC_IDX_TYPE* block_idx );
void _block_set( uint8_t* pool, LALLOC_IDX_TYPE idx, LALLOC_IDX_TYPE size, LALLOC_IDX_TYPE next, LALLOC_IDX_TYPE prev, LALLOC_IDX_TYPE flags );
LALLOC_IDX_TYPE _block_remove( uint8_t *pool, LALLOC_IDX_TYPE *idx );