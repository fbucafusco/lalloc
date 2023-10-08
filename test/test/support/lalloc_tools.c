#include "lalloc_priv.h"
#include <math.h>

/* internal private data from lalloc.c */
extern const LALLOC_IDX_TYPE lalloc_b_overhead_size;
extern const LALLOC_IDX_TYPE lalloc_block_size ;

/* TEST PURPOSES ONLY
   It saves data into the data field of the block  */
void _block_set_data( uint8_t *pool, LALLOC_IDX_TYPE block_idx, uint8_t *addr, LALLOC_IDX_TYPE size )
{
    LALLOC_IDX_TYPE block_size;
    uint8_t *pdataDest;

    LALLOC_GET_BLOCK_SIZE_FROMPOOL_( pool, block_idx, block_size );

    /* limit the length of the data */
    if ( block_size < size )
    {
        size = block_size;
    }

    LALLOC_GET_BLOCK_DATA_FROMPOOL_( pool, block_idx, pdataDest );

    memcpy( pdataDest, addr, size );
}

/**
   @brief Measures the fragmentation by averaging the the free blocks size against the total unallocated space.

          the squared root methos is taken from https://asawicki.info/news_1757_a_metric_for_memory_fragmentation

   @param obj
   @return uint16_t  returns a % x 10
 */
void lalloc_measure_framgentation( LALLOC_T *obj, float *sqbase, float *average_based )
{
    uint32_t n;
    float sum2 = 0;
    float sum = 0;

    LALLOC_CRITICAL_START;

    LALLOC_IDX_TYPE node = obj->dyn->flist;

    /* Take the flist element (the first) and return your information, and remove the flist node. */
    if ( LALLOC_IDX_INVALID != node )
    {
        n = 0;

        while ( 1 )
        {
            LALLOC_IDX_TYPE size;

            LALLOC_GET_BLOCK_SIZE_FROMPOOL_( obj->pool, node, size );
            size &= ~LALLOC_FREE_NODE_MASK;

            sum += size;
            sum2 += size * size;
            n++;

            LALLOC_GET_BLOCK_NEXT_FROMPOOL( obj->pool, node, node );
            if ( node == obj->dyn->flist )
            {
                break;
            }
        }

        // float mean = ((float)sum) / ((float)n);

        float frag = sqrtf( sum2 ) / sum;
        frag = 1 - frag * frag;

        *sqbase = 100.0 * frag;
        *average_based = 100.0 - ( 100 / n );
    }
    else
    {
        /* there isn't any node in the list  */
        *sqbase = 0.0;
        *average_based = 0.0;
    }

    LALLOC_CRITICAL_END;
}

/* verifica la integridad de datos de la cola TESTS PURPOSES ONLY
 * 1: ok,
 * 0: error
 * */
LALLOC_IDX_TYPE lalloc_sanity_check( LALLOC_T *obj )
{
    LALLOC_IDX_TYPE idx;
    LALLOC_IDX_TYPE next_phy;
    LALLOC_IDX_TYPE prev_phy;
    LALLOC_IDX_TYPE good1 = 1;
    LALLOC_IDX_TYPE good2 = 1;
    LALLOC_IDX_TYPE good3 = 1;
    LALLOC_IDX_TYPE good4 = 1;

    LALLOC_IDX_TYPE num_prev = 0;
    LALLOC_IDX_TYPE num_next = 0;
    LALLOC_IDX_TYPE size_sum = 0;
    LALLOC_IDX_TYPE size;

    LALLOC_CRITICAL_START;

    /* ==================================================================
       1st stage: validate physical boundries
       ==================================================================
    */
    idx = 0;

    /* forward validation */
    while ( 1 )
    {
        LALLOC_GET_BLOCK_NEXTPHYS_FROMPOOL_( obj->pool, idx, next_phy );
        LALLOC_GET_BLOCK_SIZE_FROMPOOL_( obj->pool, idx, size );

        /* clear free node flag */
        size &= ~LALLOC_FREE_NODE_MASK;
        size_sum += size;

        num_next++;

        if ( next_phy == obj->size )
        {
            /* end */
            good1 = 1;
            break;
        }

        /* if the next_phy block is not higher than the current idx, is an error */
        if ( !( next_phy > idx ) )
        {
            good1 = 0;
            break;
        }

        /* if the next_phy block is not less than the total pool size is an error */
        if ( !( next_phy < obj->size ) )
        {
            good1 = 0;
            break;
        }

        idx = next_phy;
    }

    /* idx stays at the last physical block.
       backward validation */
    while ( 1 )
    {
        LALLOC_GET_BLOCK_PREVPHYS_FROMPOOL_( obj->pool, idx, prev_phy );

        num_prev++;

        if ( prev_phy == LALLOC_IDX_INVALID )
        {
            /* end */
            good2 = 1;
            break;
        }

        /* if the prev_phy block is not lower than the current idx, is an error */
        if ( !( prev_phy < idx ) )
        {
            good2 = 0;
            break;
        }

        /* if the next_phy block is not less than the total pool size, is an error */
        if ( !( prev_phy < obj->size ) )
        {
            good2 = 0;
            break;
        }

        idx = prev_phy;
    }

    if ( num_prev != num_next )
    {
        good3 = 0;
    }
    else
    {
        /* Here verify that Nex's indices coincide */
    }

    if ( !( size_sum < obj->size ) )
    {
        good4 = 0;
    }


    /* 2nd stage: move through the lists in both directions.  TODO */



    LALLOC_CRITICAL_END;
    return good1 && good2 && good3 && good4;
}
