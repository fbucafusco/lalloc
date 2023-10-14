#ifndef LALLOC_CONFIG_H
#define LALLOC_CONFIG_H

#include "unity.h"
#include "lalloc_abstraction.h"

#ifndef LALLOC_ALIGNMENT
#define LALLOC_ALIGNMENT               1
#endif

#ifndef LALLOC_MAX_BYTES
#define LALLOC_MAX_BYTES               0xFFFF // 0xFFFFFFFF
#endif

/* ====================================================================================
    LALLOC_USE_OS
    OPTIONS FOR LALLOC_USE_OS:
    1: THE LIBRARY WILL BE USED IN A MULTITASKING OS.
    0: THE LIBRARY WILL BE USED IN A COOPERATIVE SYSTEM OR IN WITHIN JUST ONLY ON TASK IN AN OS.
   ==================================================================================== */
#define LALLOC_USE_OS                   1

/* ===============================================================================================================================================
   ASSERT
   =============================================================================================================================================== */
#define LIB_DEBUG_LEVEL 1

#if LIB_DEBUG_LEVEL==-1
#include <assert.h>
#define LALLOC_ASSERT(CONDITION)
#endif

#if LIB_DEBUG_LEVEL==0
#include <assert.h>
#define LALLOC_ASSERT(CONDITION)    assert(CONDITION)
#endif

#if LIB_DEBUG_LEVEL==1
#define LALLOC_ASSERT(CONDITION)   test_assert(CONDITION, __FUNCTION__ , __LINE__ )
#endif

#if LIB_DEBUG_LEVEL==2
#include <assert.h>
#define LALLOC_ASSERT(CONDITION)    if(!(CONDITION)) printf("error EN %s en linea %u", __FUNCTION__ , __LINE__   ); assert(CONDITION);
#endif

#define LALLOC_CRITICAL_START test_crtical_start(__FUNCTION__ , __LINE__)
#define LALLOC_CRITICAL_END   test_crtical_end()

/* LALLOC_VALIDATE_INDEX
it defines a run time validation for the index passed to methods.
*/

/* in order to enable test code */
#define LALLOC_TEST

#define LALLOC_ALLOW_QUEUED_FREES 0

#endif //LALLOC_UART_CONFIG_H
