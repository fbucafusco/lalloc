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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?![]{}<>123456790"; // could be const

#define CHARSET_SIZE    (sizeof(charset)-1);

/**
   @brief   Creates a random array of printable ASCII caracters of lenght "len"
            The pool should have allocated at least len+1 bytes.
            It ends as a NULL TERMINATED string.
   @param len
   @param buffer
 */
void random_char_array ( uint16_t len, char *buffer )
{
    if ( len>0 )
    {
        int n;
        len--; //for the '\0'

        for ( n = 0; n < len; n++ )
        {
            uint16_t key  = rand() % CHARSET_SIZE;
            buffer[n] = charset[key];
        }

        buffer[len] = '\0';
    }
}


/**
   @brief   Creates a random array of bytes of lenght "len"
            The pool should have allocated at least len bytes

   @param len
   @param buffer
 */
void random_byte_array ( uint16_t len, char *buffer )
{
    if ( len>0 )
    {
        int n;

        for ( n = 0; n < len; n++ )
        {
            uint16_t key    = rand() % 0xFF;
            uint8_t key_    = key;
            buffer[n]       = key_;
        }
    }
}


/*
if generates a 32 bit random number bounded to min and max.
*/
uint32_t uint32_random_range( uint32_t min, uint32_t max )
{
    if ( max == min )
    {
        return min;
    }

    int rand_ = rand();
    int rand_max = RAND_MAX;
    float slope = ( ( float )max - min ) / ( ( float )RAND_MAX );
    uint64_t rand_num = ( rand_ * slope );
    return rand_num + min;
}