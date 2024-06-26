#MAKEFILE PATH
MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#LIBS PATH
LIBS_PATH = $(abspath $(MAKEFILE_DIR)../)/

#TESTS PATH
TESTS_BASE_PATH = $(LIBS_PATH)test/

#OUTPUT FILE PATHS
OUT_PATH = $(MAKEFILE_DIR)out
BIN_PATH = $(MAKEFILE_DIR)bin
COVERAGE_PATH = $(MAKEFILE_DIR)coverage

#includes all the project definitions
include project.mk

#compiler flags
CFLAGS += $(foreach inc, $(INC_FILES), -I$(inc) )
CFLAGS += -D_WIN32 -ggdb -Wall -fprofile-arcs -ftest-coverage

#linke flags
LFLAGS +=  -lgcov

INDEX = ""
 
#:PHONY: all  
all: info $(foreach test, $(TESTS), $(test) )

#rule that compiles each .c into a .o
.PHONY:  option%
option%:
	@echo '----------------------------------------------------------- OPTION - '	
	$(eval INDEX = $*)
	$(eval CFLAGS_EXTRA = $(CFLAGS_T$(@:option%=%)) )
	$(eval SRC_FILES_T = $(SRC_FILES_T$(@:option%=%)) )
	@echo "Flags extras: "$(CFLAGS_EXTRA)
	@echo " "
	@echo '---------------------------------------------------------------------'
	@echo "Creando "$(OUT_PATH)$(INDEX)
	@mkdir -p $(OUT_PATH)$(INDEX)
	@echo " "

.PHONY: test%
test%: CFLAGS_EXTRA=$(CFLAGS_T$(@:test%=%))
test%: INDEX=$(@:test%=%)
test%: SRC_FILES_T=$(SRC_FILES_T$(@:test%=%))
test%:
	@echo ""  	
	@echo "------------------------------------------------------------- TEST "$(INDEX)
	@echo  $(CFLAGS_EXTRA)	 
	@echo $(SRC_FILES_T)
	@echo '---------------------------------------------------------------------'
	@mkdir -p $(BIN_PATH)
	@echo "Creating "$(OUT_PATH)$(INDEX)
	@mkdir -p $(OUT_PATH)$(INDEX)	
	@echo '---------------------------------------------------------------------'
	@echo "Compiling test files"
	$(foreach src_file_t, $(SRC_FILES_T), gcc -c $(src_file_t) -Wall $(CFLAGS) $(CFLAGS_EXTRA) -o $(OUT_PATH)$(INDEX)/$(notdir $(patsubst %.c,%.o, $(src_file_t))); )  
	@echo '---------------------------------------------------------------------'
	@echo "Compiling common test files"
	$(foreach src_file, $(SRC_FILES), gcc -c $(src_file) -Wall $(CFLAGS) $(CFLAGS_EXTRA) -o $(OUT_PATH)$(INDEX)/$(notdir $(patsubst %.c,%.o, $(src_file))); )  
	@echo '---------------------------------------------------------------------'
	@echo "Linking "$<	
	gcc $(OUT_PATH)$(INDEX)/*.o $(LFLAGS) -o $(BIN_PATH)/$(PROJECT_NAME)_$(INDEX)

#rule that prints information
.PHONY: info
info:
	@echo '---------------------------------------------------------------------'
	@echo 'Libs Base Path:       '$(LIBS_PATH)
	@echo 'Tests base Path:      '$(TESTS_BASE_PATH)	
	@echo '---------------------------------------------------------------------'	

clean%: INDEX=$(@:clean%=%)
clean%:
	@echo "Cleaning Test "$(INDEX)
	@rm -rf $(OUT_PATH)$(INDEX)*.o  
	@rm -rf $(OUT_PATH)$(INDEX)*.gcda
	@rm -rf $(OUT_PATH)$(INDEX)*.gcno
	@rm -rf $(BIN_PATH)
	@rm -rf $(COVERAGE_PATH)
	@rm -rf $(PROJECT_NAME)_$(INDEX) #$(OUT_PATH)$(INDEX)

clean: $(foreach test_i, $(TESTS),  $(subst test,clean, $(test_i))    )
	@echo 
	@echo "done"

run%: INDEX=$(@:run%=%)
run%:
	$(BIN_PATH)/$(PROJECT_NAME)_$(INDEX)

run: $(foreach test_i, $(TESTS),  $(subst test,run, $(test_i)) )
	@echo "done"

.PHONY: coverage
coverage:
	@echo "Generating coverage report"
	@mkdir -p ./coverage
	@lcov --capture --initial --directory . --output-file ./coverage/base.info
	@$(foreach outdir,$(wildcard ./out*),lcov --capture --directory $(outdir) --output-file $(outdir)/coverage.info &&) true
	@lcov $(foreach outdir,$(wildcard ./out*),--add-tracefile $(outdir)/coverage.info ) --output-file ./coverage/total_coverage.info
	@genhtml ./coverage/total_coverage.info --output-directory ./coverage/html
	@echo "Coverage reports generated in "./coverage/html

