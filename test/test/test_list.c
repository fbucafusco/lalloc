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
#include "lalloc_tools.h"
#include "lalloc_abstraction.h"

/* internal private data from lalloc.c */
extern const LALLOC_IDX_TYPE lalloc_b_overhead_size;

/* simulates the biggest node removal */
void test_list_related_1()
{
    LALLOC_DECLARE( test_alloc, 100, 0 );

    lalloc_init( &test_alloc );

    LALLOC_IDX_TYPE removed = _block_remove( test_alloc.pool, &( test_alloc.dyn->flist ) );

    /* SE REMOVIO EL ELEMENTO 0*/
    TEST_ASSERT_EQUAL_INT( 0, removed );

    /* LA LISTA QUEDO VACIA */
    TEST_ASSERT_EQUAL_INT( LALLOC_IDX_INVALID, test_alloc.dyn->flist );
}

/* simulates the node insertion and the removal of a middle node */
void test_list_related_2()
{
    char text1[] = "walkingded";
    char text2[] = "elementary";
    char text3[] = "ingodwetst";

    LALLOC_DECLARE( test_alloc, 100, 0 );

    lalloc_init( &test_alloc );

    LALLOC_IDX_TYPE removed = _block_remove( test_alloc.pool, &( test_alloc.dyn->flist ) );
    LALLOC_IDX_TYPE added;

    /* LA LISTA QUEDA VACIA */

    memset( test_alloc.pool, 0x55, test_alloc.size );

    /* create 2 artificial blocks within the pool */
    /* both blocks, HEADER + DATA = 20 bytes */

    _block_set( test_alloc.pool, 0, 10, 0, 0, LALLOC_FREE_NODE_MASK );
    _block_set_data( test_alloc.pool, 0, text1, sizeof( text1 ) - 1 );
    _block_set( test_alloc.pool, 20, 10, 20, 20, 0 );
    _block_set_data( test_alloc.pool, 20, text2, sizeof( text2 ) - 1 );
    _block_set( test_alloc.pool, 40, 10, 40, 40, 0 );
    _block_set_data( test_alloc.pool, 40, text3, sizeof( text3 ) - 1 );

    /* 20 first */
    added = 20;
    _block_list_add_first( test_alloc.pool, &( test_alloc.dyn->flist ), added );

    TEST_ASSERT_EQUAL_INT( 20, test_alloc.dyn->flist );

    /* 0 second */
    added = 0;
    _block_list_add_first( test_alloc.pool, &( test_alloc.dyn->flist ), added );

    TEST_ASSERT_EQUAL_INT( 0, test_alloc.dyn->flist );

    /* 36 third  */
    added = 40;
    _block_list_add_first( test_alloc.pool, &( test_alloc.dyn->flist ), added );

    TEST_ASSERT_EQUAL_INT( 40, test_alloc.dyn->flist );

    uint8_t *data;
    LALLOC_IDX_TYPE size;

    /* we move through the list  */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 0, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text3, data, sizeof( text3 ) - 1 );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 1, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text1, data, sizeof( text1 ) - 1 );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 2, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text2, data, sizeof( text2 ) - 1 );

    /* non existant element */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 3, &data, &size );

    TEST_ASSERT_NULL( data );
    TEST_ASSERT_EQUAL_INT( 0, size );

    LALLOC_IDX_TYPE remove = 0;

    removed = _block_remove( test_alloc.pool, &( remove ) );

    /* the removed item is 0 */
    TEST_ASSERT_EQUAL_INT( 0, removed );

    /* we move through the list, again */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 0, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text3, data, sizeof( text3 ) - 1 );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 1, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text2, data, sizeof( text2 ) - 1 );
}

/* sorted node insertion */
void test_list_related_3()
{
    char text1[] = "walking dead";                     // 12
    char text2[] = "supercalifragilisticoespialidoso"; // 32
    char text3[] = "jumanji";                          // 7
    char text4[] = "umpalumpa";                        // 9
    char text5[] = "pen";                              // 3

    char *text_arr[5] = {text1, text2, text3, text4, text5};
    LALLOC_IDX_TYPE indexes[5];
    uint16_t i;
    uint16_t acum = 0;

    LALLOC_DECLARE( test_alloc, 300, 0 );

    lalloc_init( &test_alloc );

    // LALLOC_IDX_TYPE removed = _block_remove ( test_alloc.pool,  & ( test_alloc.dyn->flist )  );
    LALLOC_IDX_TYPE added;

    /* list is empty */

    memset( test_alloc.pool, 0x55, test_alloc.size );

    /* create 3 artificial blocks within the pool */
    /*TODO REMOVE COMMENT both blocks, HEADER + DATA + FOOTER = 18 bytes */
    for ( i = 0; i < 5; i++ )
    {
        _block_set( test_alloc.pool, acum, strlen( text_arr[i] ), acum, acum, 0 );
        _block_set_data( test_alloc.pool, acum, text_arr[i], strlen( text_arr[i] ) );
        indexes[i] = acum;
        acum += lalloc_b_overhead_size + strlen( text_arr[i] );
    }

    added = indexes[0];
    _block_list_add_sorted( test_alloc.pool, &( test_alloc.dyn->flist ), added );

    TEST_ASSERT_EQUAL_INT( indexes[0], test_alloc.dyn->flist );

    added = indexes[1];
    _block_list_add_sorted( test_alloc.pool, &( test_alloc.dyn->flist ), added );

    TEST_ASSERT_EQUAL_INT( indexes[1], test_alloc.dyn->flist );

    added = indexes[2];
    _block_list_add_sorted( test_alloc.pool, &( test_alloc.dyn->flist ), added );

    TEST_ASSERT_EQUAL_INT( indexes[1], test_alloc.dyn->flist );

    added = indexes[3];
    _block_list_add_sorted( test_alloc.pool, &( test_alloc.dyn->flist ), added );

    TEST_ASSERT_EQUAL_INT( indexes[1], test_alloc.dyn->flist );

    added = indexes[4];
    _block_list_add_sorted( test_alloc.pool, &( test_alloc.dyn->flist ), added );

    TEST_ASSERT_EQUAL_INT( indexes[1], test_alloc.dyn->flist );

    uint8_t *data;
    LALLOC_IDX_TYPE size;

    /* we move through the list  */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 0, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text_arr[1], data, strlen( text_arr[1] ) );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 1, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text_arr[0], data, strlen( text_arr[0] ) );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 2, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text_arr[3], data, strlen( text_arr[3] ) );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 3, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text_arr[2], data, strlen( text_arr[2] ) );

    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 4, &data, &size );

    TEST_ASSERT_EQUAL_STRING_LEN( text_arr[4], data, strlen( text_arr[4] ) );

    /* non existant element */
    _block_list_get_n( test_alloc.pool, test_alloc.dyn->flist, 5, &data, &size );

    TEST_ASSERT_NULL( data );
    TEST_ASSERT_EQUAL_INT( 0, size );
}
#if 1
/* join  */
void test_list_join()
{
    int nodes = 20;
    LALLOC_IDX_TYPE data_size = 4;
    LALLOC_IDX_TYPE indexes[nodes];
    uint8_t *addreses[nodes];
    uint16_t i;
    uint16_t acum = 0;

    LALLOC_DECLARE( test_alloc, nodes*( lalloc_b_overhead_size+data_size ), 0 );

    lalloc_init( &test_alloc );

    LALLOC_IDX_TYPE added;

    /* list is empty */


    /*TODO REMOVE COMMENT both blocks, HEADER + DATA + FOOTER = 18 bytes */
    uint8_t *addr;
    LALLOC_IDX_TYPE size;

    /* create 20 nodes, with dummy prev next indexes */
    for ( i = 0; i < nodes; i++ )
    {
        lalloc_alloc( &test_alloc, &addr, &size );
        lalloc_commit( &test_alloc, data_size );
        addreses[i] = addr;
        indexes[i] = addr - test_alloc.pool - lalloc_b_overhead_size;
    }

    LALLOC_IDX_TYPE removed;

    // remove the [1] node, then the [2] and join   //joins left
    lalloc_free( &test_alloc, addreses[1] );
    lalloc_free( &test_alloc, addreses[2] );

    // free list must be the node indexes[1]
    _block_get_data( test_alloc.pool, indexes[1], &addr, &size );

    TEST_ASSERT_EQUAL( 2 * data_size + lalloc_b_overhead_size, size );
    TEST_ASSERT_EQUAL( 2 * data_size + lalloc_b_overhead_size,  lalloc_get_free_space( &test_alloc ) );

    //remove 0                                      //joins rigth
    lalloc_free( &test_alloc, addreses[0] );
    _block_get_data( test_alloc.pool, indexes[0], &addr, &size );

    TEST_ASSERT_EQUAL( 3 * data_size + 2* lalloc_b_overhead_size, size );
    TEST_ASSERT_EQUAL( 3 * data_size + 2*  lalloc_b_overhead_size,  lalloc_get_free_space( &test_alloc ) );

    //remove [7], [5], [6]                          //joins both sides
    lalloc_free( &test_alloc, addreses[7] );
    lalloc_free( &test_alloc, addreses[5] );
    lalloc_free( &test_alloc, addreses[6] );

    _block_get_data( test_alloc.pool, indexes[5], &addr, &size );
    TEST_ASSERT_EQUAL( 3 * data_size + 2* lalloc_b_overhead_size, size );
    TEST_ASSERT_EQUAL( 3 * data_size + 2* lalloc_b_overhead_size,  lalloc_get_free_space( &test_alloc ) );  //gives the gratest node's size

    //remove [4]
    lalloc_free( &test_alloc, addreses[4] );
    _block_get_data( test_alloc.pool, indexes[4], &addr, &size );
    TEST_ASSERT_EQUAL( 4 * data_size + 3* lalloc_b_overhead_size, size );
    TEST_ASSERT_EQUAL( 4 * data_size + 3* lalloc_b_overhead_size,  lalloc_get_free_space( &test_alloc ) );

    //remove [3]
    lalloc_free( &test_alloc, addreses[3] );
    _block_get_data( test_alloc.pool, indexes[0], &addr, &size );
    TEST_ASSERT_EQUAL( 8 * data_size + 7* lalloc_b_overhead_size, size );
    TEST_ASSERT_EQUAL( 8 * data_size + 7* lalloc_b_overhead_size,  lalloc_get_free_space( &test_alloc ) );
}
#endif
/*
TODO:
test _block_list_find_by_ref
test _block_list_find_by_idx
*/
