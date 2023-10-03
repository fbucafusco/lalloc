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
#include <time.h>
#include "unity.h"
#include "lalloc.h"
#include "lalloc_tools.h"
#include "random_tools.h"
#include "lalloc_abstraction.h"

extern int isr_dis;

typedef struct
{
    char *title;                   // test description
    uint32_t simulations_count;    //# of simulations
    LALLOC_IDX_TYPE blocksize_min; // min size for each element to be written
    LALLOC_IDX_TYPE blocksize_max; // max size for each element to be written
    LALLOC_IDX_TYPE pool_size;     // memory pool size
} tTestParams_In;

/* estructuras */
typedef struct
{
    // uint32_t efficiency_avrg;           //average efficiency
    // uint32_t efficiency_min;            //min efficiency
    uint32_t num_allocs;
    uint32_t num_bytes_written;
    uint32_t num_pool_wrap_arround;
} tTestParams_Out;

/**
   @brief this test does:
          - request a memory block
          - write random data
          - commit
          - request another memory block
          - deleted the 1rst block
          -
          - loop

   @param obj
   @param params
   @param paramsout
 */
void random_test( LALLOC_T *obj, tTestParams_In *params, tTestParams_Out *paramsout )
{
    isr_dis = 0;
    static uint32_t runcount = 0;
    uint32_t i;
    uint8_t toggle;

    LALLOC_IDX_TYPE current_charnum;
    LALLOC_IDX_TYPE given_size;
    LALLOC_IDX_TYPE free_space;

    char *p_mem;
    char *p_mem_last;

    char message[200];

    /* randomizo */
    srand( time( NULL ) );

    paramsout->num_bytes_written = 0;
    paramsout->num_pool_wrap_arround = 0;

    paramsout->num_allocs = 0;

    toggle = 0;

    printf( "Simulations: ........................... %u\n", params->simulations_count );
    printf( "Buffer Size: ........................... %u B\n", params->pool_size );

    /* initializes the object */
    lalloc_init( obj );
    TEST_ASSERT_TRUE( isr_dis == 0 );
    /* request memory space */
    lalloc_alloc( obj, ( void** ) &p_mem, &given_size );
    TEST_ASSERT_TRUE( isr_dis == 0 );
    p_mem_last = p_mem;

    for ( i = 0; i < params->simulations_count; i++ )
    {
        TEST_ASSERT_TRUE( p_mem != NULL && given_size > 0 );

        /* generate a random number of bytes to insert */
        current_charnum = uint32_random_range( params->blocksize_min, params->blocksize_max );

        if ( current_charnum > given_size )
        {
            /* ensures that there will be space always */
            current_charnum = given_size / 2;
        }

        if ( current_charnum > 0 )
        {
            /* random data generation (it uses the charset provided by random_tools.c ) */
            random_byte_array( current_charnum, p_mem );

            sprintf( message, "Bytes given: %d Bytes written %d", given_size, current_charnum );

            lalloc_commit( obj, current_charnum );

            TEST_ASSERT_TRUE( lalloc_sanity_check( obj ) );

            TEST_ASSERT_TRUE( isr_dis == 0 );

            paramsout->num_allocs++;
            paramsout->num_bytes_written += current_charnum;

            /* Requesta a ram area */
            lalloc_alloc( obj, ( void** ) &p_mem, &given_size );
        }

        TEST_ASSERT_TRUE( lalloc_sanity_check( obj ) );

        LALLOC_IDX_TYPE num_allocs = lalloc_get_alloc_count( obj );

        TEST_ASSERT_TRUE( isr_dis == 0 );

        if ( num_allocs )
        {
            /* Dealloc */
            lalloc_free( obj, p_mem_last );

            TEST_ASSERT_TRUE( lalloc_sanity_check( obj ) );
            TEST_ASSERT_TRUE( isr_dis == 0 );
        }

        if ( current_charnum > 0 )
        {
            p_mem_last = p_mem;
        }

        /* test that ensures that the free node is always the biggest */
        // TEST_FREE_SPACE_VALIDATION( obj );
    }

    /* final calculations and report */
    float a;

    a = paramsout->num_bytes_written / 1024;
    printf( "Processed bytes ........................ %0.2f kB\n", a );
    a = ( float )paramsout->num_bytes_written / paramsout->num_allocs;
    printf( "Mean element size ...................... %3.2f B\n", a );
    printf( "Buffer wrap arround count .............. %u\n", paramsout->num_pool_wrap_arround );
    // printf ( "Buffer Eff (min): ...................... %u\%\n", paramsout->efficiency_min );
    // printf ( "Buffer Eff (prom): ..................... %u\%\n", paramsout->efficiency_avrg );

    runcount++;
}

/* WHITE BOX TEST
 * TEST THE ALLOCATION COMMIT AND FREE OPERATIONS, AND VALIDATES THE INTEGRITY OF THE LIST AFTER THEM.
 * IT VALIDATES THE COALESCENSE ALGORITM FOR THE FREED NODES.
 * */
void test_random_1()
{
    int i;
    uint8_t *data;
    LALLOC_IDX_TYPE size;

    tTestParams_In params_in;
    tTestParams_Out paramsout;

    uint32_t pool_size = uint32_random_range( 50, 300 );

    params_in.pool_size = pool_size;
    params_in.simulations_count = 50000;
    params_in.blocksize_min = 10;
    params_in.blocksize_max = pool_size;
    params_in.title = ( char * )__FUNCTION__;

    LALLOC_DECLARE( test_alloc, pool_size, 0 );

    random_test( &test_alloc, &params_in, &paramsout );
}

void test_random_2()
{
    int i;
    uint8_t *data;
    LALLOC_IDX_TYPE size;

    tTestParams_In params_in;
    tTestParams_Out paramsout;

    uint32_t pool_size = uint32_random_range( 50, 300 );

    params_in.pool_size = pool_size;
    params_in.simulations_count = 50000;
    params_in.blocksize_min = 1;
    params_in.blocksize_max = pool_size / 2;
    params_in.title = ( char * )__FUNCTION__;

    LALLOC_DECLARE( test_alloc, pool_size, 0 );

    random_test( &test_alloc, &params_in, &paramsout );
}

void test_random_3()
{
    int i;
    uint8_t *data;
    LALLOC_IDX_TYPE size;

    tTestParams_In params_in;
    tTestParams_Out paramsout;

    // uint32_t pool_size = uint32_random_range( 50, 300 );

    params_in.pool_size = 151;
    params_in.simulations_count = 50000;
    params_in.blocksize_min = 1;
    params_in.blocksize_max = 3;
    params_in.title = ( char * )__FUNCTION__;

    LALLOC_DECLARE( test_alloc, params_in.pool_size, 0 );

    random_test( &test_alloc, &params_in, &paramsout );
}
