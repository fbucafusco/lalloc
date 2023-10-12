#pragma once
#include "lalloc.h"


#ifdef __cplusplus
extern "C" {
#endif
LALLOC_IDX_TYPE lalloc_print_graph(LALLOC_T *obj, char last,  uint32_t scale);

LALLOC_IDX_TYPE lalloc_sanity_check( LALLOC_T * obj ) ;

void lalloc_measure_framgentation( LALLOC_T * obj, float* rmsbased, float* average_based ) ;

void _block_set_data( uint8_t *pool, LALLOC_IDX_TYPE block_idx, uint8_t *addr, LALLOC_IDX_TYPE size );

#ifdef __cplusplus
}
#endif
