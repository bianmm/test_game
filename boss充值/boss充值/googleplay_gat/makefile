CC = g++
WORKDIR = .
SRCDIR = $(WORKDIR)/src
LIBDIR = $(WORKDIR)/lib
OBJDIR = $(WORKDIR)/obj
INCLUDEDIR = $(WORKDIR)/include
BINDIR = $(WORKDIR)/bin
TARGET = $(BINDIR)/libgoogleplay.so
CFLAGS = -I$(INCLUDEDIR) -L$(LIBDIR) -lsafemysql -lmysqlclient -lcrypto -fPIC -ggdb -Wall -liprestrictor -DDEBUG -lssl -lcurl -ljson
SRCS = $(wildcard $(SRCDIR)/*.cpp)
HEADERS = $(wildcard $(INCLUDEDIR)/*.h)
OBJS := $(patsubst %.cpp,%.o,$(subst $(SRCDIR),$(OBJDIR),$(SRCS)))

$(TARGET): $(OBJS) makefile
	$(CC) -shared -o $(TARGET) $(OBJS) $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INCLUDEDIR)/%.h
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	-rm $(OBJS) $(TARGET)
