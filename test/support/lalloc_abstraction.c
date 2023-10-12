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
#include <stdbool.h>
#include "unity.h"

int isr_dis = 0;
void test_assert( bool condition, const char *fcn, int line_ )
{
    if ( condition )
    {
    }
    else
    {
        char buffer[500];
        sprintf( buffer, "Assertion failed in %s, line %d", fcn, line_ );
        TEST_ASSERT_TRUE_MESSAGE( condition, buffer );
    }
}

void test_crtical_start( const char *fcn, int line_ )
{
    static char *lastfcn = NULL;
    static int lastline = 0;

    isr_dis++;

    if ( isr_dis >= 2 )
    {
        char buffer[500];
        sprintf( buffer, "Recursive critical section in %s, line %d", lastfcn, lastline );
    
        TEST_ASSERT_TRUE_MESSAGE( 0, buffer );

#ifndef STM32L475xx
        fflush( stdout );
#endif
    };

    lastline = line_;
    lastfcn = fcn;
}

void test_crtical_end( void )
{
    isr_dis--;
}
