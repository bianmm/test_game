CC = g++
WORKDIR = .
SRCDIR = $(WORKDIR)/src
LIBDIR = $(WORKDIR)/lib
OBJDIR = $(WORKDIR)/obj
INCLUDEDIR = $(WORKDIR)/include
BINDIR = $(WORKDIR)/bin
TARGET = $(BINDIR)/libanypayiap.so
CFLAGS = -I$(INCLUDEDIR) -L$(LIBDIR) -ljson -lsafemysql -lmysqlclient -lcrypto -fPIC -ggdb -Wall -liprestrictor -DDEBUG -lssl
SRCS = $(wildcard $(SRCDIR)/*.cpp)
HEADERS = $(wildcard $(INCLUDEDIR)/*.h)
OBJS := $(patsubst %.cpp,%.o,$(subst $(SRCDIR),$(OBJDIR),$(SRCS)))

$(TARGET): $(OBJS) makefile
	$(CC) -shared -o $(TARGET) $(OBJS) $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INCLUDEDIR)/%.h
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	-rm $(OBJS) $(TARGET)
