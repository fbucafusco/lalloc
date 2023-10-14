#NOMBRE DEL PROYECTO
PROJECT_NAME = test_lalloc

#COMMON INCLUDE FOR TESTS
INC_FILES += $(TESTS_BASE_PATH)unity_src
INC_FILES += $(TESTS_BASE_PATH)support
INC_FILES += $(TESTS_BASE_PATH)
INC_FILES += $(LIBS_PATH)inc

#COMMON SOURCES FOR TESTS
SRC_FILES += $(TESTS_BASE_PATH)support/lalloc_abstraction.c
SRC_FILES += $(TESTS_BASE_PATH)support/malloc_replace.c
SRC_FILES += $(TESTS_BASE_PATH)unity_src/unity.c
SRC_FILES += $(LIBS_PATH)src/lalloc.c
# SRC_FILES += $(MAKEFILE_DIR)src/helpers.c

#COMON FLAGS FOR COMPILER&LINKER
CFLAGS += -D_x86_TESTS -std=gnu99
LFLAGS += -pthread -lm -ldl

# TESTS = test3 
TESTS = test1 test2 test3 test4 test5 test6

#TEST1
SRC_FILES_T1 	+= $(TESTS_BASE_PATH)test_basic.c
INC_FILES_T1 	=
CFLAGS_T1		=

#TEST2
SRC_FILES_T2 	+= $(TESTS_BASE_PATH)test_list.c
SRC_FILES_T2 	+= $(TESTS_BASE_PATH)support/lalloc_tools.c
INC_FILES_T2 	=
CFLAGS_T2		=

#TEST3 			default LALLOC_ALIGNMENT & LALLOC_MAX_BYTES 
SRC_FILES_T3 	+= $(TESTS_BASE_PATH)test_random.c
SRC_FILES_T3 	+= $(TESTS_BASE_PATH)support/lalloc_tools.c
SRC_FILES_T3 	+= $(TESTS_BASE_PATH)support/random_tools.c
INC_FILES_T3 	=
CFLAGS_T3		=

#TEST4          TEST3 without defaults 
SRC_FILES_T4 	+= $(TESTS_BASE_PATH)test_random.c
SRC_FILES_T4 	+= $(TESTS_BASE_PATH)support/lalloc_tools.c
SRC_FILES_T4 	+= $(TESTS_BASE_PATH)support/random_tools.c
INC_FILES_T4 	=
CFLAGS_T4		=  -DLALLOC_ALIGNMENT=4 -DLALLOC_MAX_BYTES=0xFFFFFFFF

#TEST5			default LALLOC_ALIGNMENT & LALLOC_MAX_BYTES
SRC_FILES_T5 	+= $(TESTS_BASE_PATH)test_random_2.c
SRC_FILES_T5 	+= $(TESTS_BASE_PATH)support/lalloc_tools.c
SRC_FILES_T5 	+= $(TESTS_BASE_PATH)support/random_tools.c
INC_FILES_T5 	=
CFLAGS_T5		=

#TEST6			TEST5 without defaults
SRC_FILES_T6 	+= $(TESTS_BASE_PATH)test_random_2.c
SRC_FILES_T6 	+= $(TESTS_BASE_PATH)support/lalloc_tools.c
SRC_FILES_T6 	+= $(TESTS_BASE_PATH)support/random_tools.c
INC_FILES_T6 	=
CFLAGS_T6		=  -DLALLOC_ALIGNMENT=4 -DLALLOC_MAX_BYTES=0xFFFFFFFF
 