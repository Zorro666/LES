LES_FUNCTION:= les_function.cpp les_function_macros.cpp les_type.cpp les_parameter.cpp les_struct.cpp
LES_TEST:= les_test.cpp les_test_macros.cpp
LES_DEFINITIONALFILE:= les_definitionfile.cpp les_chunkfile.cpp \
											 les_stringtable.cpp les_typedata.cpp \
											 les_structdata.cpp les_funcdata.cpp
LES_CORE:= les_core.cpp les_hash.cpp les_logger.cpp les_loggerchannel.cpp
LES_DEPENDS:=$(LES_CORE) $(LES_FUNCTION) $(LES_TEST) $(LES_DEFINITIONALFILE) les_jake.cpp
LES_TEST_DEPENDS:=les_main.cpp $(LES_DEPENDS) 

PROJECTS:=\
		  les_test\

all: $(PROJECTS)

C_COMPILE:=gcc -c
C_COMPILE_FLAGS:=-g -Wall -Wextra -Wuninitialized -Winit-self -Wstrict-aliasing -Wfloat-equal -Wshadow -Wconversion -Werror -ansi -pedantic-errors -fpack-struct=4

CPP_COMPILE:=g++ -c
CPP_COMPILE_FLAGS:=-g -Wall -Wextra -Wuninitialized -Winit-self -Wstrict-aliasing -Wfloat-equal -Wshadow -Wconversion -Werror -fpack-struct=4

LINK:=g++
LINK_FLAGS:=-g -lm

ifdef WINDIR
TARGET_EXTENSION := .exe
else
TARGET_EXTENSION := 
endif	# ifdef WINDIR

define upperString
$(shell echo $1 | tr [a-z] [A-Z] )
endef

define PROJECT_template
$2_SRCFILES += $1.cpp
$2_SRCFILES += $($2_DEPENDS)
$2_DFILES:=$$($2_SRCFILES:.cpp=.d)

$2_OBJFILE:=$1.o
$2_OBJFILES:=$$($2_SRCFILES:.cpp=.o)

SRCFILES += $$($2_SRCFILES)
OBJFILES += $$($2_OBJFILES)
DFILES += $$($2_DFILES)

TARGETS += $1

$1: $$($2_OBJFILES) 
endef
     
$(foreach project,$(PROJECTS),$(eval $(call PROJECT_template,$(project),$(call upperString,$(project)))))

TARGET_EXES := $(foreach target,$(TARGETS),$(target)$(TARGET_EXTENSION))

test:
	@echo C_COMPILE=$(C_COMPILE)
	@echo C_COMPILE_FLAGS=$(C_COMPILE_FLAGS)
	@echo CPP_COMPILE=$(CPP_COMPILE)
	@echo CPP_COMPILE_FLAGS=$(CPP_COMPILE_FLAGS)
	@echo LINK=$(LINK)
	@echo LINK_FLAGS=$(LINK_FLAGS)
	@echo PROJECTS=$(PROJECTS)
	@echo TARGETS=$(TARGETS)
	@echo SRCFILES=$(SRCFILES)
	@echo OBJFILES=$(OBJFILES)
	@echo DFILES=$(DFILES)
	@echo LES_TEST_SRCFILES=$(LES_TEST_SRCFILES)
	@echo LES_TEST_OBJFILES=$(LES_TEST_OBJFILES)
	@echo LES_TEST_DEPENDS=$(LES_TEST_DEPENDS)
	@echo LES_TEST_DFILES=$(LES_TEST_DFILES)
	@echo LES_TEST_OBJFILE=$(LES_TEST_OBJFILE)
	@echo TARGET_EXTENSION=$(TARGET_EXTENSION)
	@echo TARGET_EXES=$(TARGET_EXES)

%.o: %.c
	@echo Compiling $<
	@$(C_COMPILE) -MMD $(C_COMPILE_FLAGS) -o $*.o $<

%.o: %.cpp
	@echo Compiling $<
	@$(CPP_COMPILE) -MMD $(CPP_COMPILE_FLAGS) -o $*.o $<

%: %.o 
	@echo Linking $@
	@$(LINK) $(LINK_FLAGS) -o $@ $^

.PHONY: all clean nuke format tags
.SUFFIXES:            # Delete the default suffixes

FORCE:

tags:
	@ctags -R --exclude=makefile --c++-kinds=+p --fields=+iaS --extra=+q .

update_tests:
	@cp -vf log.txt correctTestOutput.txt

testit: all
	les_test -tests

runtests: testit

clean: FORCE
	@$(RM) -vf $(OBJFILES)
	@$(RM) -vf $(DFILES)
	@$(RM) -vf tags
	@$(RM) -vf cscope.out
	@$(RM) -vf cscope.in.out
	@$(RM) -vf cscope.po.out


nuke: clean
	@$(RM) -vf $(TARGET_EXES)


sinclude $(DFILES)

