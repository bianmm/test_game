all:
	g++ create_mq2.cpp -lrt -o create_mq2
	g++ get_mq_msg_count.cpp -lrt -o get_mq_msg_count
	g++ send_msg.cpp -lrt -o send_msg
	g++ send_msg_sync_test.cpp -lrt -o send_msg_sync_test
	g++ get_mq_attr.cpp -lrt -o get_mq_attr
	g++ mq_unlink.cpp -lrt -o mq_unlink
	g++ send_msg_seer_bean.cpp -lrt -o send_msg_seer_bean

clean:
	rm -f create_mq2 get_mq_msg_count send_msg send_msg_sync_test get_mq_attr mq_unlink send_msg_seer_bean
