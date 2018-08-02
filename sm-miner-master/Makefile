# sm-miner-master (v2) Makefile
#
# Make software in debug mode (default):
#   make debug
# or just:
#   make
#
# Make software in release mode:
#   make release
#
# Make software with gprof profiling support:
#   make profile
#
# Make both debug and release versions of the software:
#   make all
# (same as "make debug ; make release")
#
# Clean out built project files:
#   make clean
#
# To rebuild project run "make clean" then "make [mode]".
#

# Target file name (without extension).
TARGET := sm-miner-master

BASE	= .

# Source file directory.
SRCDIR := $(BASE)/..
SRCDIR_1 := $(SRCDIR)/sm-miner-master/src
SRCDIR_2 := $(SRCDIR)/sm-miner-cmn

# Build output directory.
BUILDDIR := $(BASE)/build

# Specify the default make goal.
.DEFAULT_GOAL := debug


# Utility variables.
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)

# Build mode (debug, release, etc).
MODE := none
ifeq ($(MAKECMDGOALS),$(EMPTY))
MODE := $(.DEFAULT_GOAL)
else
MODE := $(word 1, $(MAKECMDGOALS))
endif

# Define commands
ifndef MSYS_DIR
    MSYS_DIR := /c/Program/msys
endif

ifeq ($(OS),Windows_NT)
	CC := arm-linux-gnueabihf-g++
	GCC := arm-linux-gnueabihf-gcc
	FIND := $(MSYS_DIR)/bin/find
else
	CC := arm-linux-gnueabihf-g++
	GCC := arm-linux-gnueabihf-gcc
	FIND := find
endif

# Mode-specific build output directory.
OUTDIR := $(BUILDDIR)/$(MODE)

# Mode-specific output directory for generated object files.
OBJDIR := $(OUTDIR)/obj

# Full name of the target executable.
EXECUTABLE := $(OUTDIR)/$(TARGET)

# Create a full list of the source directories starting from the SRCDIR root.
SRCDIRS := $(shell $(FIND) $(SRCDIR_1) -type d 2>/dev/null)
SRCDIRS += $(shell $(FIND) $(SRCDIR_2) -type d 2>/dev/null)

# A list of C++/C source files.
CPPSRC := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.cpp))
CSRC := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

# A list of the corresponded object files.
CPPOBJ := $(CPPSRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
COBJ := $(CSRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Compiler flag to set the C++ Standard level.
CSTD := -std=c++0x

# C/C++ compiler flags.
#   -g:        generate debugging information.
#   -Wall:     enables all the warnings.
#   -Wa,...:   tell GCC to pass this to the assembler.
#   -Wno-psabi disable va_list mangling change related warning

CFLAGS = -g
CFLAGS += -Wall
CFLAGS += -Wa,-adhlns=$(subst $(suffix $@),.lst,$@)
CFLAGS += -Wno-psabi
ifeq ($(MACH_TYPE), a1)
	CFLAGS += -DMACH_A1
	CFLAGS += -DPOWER_SUPPLY_USE_HUAWEI_R48XX
	CFLAGS += -DPOWER_SUPPLY_SUPPORT_MULTI_MODULE
else ifeq ($(MACH_TYPE), a3)
	CFLAGS += -DMACH_A3
endif

ifeq ($(MULTI_POOL), y)
	CFLAGS += -DMULTI_POOL_SUPPORT
endif
NO_WARN = -Wno-sign-compare -Wno-narrowing
NO_WARN_C = $(NO_WARN)
NO_WARN_CPP = $(NO_WARN) -Wno-reorder

INCDIRS := -I$(SRCDIR_1)
INCDIRS += -I$(SRCDIR_1)/slave-gate
INCDIRS += -I$(SRCDIR_1)/jsoncpp
INCDIRS += -I$(SRCDIR_2)
INCDIRS += -I$(SRCDIR_2)/ms-protocol

# Linker flags.
#   -g:        generate debugging information.
#   -Wl,...:   tell GCC to pass this to linker.
LDFLAGS = -g
LDFLAGS += -Wl,-Map=$(OUTDIR)/$(TARGET).map

# A list of libraries to link with.
LDLIBS = -lpthread -lrt

# Compiler flags to generate dependency files.
GENDEP = -MMD -MP -MT $@ -MF $(OBJDIR)/$(*D)/$(*F).d


# Disable built-in rules.
.SUFFIXES :

# Target: build in each mode.
all :
	@$(MAKE) --no-print-directory debug
	@echo
	@$(MAKE) --no-print-directory release

# Target: debug build.
debug : build

# Target: release build.
release : CFLAGS += -O3
release : CFLAGS += -DNDEBUG
release : build

# Target: build with gprof profiling support.
profile : CFLAGS += -pg
profile : LDFLAGS += -pg
profile : build


# Build project.
build : begin executable end

# Eye candy.
begin :
	@echo "--------- Building $(TARGET) in $(MODE) mode... ---------"
	@mkdir -p $(OUTDIR)
	@echo "Source directory:" $(CURDIR)/$(SRCDIR)
	@echo "Build directory:" $(CURDIR)/$(OUTDIR)
	@echo

end :
	@echo; echo "Build finished."


# Target to build executable.
executable : $(EXECUTABLE)

# Link: create executable file from object files.
.PRECIOUS : $(COBJ) $(CPPOBJ)
$(EXECUTABLE) : $(COBJ) $(CPPOBJ)
	@echo; echo "Linking:" $@
	$(CC) $(LDFLAGS) $(COBJ) $(CPPOBJ) $(LDLIBS) -o $@

# Compile: create object files from C++ source files.
$(CPPOBJ) : $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@echo; echo "Compiling:" $<
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(CC) -c $(CFLAGS) $(NO_WARN_CPP) $(CSTD) $(INCDIRS) $(GENDEP) $< -o $@

# Compile: create object files from C++ source files.
$(COBJ) : $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@echo; echo "Compiling:" $<
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(GCC) -c $(CFLAGS) $(NO_WARN_C) $(INCDIRS) $(GENDEP) $< -o $@
	
# Target: clean project.
clean :
	@echo "Cleaning $(TARGET)..."
	@rm -rf $(BUILDDIR)
	@echo "Clean up finished."


# Include generated dependency files from previous build (if any).
-include $(shell find $(OBJDIR) -type f -name *.d 2>/dev/null)

# Specify a list of phony targets.
.PHONY : all debug release clean build begin end executable
