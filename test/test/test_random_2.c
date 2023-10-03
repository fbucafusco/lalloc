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
#include "random_tools.h"
#include "lalloc_abstraction.h"


extern int isr_dis;

typedef struct
{
    char *title;                   // Test descripion
    uint32_t simulations_count;    //# of simulations per test
    LALLOC_IDX_TYPE blocksize_min; // minimum size of the element to insert to the adt
    LALLOC_IDX_TYPE blocksize_max; // maximum size of the element to insert to the adt
    LALLOC_IDX_TYPE pool_size;     // pool size
} tTestParams_In;

/* estructuras */
typedef struct
{
    uint32_t efficiency_avrg; // average efficiency
    uint32_t num_allocs;
    uint32_t num_bytes_written;
} tTestParams_Out;

/* This test:
   - repeats "simulations_count" times the next procedure:
     - fills all the pool with random size elements
     - removes from the the half of the elements
     - from the remaining # of elements, it removes the middle one.
     - TODO: alternating even and odd # of bytes per element
*/
void random_test( LALLOC_T *obj, tTestParams_In *params, tTestParams_Out *paramsout )
{
    isr_dis = 0;
    static uint32_t runcount = 0;

    uint32_t i;
    uint32_t j;
    LALLOC_IDX_TYPE current_charnum;
    LALLOC_IDX_TYPE given_size;

    uint8_t *p_mem;

    LALLOC_IDX_TYPE cant;
    char message[200];

    paramsout->num_bytes_written = 0;
    paramsout->num_allocs = 0;

    /* initializes the object */
    lalloc_init( obj );

    for ( i = 0; i < params->simulations_count; i++ )
    {
        /* fill the object */
        while ( 1 )
        {
            /* get the available space */
            LALLOC_IDX_TYPE free = lalloc_get_free_space( obj );

            /* it should be different than MEMQ_IDX_INVALID */
            TEST_ASSERT_FALSE( LALLOC_IDX_INVALID == free );
            TEST_ASSERT_TRUE( isr_dis == 0 );

            /* if there is no free space, if quits the while loop */
            if ( free == 0 )
            {
                break;
            }

            /* generate a random number of bytes to insert */
            current_charnum = uint32_random_range( params->blocksize_min, params->blocksize_max );

            /* request memory space */
            lalloc_alloc( obj, ( void** ) &p_mem, &given_size );

            TEST_ASSERT_TRUE( lalloc_sanity_check( obj ) );
            TEST_ASSERT_TRUE( isr_dis == 0 );

            if ( current_charnum > given_size )
            {
                current_charnum = given_size / 2;
            }

            if ( current_charnum > free )
            {
                current_charnum = free / 2;
            }

            if ( current_charnum == 0 )
            {
                current_charnum = free;
            }

            /* fill with random data. */
            random_byte_array( current_charnum, p_mem );

            sprintf( message, "Iteration %u, Given byte #: %u written byte # %u", i, given_size, current_charnum );

            lalloc_commit( obj, current_charnum );

            TEST_ASSERT_TRUE( lalloc_sanity_check( obj ) );
            TEST_ASSERT_TRUE( isr_dis == 0 );

            paramsout->num_allocs++;
            paramsout->num_bytes_written += current_charnum;
        }

        /* the free list should be empty, because the end condition of the loop  */
        if ( LALLOC_MIN_PAYLOAD_SIZE > 0 )
        {
            TEST_ASSERT_EQUAL_INT( LALLOC_IDX_INVALID, obj->dyn->flist );
        }

        LALLOC_IDX_TYPE num_allocs = lalloc_get_alloc_count( obj );

        TEST_ASSERT_TRUE_MESSAGE( lalloc_sanity_check( obj ), message );
        TEST_ASSERT_TRUE( isr_dis == 0 );
        TEST_ASSERT_TRUE( num_allocs > 0 );
        TEST_ASSERT_TRUE( isr_dis == 0 );
        /*
           if num_allocs = 1 the list is emptied.
           if num_allocs = 2 | 3 | 4 it removes the half # of elements
           if otherwise it removes the half # of elements + the middle one of the remaining elements
        */
        int delete_central = 0;
        LALLOC_IDX_TYPE num_allocs_delete = 0;

        if ( num_allocs == 1 )
        {
            num_allocs_delete = 1;
        }
        else if ( num_allocs < 5 )
        {
            num_allocs_delete = num_allocs / 2;
        }
        else if ( num_allocs > 4 )
        {
            num_allocs_delete = num_allocs / 2;
            delete_central = 1;
        }

        /* free upo the oldest half */
        sprintf( message, "Iteration %u removed packages %u", i, num_allocs_delete );

        for ( j = 0; j < num_allocs_delete; j++ )
        {
            lalloc_get_n( obj, ( void** ) &p_mem, &cant, 0 );
            TEST_ASSERT_TRUE( isr_dis == 0 );
            lalloc_free( obj,  ( void* ) p_mem );
            TEST_ASSERT_TRUE( isr_dis == 0 );
        }

        TEST_ASSERT_TRUE_MESSAGE( lalloc_sanity_check( obj ), message );

        if ( delete_central )
        {
            delete_central = ( num_allocs - num_allocs_delete ) / 2;

            /* tambien libero uno del medio. */
            sprintf( message, "Iteration %u remain %u and removed %u", i, num_allocs - num_allocs_delete, delete_central );

            lalloc_get_n( obj, ( void** ) &p_mem, &cant, delete_central );
            TEST_ASSERT_TRUE( isr_dis == 0 );
            TEST_ASSERT_NOT_NULL( p_mem );

            lalloc_free( obj, ( void* ) p_mem );
            TEST_ASSERT_TRUE( isr_dis == 0 );
            TEST_ASSERT_TRUE_MESSAGE( lalloc_sanity_check( obj ), message );
        }
    }



    /* Final calculations and summary */
    float a;
    printf( "test %s run count %u\n", __FUNCTION__, runcount );
    printf( "Simulations: ........................... %u\n", params->simulations_count );
    printf( "Buffer Size: ........................... %u B\n", params->pool_size );
    a = paramsout->num_bytes_written / 1024;
    printf( "Processed bytes ........................ %0.2f kB\n", a );
    a = ( float )paramsout->num_bytes_written / paramsout->num_allocs;
    printf( "Mean element size ...................... %3.2f B\n", a );


    runcount++;
}

/* WHITE BOX TEST
 * TEST THE ALLOCATION COMMIT AND FREE OPERATIONS, AND VALIDATES THE INTEGRITY OF THE LIST AFTER THEM.
 * IT VALIDATES THE COALESCENSE ALGORITM FOR THE FREED NODES.
 * */
void test_random_1()
{
    tTestParams_In params_in;
    tTestParams_Out paramsout;

    uint32_t pool_size = uint32_random_range( 50, 30000 );

    params_in.pool_size = pool_size;
    params_in.simulations_count = 2000;
    params_in.blocksize_min = 10;
    params_in.blocksize_max = pool_size;
    params_in.title = "test random";

    LALLOC_DECLARE( test_alloc, pool_size, 0 );

    random_test( &test_alloc, &params_in, &paramsout );
}
