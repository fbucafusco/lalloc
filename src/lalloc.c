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

#include <stdlib.h>
#include "lalloc.h"
#include "lalloc_priv.h"

/* CONSTANTS ============================================================================================================ */
LALLOC_STATIC const LALLOC_IDX_TYPE lalloc_alignment = LALLOC_ALIGNMENT;
LALLOC_STATIC const LALLOC_IDX_TYPE lalloc_invalid_index = LALLOC_IDX_INVALID;
LALLOC_STATIC const LALLOC_IDX_TYPE lalloc_b_overhead_size = LALLOC_BLOCK_HEADER_SIZE;



/* ==PRIVATE METHODS================================================================================= */

/**
    @brief sets the block information at the selected pool and block index

    @param pool
    @param idx
    @param size
    @param next
    @param prev
    @param flags
 */
LALLOC_INLINE void _block_set( uint8_t *pool, LALLOC_IDX_TYPE idx, LALLOC_IDX_TYPE size, LALLOC_IDX_TYPE next, LALLOC_IDX_TYPE prev, LALLOC_IDX_TYPE flags )
{
    LALLOC_SET_BLOCK_NEXT( pool, idx, next );
    LALLOC_SET_BLOCK_PREV( pool, idx, prev );
    LALLOC_SET_BLOCK_PREVPHYS( pool, idx, LALLOC_IDX_INVALID );
    LALLOC_SET_BLOCK_SIZE( pool, idx, size | flags );
}

/**
    @brief Gets the size of a block based on its index in the pool.
    @param pool
    @param block_idx
    @return LALLOC_IDX_TYPE size of the block
 */
LALLOC_INLINE LALLOC_IDX_TYPE _block_get_size( uint8_t *pool, LALLOC_IDX_TYPE block_idx )
{
    LALLOC_IDX_TYPE size;
    LALLOC_GET_BLOCK_SIZE( pool, block_idx, size );

#if LALLOC_ALIGNMENT == 1
#else
    /* clear the control flags */
    size &= ~LALLOC_FREE_BLOCK_MASK;
#endif

    return size;
}

/**
    @brief Tells if a block is free or not based on its index in the pool.

    @param pool
    @param block_idx
    @return bool
 */
LALLOC_INLINE bool _block_is_free( uint8_t *pool, LALLOC_IDX_TYPE block_idx )
{
#if LALLOC_ALIGNMENT == 1
    LALLOC_IDX_TYPE flags;
    LALLOC_GET_BLOCK_FLAGS( pool, block_idx, flags );
    return flags & LALLOC_FREE_BLOCK_MASK;
#else
    LALLOC_IDX_TYPE size;
    LALLOC_GET_BLOCK_SIZE( pool, block_idx, size );
    return size & LALLOC_FREE_BLOCK_MASK;
#endif
}

/**
    @brief  overwrites the size of a block.
            If the built version has the FREE_BLOCK_MASK flag, it will overwriten.
            CALL _block_set_flags after

    @param pool
    @param block_idx
    @param size
 */
LALLOC_INLINE void _block_set_size( uint8_t *pool, LALLOC_IDX_TYPE block_idx, LALLOC_IDX_TYPE size )
{
    LALLOC_SET_BLOCK_SIZE( pool, block_idx, size );
}

LALLOC_INLINE LALLOC_IDX_TYPE _block_get_prev_phy( uint8_t *pool, LALLOC_IDX_TYPE block_idx )
{
    LALLOC_IDX_TYPE prev_phy;
    LALLOC_GET_BLOCK_PREVPHYS( pool, block_idx, prev_phy );
    return prev_phy;
}

LALLOC_INLINE LALLOC_IDX_TYPE _block_get_next_phy( uint8_t *pool, LALLOC_IDX_TYPE block_idx )
{
    return LALLOC_NEXT_BLOCK_IDX( block_idx, _block_get_size( pool, block_idx ) );
}

/**
    @brief  Overwrites the flags of a block.
            This is a private lalloc operation.
            NOT THREAD SAFE

    @param pool
    @param block_idx
    @param newflags
    @return LALLOC_INLINE
 */
LALLOC_INLINE void _block_set_flags( uint8_t *pool, LALLOC_IDX_TYPE block_idx, LALLOC_IDX_TYPE newflags )
{
#if LALLOC_ALIGNMENT == 1
    LALLOC_SET_BLOCK_FLAGS( pool, block_idx, newflags );
#else
    LALLOC_IDX_TYPE size;
    LALLOC_GET_BLOCK_SIZE( pool, block_idx, size );
    size &= ~LALLOC_FREE_BLOCK_MASK;
    LALLOC_SET_BLOCK_SIZE( pool, block_idx, size | newflags );
#endif
}

/**
   @brief  gets the block data

   @param pool          memory pool
   @param block_idx     block index of the block
   @param addr          address of the block
   @param size          size of the block
 */
void _block_get_data( uint8_t *pool, LALLOC_IDX_TYPE block_idx, uint8_t **addr, LALLOC_IDX_TYPE *size )
{
    LALLOC_GET_BLOCK_DATA( pool, block_idx, *addr );
    *size = _block_get_size( pool, block_idx );
}

/**
   @brief Removes the block from one double linked list and updates the original block pointer to the next.
          e.g
          *idx points to a block of a list with only one element. *ids becomes LALLOC_IDX_INVALID
          *idx points to a block of a list with more than one element. *ids becomes next block of the removed block

          Externally the user must ensure that the block belongs to a well configured list.

   @param pool
   @param idx               reference to the index of the block to be removed
                            will be updated with the next block's index or to invalid if the list is empty after the removal
   @return LALLOC_IDX_TYPE  the original value of *idx, which now is orphan
 */
LALLOC_IDX_TYPE _block_remove( uint8_t *pool, LALLOC_IDX_TYPE *idx )
{
    LALLOC_IDX_TYPE next;
    LALLOC_IDX_TYPE prev;

    LALLOC_ASSERT( ( *idx ) != LALLOC_IDX_INVALID );

    LALLOC_GET_BLOCK_NEXT( pool, ( *idx ), next );
    LALLOC_GET_BLOCK_PREV( pool, ( *idx ), prev );

    /* backup */
    LALLOC_IDX_TYPE orphan_idx = ( *idx );

    if ( ( *idx ) == next ) // idx is an index for a block beloning to a list of one element
    {
        /* there is only one element in the list. */
        LALLOC_ASSERT( ( *idx ) == prev ); // If there is only one, this must be true

        /* mark the list as empty */
        ( *idx ) = LALLOC_IDX_INVALID;
    }
    else
    {
        /* the list has more than one element */
        LALLOC_SET_BLOCK_NEXT( pool, prev, next );
        LALLOC_SET_BLOCK_PREV( pool, next, prev );

        /* mark the list as the next block  */
        ( *idx ) = next;
    }

    return orphan_idx;
}

/**
     @brief find a block in a list by pool index.
            idx must target ANY byte belonging to tha payload of a list's block.

            If found, it returns the index block that contains the index provided (should it be the same? NO)
            If not found, it return LALLOC_IDX_INVALID

            NOT THREAD SAFE

    @param pool
    @param list
    @param idx
    @return LALLOC_IDX_TYPE
*/
LALLOC_IDX_TYPE _block_list_find_by_idx( uint8_t *pool, LALLOC_IDX_TYPE list, LALLOC_IDX_TYPE idx )
{
    // LALLOC_IDX_TYPE size;
    LALLOC_IDX_TYPE next = list;

    while ( 1 )
    {
        /* Calculates the condition for ending the loop:
           The condition is "idx is bigger that the begining of the block && idx is less that its top boundry") */
        LALLOC_IDX_TYPE condition = ( idx >= next );

        if ( condition )
        {
            /* get the block size in order to compute the top boundry */
            // size = _block_get_size( pool, next );

            condition = ( idx < _block_get_next_phy( pool, next ) );
        }

        if ( condition )
        {
            /* found */
            return next;
        }

        LALLOC_GET_BLOCK_NEXT( pool, next, next );

        if ( next == list )
        {
            /* start again not found */
            return LALLOC_IDX_INVALID;
        }
    }
}

/**
   @brief   finds a block in a NON EMPTY list by payload reference.
            returns the found relative block index or LALLOC_IDX_INVALID if not found
            If found, it returns the index block whos payload contains the provided reference

            if LALLOC_FREE_ANY==1 will finds a block within the list whose address is wihin the user area of the block.
            if LALLOC_FREE_ANY==0 will return the block from the list, whose address matches the first byte of the user area of the block.
            If not found, it returns LALLOC_IDX_INVALID
            NOT THREAD SAFE

   @param pool
   @param list_idx
   @param addr          ANY address that must point somewhere in the pool
   @return LALLOC_IDX_TYPE
 */
LALLOC_IDX_TYPE _block_list_find_by_ref( uint8_t *pool, LALLOC_IDX_TYPE list, uint8_t *addr )
{
    LALLOC_IDX_TYPE rv;

    /* relativize addr to the pool.  */
    LALLOC_IDX_TYPE idx = addr - pool;

#if LALLOC_FREE_ANY == 1
    rv = _block_list_find_by_idx( pool, list, idx );
#else

    /* addr is the user addres, that is shifted from the block address in lalloc_b_overhead_size bytes */
    idx -= lalloc_b_overhead_size;

    /* is the list the allocated list or the free list ? */
    bool is_list_free = _block_is_free( pool, list );
    bool is_block_free = _block_is_free( pool, idx );

    if ( is_list_free == is_block_free )
    {
        // belongs to the same list, found
        rv = idx;
    }
    else
    {
        // belongs to different list, not found
        rv = LALLOC_IDX_INVALID;
    }
#endif

    return rv;
}

/**
   @brief   Obtains the nth element in the list

   @param pool
   @param list
   @param n
   @param addr
   @param size
 */
void _block_list_get_n( uint8_t *pool, LALLOC_IDX_TYPE list, LALLOC_IDX_TYPE n, uint8_t **addr, LALLOC_IDX_TYPE *size )
{
    /* TODO, very similtar to _block_list_find_by_idx but with a different search criteria.
             Could be improved with lambdas or callbacks  */

    LALLOC_IDX_TYPE i = 0;

    LALLOC_IDX_TYPE next = list;

    if ( LALLOC_IDX_INVALID != list )
    {
        /* it only searches when the list is valid */
        while ( 1 )
        {
            if ( i == n )
            {
                /* found */
                _block_get_data( pool, next, addr, size );
                return;
            }

            LALLOC_GET_BLOCK_NEXT( pool, next, next );

            if ( next == list )
            {
                /* list wrap around: not found */
                break;
            }

            i++;
        }
    }

    *addr = NULL;
    *size = 0;

    return;
}

/**
   @brief   adds a block at the begining of the list.
            block_idx: index of the block to add
            list_idx:  reference to a list
            note: the block must be preconfigured with the correct size and flags

   @param pool
   @param list_idx
   @param block_idx
*/
void _block_list_add_before( uint8_t *pool, LALLOC_IDX_TYPE *list_idx, LALLOC_IDX_TYPE block_idx )
{
    LALLOC_IDX_TYPE prev;
    LALLOC_ASSERT( block_idx != LALLOC_IDX_INVALID );

    if ( LALLOC_IDX_INVALID == ( *list_idx ) )
    {
        /* if the list is empty, then the block is the first and must be added */
        LALLOC_SET_BLOCK_NEXT( pool, block_idx, block_idx );
        LALLOC_SET_BLOCK_PREV( pool, block_idx, block_idx );
    }
    else
    {
        /* the list has, at least, one block. */
        LALLOC_GET_BLOCK_PREV( pool, ( *list_idx ), prev );

        LALLOC_SET_BLOCK_NEXT( pool, block_idx, ( *list_idx ) );
        LALLOC_SET_BLOCK_PREV( pool, block_idx, prev );
        LALLOC_SET_BLOCK_NEXT( pool, prev, block_idx );
        LALLOC_SET_BLOCK_PREV( pool, ( *list_idx ), block_idx );
    }

    /* the list is updated to the added block */
    *list_idx = block_idx;
}

/**
   @brief   adds a block into de list ordered by block size (biggest to smallest)
            block_idx: index from the block to add
            list_idx:  reference to a list
            note: the block mus be preconfigured with the correct size and flags

   @param pool
   @param list_idx
   @param block_idx
*/
void _block_list_add_sorted( uint8_t *pool, LALLOC_IDX_TYPE *list_idx, LALLOC_IDX_TYPE block_idx )
{
    LALLOC_IDX_TYPE current = *list_idx;

    if ( LALLOC_IDX_INVALID == current )
    {
        /* if the list is empty, the block is the first */
        _block_list_add_before( pool, &current, block_idx );
        *list_idx = current;
    }
    else
    {
        LALLOC_IDX_TYPE size_curr;
        LALLOC_IDX_TYPE size_new;
        LALLOC_IDX_TYPE prev = LALLOC_IDX_INVALID;

        size_new = _block_get_size( pool, block_idx );

        while ( 1 )
        {
            size_curr = _block_get_size( pool, current );

            if ( size_new > size_curr )
            {
                break;
            }

            prev = current;
            LALLOC_GET_BLOCK_NEXT( pool, current, current );

            if ( current == *list_idx )
            {
                break;
            }
        }

        _block_list_add_before( pool, &current, block_idx );

        if ( current == *list_idx )
        {
            *list_idx = current;
        }
        else
        {
            if ( prev != LALLOC_IDX_INVALID )
            {
            }
            else
            {
                *list_idx = block_idx; // Update the list's start if it was the first element
            }
        }
    }
}

/**
   @brief   Removes a block from a list. This is a private lalloc operation.
            The list is not updated unless the block to be removed is the first one.
            WARNING: There is no validation at all that the idx belongs to a block of the given list.

   @param obj
   @param list
   @param block_idx
   @return LALLOC_IDX_TYPE  An orphaned block
 */
LALLOC_IDX_TYPE _block_list_remove_block( uint8_t *pool, LALLOC_IDX_TYPE *list, LALLOC_IDX_TYPE idx )
{
    LALLOC_IDX_TYPE orphan_idx;

    if ( *list == idx )
    {
        /* The source list starts with the block that is being removed, so the content of *list will be upated. */
        orphan_idx = _block_remove( pool, list );

        /* If the block is the only one in the list, list will point to LALLOC_IDX_INVALID */
    }
    else
    {
        /* this operation can fail if block_idx does not belong to *list */
        orphan_idx = _block_remove( pool, &( idx ) );
    }
    return orphan_idx;
}

/**
    @brief  Given a orphan node (a block that is not in any list)
            this function joins it with its physical and previous physical adjacent blocks if they are free.
            Since this block will end up being in a free list, and the adjacent blocks are free,
            the blocks are trated as part of the list.

        it returns the resultant block, with all the header updated.
        this is an internal method for lalloc operation
        NOT THREAD SAFE

    @param obj
    @param orphan_block         Orphan block to be joined with adjacent blocks
    @return LALLOC_IDX_TYPE     the resultant block, also orphan
 */
LALLOC_IDX_TYPE _block_join_adjacent( LALLOC_T *obj, LALLOC_IDX_TYPE orphan_block )
{
    LALLOC_IDX_TYPE prev_phy;
    LALLOC_IDX_TYPE next_phy;

    prev_phy = _block_get_prev_phy( obj->pool, orphan_block );
    next_phy = _block_get_next_phy( obj->pool, orphan_block );

    /*
        |           |DDDDDDDDTTTTT|DDDDDDDDDDDD|
        |  prev phy |             |  next phy  |
        ^prev_phy   ^removed  ^   ^next_phy
    */

    if ( prev_phy != LALLOC_IDX_INVALID )
    {
        /* obtains the size of the previous physical block  */
        if ( _block_is_free( obj->pool, prev_phy ) )
        {
            /* the previous physcal block is free. */
            orphan_block = _block_list_remove_block( obj->pool, &( obj->dyn->flist ), prev_phy );
        }
        else
        {
            /* the block is not free, cant be merged */
        }
    }
    else
    {
        /* the previous phy block is invalid, it means that is the first block */
        LALLOC_ASSERT( orphan_block == 0 );
    }

    /* check right */
    if ( next_phy != obj->size )
    {
        if ( _block_is_free( obj->pool, next_phy ) )
        {
            /* the next physcal block is free. */
            LALLOC_IDX_TYPE temp = _block_list_remove_block( obj->pool, &( obj->dyn->flist ), next_phy );

            /* ovewrite next physical */
            next_phy = _block_get_next_phy( obj->pool, temp );
        }
        else
        {
            /* the block is not free, cant be merged */
        }
    }
    else
    {
        /* the next block is invalid. Means that it is the last block of the cointainer. So the block is configured up to the pool's boundry */
    }

    /* repair the previous physical of the next. */
    if ( next_phy != obj->size )
    {
        LALLOC_SET_BLOCK_PREVPHYS( obj->pool, next_phy, orphan_block );
    }

    _block_set_size( obj->pool, orphan_block, next_phy - orphan_block - lalloc_b_overhead_size );
    _block_set_flags( obj->pool, orphan_block, LALLOC_FREE_BLOCK_MASK );

    return orphan_block;
}

/**
   @brief   moves a block from the allocated list to the free list.
            This is an internal method for lalloc operation
            NOT THREAD SAFE
   @param obj
   @param addr
   @return int
 */
bool _block_move_from_alloc_to_free( LALLOC_T *obj, void *addr )
{
    bool rv;

    if ( obj->dyn->alist != LALLOC_IDX_INVALID )
    {
        /* Remove the addr from the alocated list. */
        LALLOC_IDX_TYPE idx = _block_list_find_by_ref( obj->pool, obj->dyn->alist, ( uint8_t * )addr );

        if ( LALLOC_IDX_INVALID != idx )
        {
            LALLOC_IDX_TYPE orphan_idx = _block_list_remove_block( obj->pool, &( obj->dyn->alist ), idx );

            orphan_idx = _block_join_adjacent( obj, orphan_idx );

            _block_list_add_sorted( obj->pool, &( obj->dyn->flist ), orphan_idx );

            obj->dyn->allocated_blocks--;

            rv = true;
        }
        else
        {
            rv = false;
        }
    }
    else
    {
        rv = false;
    }

    return rv;
}

/**
   @brief Constructs in runtime a new lalloc_t object.

   @param size      pool size
   @return void*    handler to the new lalloc_t object
 */
void *lalloc_ctor( LALLOC_IDX_TYPE size )
{
    lalloc_t *rv = ( lalloc_t * )malloc( sizeof( lalloc_t ) );

    if ( rv != NULL )
    {
        rv->size = size;

        rv->pool = ( uint8_t * )malloc( rv->size );
        // rv->pool = ( uint8_t * ) aligned_alloc( LALLOC_ALIGNMENT , rv->size );

        if ( rv->pool != NULL )
        {
            rv->dyn = ( lalloc_dyn_t * )malloc( sizeof( lalloc_dyn_t ) );

            if ( rv->dyn != NULL )
            {
                lalloc_init( rv );
            }
            else
            {
                free( rv->pool );
                free( rv );
                rv = NULL;
            }
        }
        else
        {
            free( rv );
            rv = NULL;
        }
    }

    return rv;
}

void lalloc_dtor( void *me )
{
    free( ( ( lalloc_t * )me )->dyn );
    free( ( ( lalloc_t * )me )->pool );
    free( ( ( lalloc_t * )me ) );
}

/* ==PUBLIC METHODS================================================================================== */

/**
   @brief it clears the object

   @param obj
 */
void lalloc_clear( LALLOC_T *obj )
{
    LALLOC_CRITICAL_START;

    /* initialize the object data */
    obj->dyn->flist = 0;
    obj->dyn->alist = LALLOC_IDX_INVALID;
    obj->dyn->alloc_block = LALLOC_IDX_INVALID;
    obj->dyn->allocated_blocks = 0;

    /* initialice the only free block available (flist) */
    LALLOC_IDX_TYPE block_size = obj->size - lalloc_b_overhead_size;
    _block_set( obj->pool, obj->dyn->flist, block_size, obj->dyn->flist, obj->dyn->flist, 0 );
    _block_set_flags( obj->pool, obj->dyn->flist, LALLOC_FREE_BLOCK_MASK );

    LALLOC_CRITICAL_END;
}

/**
   @brief initializes the object

   @param obj
 */
void lalloc_init( LALLOC_T *obj )
{
    lalloc_clear( obj );
}

/**
   @brief it request a memory space to the object

   @param obj       reference to obj to work with
   @param addr      return of the address
   @param size      return of the size of the block
 */
void lalloc_alloc( LALLOC_T *obj, void **addr, LALLOC_IDX_TYPE *size )
{
    LALLOC_CRITICAL_START;

    /* Take the flist element (the first) and return your information, and remove the flist block. */
    if ( LALLOC_IDX_INVALID != obj->dyn->flist )
    {
        /* If the free list has some block, the first block will be the highest size one.
           The alloc function returns the first block in the free list */
        _block_get_data( obj->pool, obj->dyn->flist, ( uint8_t ** )addr, size );

        /* when an allocation takes place, the block is mark as not free (without the bit set) */
        _block_set_size( obj->pool, obj->dyn->flist, *size );
        _block_set_flags( obj->pool, obj->dyn->flist, LALLOC_USED_BLOCK_MASK );

        obj->dyn->alloc_block = obj->dyn->flist;
    }
    else
    {
        /* there isn't any block in the list  */
        *addr = NULL;
        *size = 0;
    }

    LALLOC_CRITICAL_END;
}

/**
   @brief reverts the alloc operation

   @param obj
 */
void lalloc_alloc_revert( LALLOC_T *obj )
{
    LALLOC_CRITICAL_START;

    if ( obj->dyn->flist != LALLOC_IDX_INVALID )
    {
        _block_set_flags( obj->pool, obj->dyn->flist, LALLOC_FREE_BLOCK_MASK );

        obj->dyn->alloc_block = LALLOC_IDX_INVALID;
    }
    LALLOC_CRITICAL_END;
}

/**
   @brief commits the previous allocated memory block

   @param obj
   @param size
   @return int 1 if the operation success
               0 otherwise
 */
bool lalloc_commit( LALLOC_T *obj, LALLOC_IDX_TYPE size )
{
    int rv;

    /* all the commited user memory areas are aligned as well */
    size = LALLOC_ALIGN_ROUND_UP( size );

#if LALLOC_MIN_PAYLOAD_SIZE > 0
    if ( size >= LALLOC_MIN_PAYLOAD_SIZE )
#endif
    {
        LALLOC_CRITICAL_START;

        if ( obj->dyn->alloc_block != LALLOC_IDX_INVALID )
        {
            LALLOC_IDX_TYPE block_size;

            /* SIZE VALIDATION */
            block_size = _block_get_size( obj->pool, obj->dyn->alloc_block );
            LALLOC_ASSERT( _block_is_free( obj->pool, obj->dyn->alloc_block ) == false );

            if ( size <= block_size )
            {
                /* calculation of the new block position
                |<-- size ------>|              |
                |<---- block_size -------------->|
                                    |
                                    \new_block_idx
                */

                bool split_block = false;
                LALLOC_IDX_TYPE new_block_size = block_size - size;

                /* removes the first block in the free list and modify flist to point to the next larger block */
                LALLOC_IDX_TYPE orphan_idx = _block_list_remove_block( obj->pool, &( obj->dyn->flist ), obj->dyn->alloc_block );

                if ( new_block_size < lalloc_b_overhead_size + LALLOC_MIN_PAYLOAD_SIZE )
                {
                    /* the block wont be splitted */
                    size = block_size;
                }
                else
                {
                    split_block = true;
                }

                /* set the new size of the block with the provided size*/
                _block_set_size( obj->pool, orphan_idx, size );
                _block_set_flags( obj->pool, orphan_idx, LALLOC_USED_BLOCK_MASK );

                /* add the block to allocated list */
                _block_list_add_before( obj->pool, &( obj->dyn->alist ), orphan_idx );

                if ( split_block )
                {
                    /* the bigblock must be splitted */
                    LALLOC_IDX_TYPE new_block_idx = LALLOC_NEXT_BLOCK_IDX( orphan_idx , size );

                    /* You have to separate the block, and insert the new */
                    new_block_size = new_block_size - lalloc_b_overhead_size;

                    /* sets the size of the commited block */
                    _block_set_size( obj->pool, new_block_idx, new_block_size );
                    _block_set_flags( obj->pool, new_block_idx, LALLOC_FREE_BLOCK_MASK );

                    /* next physical to the new block */
                    LALLOC_IDX_TYPE next_physical = _block_get_next_phy( obj->pool, new_block_idx );

                    /* set the previous phy to the new block */
                    LALLOC_SET_BLOCK_PREVPHYS( obj->pool, new_block_idx, orphan_idx );

                    if ( next_physical != obj->size )
                    {
                        /* if the next phy of the removed block is not the boundry, set the previous phy of it */
                        LALLOC_SET_BLOCK_PREVPHYS( obj->pool, next_physical, new_block_idx );
                    }

                    /* This operation only make sense if the user must to alloc then free blocks before commiting the allocated space. */
                    new_block_idx = _block_join_adjacent( obj, new_block_idx );

                    /* add the block to free list */
                    _block_list_add_sorted( obj->pool, &( obj->dyn->flist ), new_block_idx );
                }

                obj->dyn->alloc_block = LALLOC_IDX_INVALID;

                obj->dyn->allocated_blocks++;

                rv = true;
            }
            else
            {
                /* the user wants to allocate a buffer bigger than the max.
                   WARNING: did the user fill the buffer beyond block_size? if yes, KATAPUM */
                rv = false;
            }
        }
        else
        {
            /* there is no previous allocation */
            rv = false;
        }
        LALLOC_CRITICAL_END;
    }
#if LALLOC_MIN_PAYLOAD_SIZE > 0
    else
    {
        /* the commited size is less than de minimum */
        rv = false;
    }
#endif

    return rv;
}

#if LALLOC_ALLOW_QUEUED_FREES == 1
/**
   @brief it frees up the last added block

   @param obj
   @return int
 */
bool lalloc_free_last( LALLOC_T *obj )
{
    bool rv;

    LALLOC_CRITICAL_START;

    /* calculate the index of the 1st byte of the payload */
    LALLOC_IDX_TYPE idx = obj->dyn->alist + lalloc_b_overhead_size;

    rv = _block_move_from_alloc_to_free( obj, &( obj->pool[idx] ) );

    LALLOC_CRITICAL_END;

    return rv;
}

/* it frees up the first added block */
bool lalloc_free_first( LALLOC_T *obj )
{
    bool rv;
    LALLOC_IDX_TYPE idx;

    LALLOC_CRITICAL_START;

    /* calculate the index of the 1st byte of the payload */
    if ( obj->dyn->alist != LALLOC_IDX_INVALID )
    {
        LALLOC_GET_BLOCK_PREV( obj->pool, obj->dyn->alist, idx );

        ( idx ) = LALLOC_BLOCK_PREV( obj->pool, obj->dyn->alist );

        idx += lalloc_b_overhead_size;

        rv = _block_move_from_alloc_to_free( obj, &( obj->pool[idx] ) );
    }
    else
    {
        rv = false;
    }

    LALLOC_CRITICAL_END;

    return rv;
}
#endif

/**
   @brief Frees up the block of a given address.

          which payload includes addr memory address. *
          lalloc_free(0) must behaves just like lalloc_free_first()
          if the operation was ok, it return 1. 0 otherwise

   @param obj
   @param addr
   @return int
 */
bool lalloc_free( LALLOC_T *obj, void *addr )
{
    bool rv;
    bool in_global_range = ( uint8_t * )addr >= obj->pool && ( uint8_t * )addr < obj->pool + obj->size;

    if ( in_global_range )
    {
        LALLOC_CRITICAL_START;
        // TODO OPTIMIZATION FOR #if LALLOC_ALLOW_QUEUED_FREES==1 AND FREE ANY COMBINATIONS. ALIST IS NOT NEEDED IN SOME CASES.
        rv = _block_move_from_alloc_to_free( obj, addr );
        LALLOC_CRITICAL_END;
    }
    else
    {
        rv = false;
    }

    return rv;
}

/**
   @brief gets the free space of the object

   @param obj
   @return LALLOC_IDX_TYPE
 */
LALLOC_IDX_TYPE lalloc_get_free_space( LALLOC_T *obj )
{
    LALLOC_IDX_TYPE size;
    uint8_t *pdata_dummmy;

    LALLOC_CRITICAL_START;

    if ( LALLOC_IDX_INVALID != obj->dyn->flist )
    {
        /* if the free list is not empty, the information returned is its first block's size */
        _block_get_data( obj->pool, obj->dyn->flist, &pdata_dummmy, &size );
    }
    else
    {
        /* full, free=0 */
        size = 0;
    }

    LALLOC_CRITICAL_END;

    return size;
}

/**
   @brief returns if the obj is full or not

   @param obj
   @return true
   @return false
 */
bool lalloc_is_full( LALLOC_T *obj ) // TODO: NON TESTED
{
    LALLOC_IDX_TYPE cnt = lalloc_get_free_space( obj );
    return ( cnt == 0 );
}

/**
   @brief returns if the obj is empty or not

   @param obj
   @return true
   @return false
 */
bool lalloc_is_empty( LALLOC_T *obj ) // TODO: NON TESTED
{
    LALLOC_IDX_TYPE cnt = lalloc_get_alloc_count( obj );
    return ( cnt == 0 );
}

/**
   @brief returns 1 if the object hasn't allocated some space

   @param obj
   @return true
   @return false
 */
bool lalloc_is_none_allocated( LALLOC_T *obj ) // TODO: NON TESTED
{
    return ( obj->dyn->alloc_block == LALLOC_IDX_INVALID );
}

/* returns the allocated packet count */
LALLOC_IDX_TYPE lalloc_get_alloc_count( LALLOC_T *obj )
{
    LALLOC_IDX_TYPE n;

    LALLOC_CRITICAL_START;

    n = obj->dyn->allocated_blocks;

    LALLOC_CRITICAL_END;

    return n;
}

/**
   @brief Gets the nth  logical allocated element
          |ELEM0|->|ELEM1|->|ELEM2|->....->|ELEMn|->....->|ELEM0|
   @param obj
   @param addr
   @param size
   @param n
 */
void lalloc_get_n( LALLOC_T *obj, void **addr, LALLOC_IDX_TYPE *size, LALLOC_IDX_TYPE n )
{
    LALLOC_CRITICAL_START;

    /* the alocated list is sorted backwards, so the 0 element is the last. */
    LALLOC_IDX_TYPE cnt = obj->dyn->allocated_blocks;

    n = cnt - n - 1;

    _block_list_get_n( obj->pool, obj->dyn->alist, n, ( uint8_t ** )addr, size );

    LALLOC_CRITICAL_END;
}

/* v1.00 */
