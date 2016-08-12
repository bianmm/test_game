CC = g++
WORKDIR = .
SRCDIR = $(WORKDIR)/
HEADERDIR = $(WORKDIR)/
LIBDIR = $(WORKDIR)/lib
OBJDIR = $(WORKDIR)/obj
TARGET = libiprestrictor.a
CFLAGS = -fPIC -shared -ggdb
SRCS := $(wildcard $(SRCDIR)/*.cpp)
STATICS := $(wildcard $(LIBDIR)/*.a)
HEADERS := $(wildcard $(HEADER)/*.h)
OBJS := $(patsubst %.cpp,%.o,$(subst $(SRCDIR),$(OBJDIR),$(SRCS)))

$(TARGET): $(OBJS)
	ar -r $(TARGET) $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERDIR)/*.h
	$(CC) -c $< $(STATICS) -o $@ $(CFLAGS) -I$(HEADERDIR)

.PHONY: clean test run

clean:
	rm $(OBJS) $(TARGET) test_o -rf

test: $(TARGET)
	g++ test/test.cpp -I$(HEADERDIR) -L$(WORKDIR) -liprestrictor -ggdb -o test_o

run:
	./bin/test_o
