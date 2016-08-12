DEBUG   = -ggdb -DDEBUG
#DEBUG   = -ggdb


all :
	g++ $(DEBUG) -Wall -I/usr/lib/mysql -o ./bin/buyu-golds-sync-mysql ./src/*.c ./src/*.cpp -lrt -lmysqlclient
clean :
	rm -f ./*.o

