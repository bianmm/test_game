OBJECT=lib/libmbaccount.so
CXX=g++
CFLAGS=-ggdb -shared -fPIC -Wall -DDEBUG -lmysqlclient -lssl

SOURCE=benchapi.h mb_account.cpp mb_proto.h db_connection_mgr.h db_connection_mgr.cpp ini_file.h ini_file.c mysql_res_auto_ptr.h mysql_res_auto_ptr.cpp my_exception.h my_exception.cpp mb_statistic.h mb_statistic.cpp util.cpp msglog.c cfg_file_parse.c proto.xml.cpp

.PHONE : all

all : $(OBJECT)

$(OBJECT) : $(SOURCE)
	$(CXX) $(CFLAGS) $(SOURCE) -o $(OBJECT) 

.PHONE : clean

clean : 
	rm -f $(OBJECT)

