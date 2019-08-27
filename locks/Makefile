
-include ../Makefile.common

CFLAGS += -fPIC
CFLAGS += -mrtm

# define any directories containing header files other than /usr/include
#
INCLUDES += -I./include

# define the C source files
SRCS =  ./src/spin.c \
	./src/spin-rtm.c 


OBJS = $(addsuffix .o,$(basename $(SRCS)))

.PHONY: depend clean

MAIN = lib/liblocks.so

all:    $(MAIN) $(MAIN225)
	@echo  Woot!

$(MAIN): $(OBJS)
	 mkdir -p lib
	 $(CXX) -shared -o $(MAIN) $(OBJS)

clean:
	$(RM) $(OBJS) Makefile.deps *~ $(MAIN)

-include Makefile.deps

Makefile.deps: $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MM $(SRCS) > Makefile.deps
