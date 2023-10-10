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

#include "unity.h"
#include "lalloc.h"
#include "lalloc_priv.h"
#include "malloc_replace.h"
#include "lalloc_abstraction.h"


/* internal private data from lalloc.c */
extern const LALLOC_IDX_TYPE lalloc_b_overhead_size;

/**
   @brief WHITE BOX TEST
          TEST THE ALLOCATION COMMIT AND FREE OPERATIONS, AND VALIDATES THE INTEGRITY OF THE LIST AFTER THEM.
          IT VALIDATES THE COALESCENSE ALGORITM FOR THE FREED NODES.
 */
void test_lalloc_1()
{
    int i;
    uint8_t *data;
    LALLOC_IDX_TYPE size;
    char text1[] = "walkingded";       // 10
    char text2[] = "elementary";       // 10
    char text3[] = "ingodwetst";       // 10
    char text4[] = "santaclaritadiet"; // 16
    char text5[] = "uh";               // 2

    char * text_arr[5] = {text1, text2, text3, text4, text5};
    size_t size_arr[5] = {sizeof(text1)-1 , sizeof(text2)-1 , sizeof(text3)-1, sizeof(text4)-1 , sizeof(text5)-1};
    uint8_t *addresses[5];

    size_t pool_size = 0;

    for ( i = 0; i < 5; i++ )
    {
    	pool_size += LALLOC_ALIGN_ROUND_UP(size_arr[i]) + LALLOC_NODE_HEAD_SIZE;
    }

    LALLOC_DECLARE( test_alloc, pool_size, 0 );

    lalloc_init( &test_alloc );

    /* allocation of all free space in the pool */
    for ( i = 0; i < 5; i++ )
    {
        lalloc_alloc( &test_alloc, ( void ** )&addresses[i], &size );

        memcpy( addresses[i], text_arr[i],   size_arr[i]  );

        lalloc_commit( &test_alloc, strlen( text_arr[i] ) );
    }

    /* there shouldn't be any free node */
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->flist );

    /* free 2 "middle" nodes */
    lalloc_free( &test_alloc, addresses[2] );
    lalloc_free( &test_alloc, addresses[3] );

    /* ALIST is build as a LIFO */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->alist, 2, &data, &size );
    TEST_ASSERT_EQUAL_STRING_LEN( text1, data, strlen( text1 ) );
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->alist, 1, &data, &size );
    TEST_ASSERT_EQUAL_STRING_LEN( text2, data, strlen( text2 ) );
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->alist, 0, &data, &size );
    TEST_ASSERT_EQUAL_STRING_LEN( text5, data, strlen( text5 ) );
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->alist, 3, &data, &size ); // it must give null
    TEST_ASSERT_NULL( data );
    TEST_ASSERT_EQUAL( 0, size );

    /* COUNT THE FREE NDOES.
     * During the lastest operations there were removed the "middle nodes"  so there must be a middle node "joined". Just 1 */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 0, &data, &size );

    TEST_ASSERT_EQUAL( LALLOC_ALIGN_ROUND_UP(strlen( text3 ) + strlen( text4 ) + lalloc_b_overhead_size), size );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 1, &data, &size );
    TEST_ASSERT_NULL( data );
    TEST_ASSERT_EQUAL( 0, size );
}

/**
   @brief   WHITE BOX TEST
            SAME AS TEST_LALLOC_01, BUT WITH ANOTHER SEQUENCE OF DELETIONS
 */
void test_lalloc_2()
{
    int i;
    uint8_t *data;
    LALLOC_IDX_TYPE size;

    char text1[] = "walkingded";       // 10
    char text2[] = "elementary";       // 10
    char text3[] = "ingodwetst";       // 10
    char text4[] = "santaclaritadiet"; // 16
    char text5[] = "uh";               // 2

    char *text_arr[5] = {text1, text2, text3, text4, text5};
    uint8_t *addresses[5];

    LALLOC_DECLARE( test_alloc, 100, 0 );

    lalloc_init( ( lalloc_t *const )&test_alloc );

    /* allocation of all free space in the pool */
    for ( i = 0; i < 5; i++ )
    {
        lalloc_alloc( &test_alloc, ( void ** )&addresses[i], &size );

        memcpy( addresses[i], text_arr[i], strlen( text_arr[i] ) );

        lalloc_commit( &test_alloc, strlen( text_arr[i] ) );
    }

    /* there shouldn't be any free node */
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->flist );

    /* free 2 "middle" nodes */
    lalloc_free( &test_alloc, addresses[1] );
    lalloc_free( &test_alloc, addresses[3] );
    lalloc_free( &test_alloc, addresses[2] );

    /* ALIST is build as a LIFO */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->alist, 1, &data, &size );
    TEST_ASSERT_EQUAL_STRING_LEN( text1, data, strlen( text1 ) );
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->alist, 0, &data, &size );
    TEST_ASSERT_EQUAL_STRING_LEN( text5, data, strlen( text5 ) );
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->alist, 2, &data, &size ); // it must give null
    TEST_ASSERT_NULL( data );
    TEST_ASSERT_EQUAL( 0, size );

    /* COUNT THE FREE NDOES.
     * During the lastest operations there were removed the "middle nodes"  so there must be a middle node "joined". Just 1 */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 0, &data, &size );

    TEST_ASSERT_EQUAL( strlen( text2 ) + strlen( text3 ) + strlen( text4 ) + 2 * lalloc_b_overhead_size, size );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 1, &data, &size );
    TEST_ASSERT_NULL( data );
    TEST_ASSERT_EQUAL( 0, size );
}

/**
   @brief   WHITE BOX TEST
            SAME AS TEST_LALLOC_01, BUT WITH FULL DELETION
 */
void test_lalloc_3a()
{
    int i;
    uint8_t *data;
    LALLOC_IDX_TYPE size;
    char text1[] = "walkingded";       // 10
    char text2[] = "elementary";       // 10
    char text3[] = "ingodwetst";       // 10
    char text4[] = "santaclaritadiet"; // 16
    char text5[] = "uh";               // 2

    char *text_arr[5] = {text1, text2, text3, text4, text5};
    uint8_t *addresses[5];

    LALLOC_DECLARE( test_alloc, 100, 0 );

    lalloc_init( &test_alloc );

    /* allocation of all free space in the pool */
    for ( i = 0; i < 5; i++ )
    {
        lalloc_alloc( &test_alloc, ( void ** )&addresses[i], &size );

        memcpy( addresses[i], text_arr[i], strlen( text_arr[i] ) );

        lalloc_commit( &test_alloc, strlen( text_arr[i] ) );
    }

    /* there shouldn't be any free node */
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->flist );

    /* free 2 "middle" nodes */
    lalloc_free( &test_alloc, addresses[1] );
    lalloc_free( &test_alloc, addresses[3] );
    lalloc_free( &test_alloc, addresses[4] );
    lalloc_free( &test_alloc, addresses[2] );
    lalloc_free( &test_alloc, addresses[0] );

    /* ALIST is build as a LIFO and should be invalid */
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alist );

    /* COUNT THE FREE NDOES.
     * During the lastest operations there were removed the all the nodes so there must be an only node "joined". Just 1 */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 0, &data, &size );

    TEST_ASSERT_EQUAL( test_alloc.size - lalloc_b_overhead_size, size );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 1, &data, &size );

    TEST_ASSERT_NULL( data );
    TEST_ASSERT_EQUAL( 0, size );
}

/**
   @brief   WHITE BOX TEST
            similar to test_lalloc_3a.
            Uses dynamic allocation
            Creates memory spaces until there is no more room and clears everything
            Also uses dinamically alocated object
 */
void test_lalloc_3b()
{
    int i;
    int count = 0;
    uint8_t *data;
    LALLOC_IDX_TYPE size;
    char text1[] = "walkingded";       // 10
    char text2[] = "elementary";       // 10
    char text3[] = "ingodwetst";       // 10
    char text4[] = "santaclaritadiet"; // 16
    char text5[] = "uh";               // 2
    char text6[] = "casablanca";       // 10

    /* exactly storage for 5 txts (without \0) */
    uint32_t pool_size = 5 * lalloc_b_overhead_size + strlen( text1 ) + strlen( text2 ) + strlen( text3 ) + strlen( text4 ) + strlen( text5 );

    char *text_arr[6] = {text1, text2, text3, text4, text5, text6};
    uint8_t *addresses[6];

    lalloc_t *test_alloc = lalloc_ctor( pool_size );

    /* allocation of all free space in the pool */
    for ( i = 0; i < 6; i++ )
    {
        lalloc_alloc( test_alloc, ( void ** )&addresses[i], &size );
        // printf("ptr %p\n", addresses[i]);

        if ( addresses[i] != NULL )
        {
            count++;
            memcpy( addresses[i], text_arr[i], strlen( text_arr[i] ) );
            lalloc_commit( test_alloc, strlen( text_arr[i] ) );
        }
    }

    /* validate the 5 allocations , 1 fails because of lack of space */
    TEST_ASSERT_EQUAL( 5, count );

    TEST_ASSERT_EQUAL( count, lalloc_get_alloc_count( test_alloc ) );

    /* validates lalloc_get_n */
    lalloc_get_n( test_alloc, ( void ** )&data, &size, 2 );
    TEST_ASSERT_EQUAL_PTR( addresses[2], data );
    TEST_ASSERT_EQUAL( size, strlen( text_arr[2] ) );

    /* there shouldn't be any free node */
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc->dyn->flist );

    /* free 2 "middle" nodes */
    lalloc_free( test_alloc, addresses[1] );
    lalloc_free( test_alloc, addresses[3] );

#if LALLOC_ALLOW_QUEUED_FREES==1
    lalloc_free_last( test_alloc ); // index 4
#else
    lalloc_free ( test_alloc, addresses[4] );
#endif

    lalloc_free( test_alloc, addresses[2] );
    lalloc_free( test_alloc, addresses[0] );

    /* every node should have been deleted */
    TEST_ASSERT_EQUAL( 0, lalloc_get_alloc_count( test_alloc ) );

    /* ALIST is build as a LIFO and should be invalid */
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc->dyn->alist );

    /* COUNT THE FREE NODES.
       During the lastest operations there were removed the all the
       nodes so there must be an only node "joined". Just 1 */
    _block_list_get_n( test_alloc->pool, test_alloc->dyn->flist, 0, &data, &size );

    TEST_ASSERT_EQUAL( test_alloc->size - lalloc_b_overhead_size, size );

    _block_list_get_n( test_alloc->pool, test_alloc->dyn->flist, 1, &data, &size );

    TEST_ASSERT_NULL( data );
    TEST_ASSERT_EQUAL( 0, size );

    lalloc_dtor( test_alloc );
}

/**
   @brief   WHITE BOX TEST
            IT CROSSES THE STATES OF THE LISTS BY FREEING AN ALLOCATED/COMMITED BLOCK WHILE BEING ANOTHER BLOCK BEING ALLOCATED
            THIS CASE HANDLES THE FIRST COMMIT WITH A SIZE LESS THAN THE HALF OF THE POOL
 */
void test_lalloc_4a()
{
    int i;
    uint8_t *pData0;
    uint8_t *pData1;
    LALLOC_IDX_TYPE size;
    const LALLOC_IDX_TYPE pool_size = 100;
    const LALLOC_IDX_TYPE elment_size = pool_size / 4;

    LALLOC_DECLARE( test_alloc, pool_size, 0 );

    lalloc_init( &test_alloc );

    lalloc_alloc( &test_alloc, ( void ** )&pData0, &size );

    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->flist );
    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->alloc_block );

    lalloc_commit( &test_alloc, elment_size );

    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->flist );
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alloc_block );

    lalloc_alloc( &test_alloc, ( void ** )&pData1, &size );

    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->flist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->alloc_block );

    lalloc_free( &test_alloc, pData0 );

    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->flist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->alloc_block );

    lalloc_commit( &test_alloc, elment_size );

    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->flist ); // the commit optimized the free list
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alloc_block );
}

/**
   @brief   WHITE BOX TEST
            IT CROSSES THE STATES OF THE LISTS BY FREEING AN ALLOCATED/COMMITED BLOCK WHILE BEING ANOTHER BLOCK BEING ALLOCATED
            THIS CASE HANDLES THE FIRST COMMIT WITH A SIZE BIGGER THAN THE HALF OF THE POOL
 */
void test_lalloc_4b()
{
    int i;
    uint8_t *pData0;
    uint8_t *pData1;
    LALLOC_IDX_TYPE size;
    const LALLOC_IDX_TYPE pool_size = 100;
    const LALLOC_IDX_TYPE elment_size = ( pool_size * 3 ) / 4;

    LALLOC_DECLARE( test_alloc, pool_size, 0 );

    lalloc_init( &test_alloc );

    lalloc_alloc( &test_alloc, ( void ** )&pData0, &size );

    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->flist );
    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->alloc_block );

    lalloc_commit( &test_alloc, elment_size );

    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->flist );
    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alloc_block );

    lalloc_alloc( &test_alloc, ( void ** )&pData1, &size );

    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->flist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->alloc_block );

    lalloc_free( &test_alloc, pData0 );

    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->flist );
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->alloc_block );

    lalloc_commit( &test_alloc, elment_size ); // THIS WONT WORK BECAUSE THERE IS NO SPACE.

    TEST_ASSERT_EQUAL( LALLOC_IDX_INVALID, test_alloc.dyn->alist );
    TEST_ASSERT_EQUAL( 0, test_alloc.dyn->flist ); // the commit optimized the free list
    TEST_ASSERT_EQUAL( elment_size + lalloc_b_overhead_size, test_alloc.dyn->alloc_block );
}

void test_lalloc_free_non_valid_node()
{
    uint8_t *data;
    LALLOC_IDX_TYPE size;

    LALLOC_DECLARE( test_alloc, 100, 0 );

    lalloc_init( &test_alloc );

    lalloc_alloc( &test_alloc, ( void ** )&data, &size );
    lalloc_commit( &test_alloc, 10 );

    /* try to destroy a node not belonging to the pool */
    int res = lalloc_free( &test_alloc, &test_alloc_pool[test_alloc.size + 3] );

    TEST_ASSERT_EQUAL( false, res );
}

void test_lalloc_free_valid_nodes()
{
    uint8_t *data0;
    uint8_t *data1;
    uint8_t *data3;

    LALLOC_IDX_TYPE size;
    LALLOC_IDX_TYPE size2;

    LALLOC_DECLARE( test_alloc, 100, 0 );

    lalloc_init( &test_alloc );
    //should fail
    lalloc_get_n( &test_alloc, &data3, &size2, 2 );

    TEST_ASSERT_EQUAL( data3, NULL );
    TEST_ASSERT_EQUAL( size2, 0 );

    lalloc_alloc( &test_alloc, ( void ** )&data0, &size );
    lalloc_commit( &test_alloc, 10 );
    lalloc_alloc( &test_alloc, ( void ** )&data1, &size );
    lalloc_commit( &test_alloc, 10 );

    int res;
    /* try to destroy a node not belonging to the pool */


#if LALLOC_ALLOW_QUEUED_FREES==1
    res = lalloc_free_first( &test_alloc );
    TEST_ASSERT_EQUAL( true, res );
#else
    res = lalloc_free( &test_alloc, data0 );
    TEST_ASSERT_EQUAL( true, res );
#endif

    res = lalloc_free( &test_alloc, data1 );
    TEST_ASSERT_EQUAL( true, res );
}


void test_lalloc_commit_without_alloc()
{
    uint8_t *data0;
    uint8_t *data1;

    LALLOC_IDX_TYPE size;

    LALLOC_DECLARE( test_alloc, 100, 0 );

    lalloc_init( &test_alloc );

    int rv =   lalloc_commit( &test_alloc, 10 );

    TEST_ASSERT_EQUAL( false, rv );

    lalloc_alloc( &test_alloc, ( void ** )&data0, &size );

    lalloc_alloc_revert( &test_alloc );

    rv =   lalloc_commit( &test_alloc, 10 );

    TEST_ASSERT_EQUAL( false, rv );
}

uint32_t heap_test[200];
uint32_t freecount = 0;

void *malloc_test( size_t size )
{
    static int run = 0;

    if ( run == 0 )
    {
        run++;
        return NULL;
    }

    if ( run == 1 )
    {
        run++;
        return heap_test;
    }
    if ( run == 2 )
    {
        run++;
        return NULL;   ;
    }
    if ( run == 3 )
    {
        run++;
        return heap_test;
    }
    if ( run == 4 )
    {
        run++;
        return heap_test + sizeof( lalloc_t ) ;
    }
    if ( run == 5 )
    {
        run++;
        return NULL;   ;
    }
    return NULL;
}

void free_test( void *ptr )
{
    freecount++;
    // nothing
}

void test_lalloc_ctor_fails()
{
    mem_din_set( malloc_test, free_test );

    lalloc_t *test_alloc = lalloc_ctor( 10 ); // run 1
    TEST_ASSERT_EQUAL( NULL, test_alloc );
    TEST_ASSERT_EQUAL( freecount, 0 );

    test_alloc = lalloc_ctor( 10 ); // run 2
    TEST_ASSERT_EQUAL( NULL, test_alloc );
    TEST_ASSERT_EQUAL( freecount, 1 );

    test_alloc = lalloc_ctor( 10 ); // run 2
    TEST_ASSERT_EQUAL( NULL, test_alloc );
    TEST_ASSERT_EQUAL( freecount, 3 );


    mem_din_set( NULL, NULL );
}
