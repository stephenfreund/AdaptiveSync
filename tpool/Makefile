
-include ../Makefile.common

CFLAGS += -fPIC 

# define any directories containing header files other than /usr/include
#
INCLUDES += -I./include -I../uber/include

# define the C source files
SRCS =  ./src/tpool.c \
	./src/queue.c \
	./src/barrier.c \
	./src/uberpool.c \
	./src/uberqueue.c 

OBJS = $(addsuffix .o,$(basename $(SRCS)))

LIBS     += -lgflags -luber -lpthread -lc -llocks

LFLAGS   += -L../../gflags/lib -L../uber/lib/ -L../locks/lib/ 

LDFLAGS  +=  -Wl,-rpath=../uber/lib -Wl,-rpath=../locks/lib 

.PHONY: depend clean

MAIN = lib/libpool.so

all:    $(MAIN) tests
	@echo  Woot!

$(MAIN): $(OBJS)
	mkdir -p lib
	$(CC) -shared  -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

tests:	test.exe ubertest.exe

test.exe: $(MAIN) $(OBJS) test/test.c
	$(CC)  $(LDFLAGS) $(CFLAGS) test/test.c $(INCLUDES) $(OBJS) $(LIBS) $(LFLAGS) -o test.exe 

ubertest.exe: $(MAIN) $(OBJS) test/ubertest.c
	$(CC)  $(LDFLAGS) $(CFLAGS) test/ubertest.c $(INCLUDES) $(OBJS) $(LIBS) $(LFLAGS) -o ubertest.exe

clean:
	$(RM) $(OBJS) Makefile.deps *~ $(MAIN)

-include Makefile.deps

Makefile.deps: $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MM $(SRCS) > Makefile.deps
