OBJECT=lib/libbuyudiamond.so
CXX=g++
CFLAGS=-ggdb -shared -fPIC -Wall -Wattributes -DDEBUG -lmysqlclient -lssl -lrt

SOURCE=seer_bean.cpp seer_bean_proto.h db_connection_mgr.h db_connection_mgr.cpp ini_file.h ini_file.c mysql_res_auto_ptr.h mysql_res_auto_ptr.cpp my_exception.h my_exception.cpp  msglog.c seer_bean_stat.h seer_bean_stat.cpp mq_mgr.h mq_mgr.cpp util.h util.cpp

.PHONE : all

all : $(OBJECT)

$(OBJECT) : $(SOURCE)
	$(CXX) $(CFLAGS) $(SOURCE) -o $(OBJECT)

.PHONE : clean

clean :
	rm -f $(OBJECT)

