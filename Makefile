ROOTCONFIG   := root-config

SrcSuf        = cxx
OutPutOpt     = -o # keep whitespace after "-o"

CXX           = g++
#CXXFLAGS      = -O -Wall -fPIC -g
CXXFLAGS      = -O -Wall -fPIC

ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --libs)

CXXFLAGS     += $(ROOTCFLAGS)
LIBS          = $(ROOTLIBS) $(SYSLIBS)

SRCDIR = src
INCDIR = inc

CXXFLAGS += -I$(INCDIR)

vpath %.$(SrcSuf) $(SRCDIR)

DEPS = $(INCDIR)/physics.h $(INCDIR)/evgen_fns.h

.SUFFIXES: .$(SrcSuf)

# programs
PROGRAMS = EvGen EvGenRes EvGenBasic

all:	$(PROGRAMS)

.$(SrcSuf):	$(DEPS) Makefile
		$(CXX) $(CXXFLAGS) $(LIBS) $^ $(OutPutOpt)$@
		@echo "$@ done"

clean:
		@echo "cleaning up..."
		@rm -f $(PROGRAMS) core*

cleanall:
		@echo "cleaning up..."
		@rm -f $(PROGRAMS) core* out/*.root out/*/*.root
