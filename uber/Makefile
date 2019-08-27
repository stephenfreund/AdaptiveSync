
-include ../Makefile.common

CFLAGS += -fPIC
CXXFLAGS += -fPIC

# define any directories containing header files other than /usr/include
#
INCLUDES += -I./include

# define the C source files
SRCS =  ./src/patch.c \
	./src/util.c \
	./src/ppoint.c \
	./src/uberlock.c \
	./src/probe.c \
	./src/knob.c \
	./src/uknob.cpp \

#	./src/ubergroup.c \

OBJS = $(addsuffix .o,$(basename $(SRCS)))

.PHONY: depend clean

MAIN = lib/libuber.so

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
