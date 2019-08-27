
-include ../Makefile.common

INCLUDES += -I../../gflags/include -I../uber/include -I../locks/include 

LFLAGS   += -L../../gflags/lib -L../uber/lib/ -L../locks/lib/

LIBS     += -lgflags -luber -lpthread -lc -llocks

SRCS =  ./main.cpp flags.cpp combined.c

OBJS = $(addsuffix .o,$(basename $(SRCS)))

MAIN = exp.exe

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(MAIN) 
	@echo  Woot!

$(MAIN): $(OBJS)
	$(CXX) -Wl,-rpath=../uber/lib -Wl,-rpath=../locks/lib  $(CXXFLAGS) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

clean:
	$(RM) $(OBJS) Makefile.deps *~ $(MAIN)

-include Makefile.deps

Makefile.deps: $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MM $(SRCS) > Makefile.deps
