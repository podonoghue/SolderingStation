# Shared make definitions
#.SILENT :

MAJOR_VERSION := 1
MINOR_VERSION := 0
MICRO_VERSION := 0

PKG_VERSION := $(MAJOR_VERSION).$(MINOR_VERSION)
PKG_NAME = digitallabboard

# Used as prefix with the above when in build directory $(DUMMY_CHILD)/$(SHARED_SRC) = PackageFiles/src
DUMMY_CHILD    := PackageFiles

#ifeq ('$(OS)','')
#   OS=Windows_NT
#endif

BITNESS ?= 64

ifeq ($(OS),Windows_NT)
   UNAME_S := Windows
   UNAME_M := x86_64
   MULTIARCH := x86_64-win-gnu
#   export TMP  ?= C:\Users\PETERO~1\AppData\Local\Temp
#   export TEMP ?= C:\Users\PETERO~1\AppData\Local\Temp
else
   UNAME_S   := $(shell uname -s)
   UNAME_M   := $(shell uname -m)
   MULTIARCH := $(shell gcc --print-multiarch)
endif

#===========================================================
# Shared directories - Relative to child directory
SHARED_SRC     := ../Shared/src
SHARED_LIBDIR := ../Shared/$(MULTIARCH)

#===========================================================
# Where to find private libraries on linux
PKG_LIBDIR="/usr/lib/$(MULTIARCH)/${PKG_NAME}"

#===========================================================
# Where to build
# These may be forced on the command line
ifeq ($(UNAME_S),Windows)
   DIRS = $(COMMON_DIRS) $(WIN_DIRS)
   TARGET_BINDIR   ?= ../PackageFiles/bin/$(MULTIARCH)
   TARGET_LIBDIR   ?= ../PackageFiles/bin/$(MULTIARCH)
   BUILDDIR_SUFFIX ?= .$(MULTIARCH)
   VSUFFIX         ?= .$(MAJOR_VERSION)
else
   # Assume Linux
   DIRS = $(COMMON_DIRS)
   TARGET_BINDIR   ?= ../PackageFiles/bin/$(MULTIARCH)
   TARGET_LIBDIR   ?= ../PackageFiles/lib/$(MULTIARCH)
   BUILDDIR_SUFFIX ?= .$(MULTIARCH)
   include /usr/share/java/java_defaults.mk
endif

ifeq ($(UNAME_S),Windows)
   .SUFFIXES : .d .rc
   LIB_PREFIX = 
   LIB_SUFFIX = .dll
   EXE_SUFFIX = .exe
   
   RM            := rm -fy
   RMDIR         := rm -R -f
   TOUCH         := touch
   MKDIR         := mkdir -p
   CP            := cp
   MAKE          := make
   AR            := ar
   GCC           := gcc
   GPP           := g++
   WINDRES       := windres   --use-temp-file 
   STRIP         := strip
   STRIPFLAGS    := --strip-unneeded
#	export PATH=/usr/bin:/usr/local/bin:/mingw64/bin:
#	export PATH=:

   #PROGRAM_DIR = C:/"Program Files"
   PROGRAM_DIR = C:/'Program Files (x86)'
else
   .SUFFIXES : .d
   LIB_PREFIX 			:= lib
   LIB_SUFFIX 			:= .so.$(MAJOR_VERSION).$(MINOR_VERSION)
   LIB_MAJOR_SUFFIX 	:= .so.$(MAJOR_VERSION)
   LIB_NO_SUFFIX 		:= .so
   EXE_SUFFIX 			:= 

   RM            := rm -f
   RMDIR         := rm -R -f
   TOUCH         := touch
   MKDIR         := mkdir -p
   CP            := cp
   LN            := ln -s -f
   MAKE          := make
   AR            := ar
   GCC           := gcc
   GPP           := g++
   STRIP         := strip
   STRIPFLAGS    := --strip-unneeded
   WINDRES       := echo
endif

#===========================================================
# Options to build standalone DLL (windows)
ifeq ($(UNAME_S),Windows)
   STATIC_GCC_OPTION := -static -static-libstdc++ -static-libgcc
endif

#===========================================================
#
ifeq ($(UNAME_S),Windows)
   GUI_OPTS := -Wl,--subsystem,windows -mwindows
else
   GUI_OPTS     := 
endif

#===========================================================
# WXWIDGETS
SHARED_WXWIDGETS := "Y"

WXWIDGETS_CPPFLAGS       := $(shell wx-config --cppflags)
WXWIDGETS_DEFS           := -DuseWxWidgets

ifdef SHARED_WXWIDGETS
   WXWIDGETS_LIBS    := $(shell wx-config --libs)
else
   WXWIDGETS_LIBS    := $(shell wx-config --libs --static)       
endif

#===========================================================
#=============================================================
# Common DLLs in debug and non-debug versions as needed

LIB_USB_SHARED  := -lusb-1.0
LIB_USB_STATIC  := -l:libusb-1.0.a

LIB_USB = $(LIB_USB_SHARED)

#===========================================================
# Debug flags
ifeq ($(UNAME_S),Windows)
   GCC_VISIBILITY_DEFS :=
   THREADS := -mthreads
else
   GCC_VISIBILITY_DEFS :=-fvisibility=hidden -fvisibility-inlines-hidden
   THREADS := 
   CFLAGS := -fPIC
endif

ifdef DEBUG
   # Compiler flags
   CFLAGS += -O0 -g3
   # Compiler flags (Linking)
   LDFLAGS = -O0 -g3
   # C Definitions
   DEFS   := -DLOG
else
   # Compiler flags
   CFLAGS += -O3 -g3 
   # Compiler flags (Linking) - include DEBUG info as can be stripped later
   LDFLAGS = -O3 -g3 
endif

ifneq ($(OS),Windows_NT)
   # Executable will look here for libraries
   LDFLAGS += -Wl,-rpath,${PKG_LIBDIR}
   
   # Linker will look here
   LDFLAGS += -Wl,-rpath-link,${SHARED_LIBDIR}
   LDFLAGS += -Wl,-rpath-link,${TARGET_LIBDIR}

   ifeq ($(UNAME_M),x86)
      CFLAGS  += -m32
      LDFLAGS += -m32
   else ifeq ($(UNAME_M),x86_64)
      CFLAGS  += -m64
      LDFLAGS += -m64
   endif
endif

CFLAGS  += -std=gnu++14 ${THREADS} -Wall -shared ${GCC_VISIBILITY_DEFS}
LDFLAGS += ${THREADS}

#===========================================================
# Look in build and shared library directories first
LIBDIRS := -L$(TARGET_LIBDIR) -L$(SHARED_LIBDIR)

PACKAGED_FILES := $(patsubst $(SHARED_LIBDIR)/%, $(TARGET_LIBDIR)/%, $(wildcard $(SHARED_LIBDIR)/*))

$(TARGET_LIBDIR)/%.dll: $(SHARED_LIBDIR)/%.dll
	@echo -- Copying $(@F) to package directory
	$(CP) $< $@

$(TARGET_LIBDIR)/%.jed: $(SHARED_LIBDIR)/%.jed
	@echo -- Copying $(@F) to package directory
	$(CP) $< $@

$(TARGET_LIBDIR)/%.xsvf: $(SHARED_LIBDIR)/%.xsvf
	@echo -- Copying $(@F) to package directory
	$(CP) $< $@

$(TARGET_LIBDIR)/%.map: $(SHARED_LIBDIR)/%.map
	@echo -- Copying $(@F) to package directory
	$(CP) $< $@

$(TARGET_LIBDIR)/%.md: $(SHARED_LIBDIR)/%.md
	@echo -- Copying $(@F) to package directory
	$(CP) $< $@

$(TARGET_LIBDIR)/%.exe: $(SHARED_LIBDIR)/%.exe
	@echo -- Copying $(@F) to package directory
	$(CP) $< $@

$(TARGET_LIBDIR) :
	@echo -- Making directory $(TARGET_LIBDIR)
	-$(MKDIR) $(TARGET_LIBDIR)

ifneq ($(TARGET_LIBDIR),$(TARGET_BINDIR))
$(TARGET_BINDIR) :
	@echo -- Making directory $(TARGET_BINDIR)
	-$(MKDIR) $(TARGET_BINDIR)
endif

$(PACKAGED_FILES) :

commonFiles: $(TARGET_LIBDIR) $(TARGET_BINDIR) $(PACKAGED_FILES) 

.PHONY: commonFiles

