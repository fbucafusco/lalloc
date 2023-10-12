#ifndef RANDOM_STRING_H
#define RANDOM_STRING_H

#include <stdint.h>

void random_char_array ( uint16_t  length, char *buffer );
void random_byte_array ( uint16_t length, char *buffer );
uint32_t uint32_random_range ( uint32_t min, uint32_t max );

#endif