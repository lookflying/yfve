test: test.cpp ../message/message.h ../message/message.cpp ../YzJni.cpp ../YzJni.h ../YzHelper.h ../YzHelper.cpp
	g++ -o test -Wall test.cpp ../message/message.cpp ../YzJni.cpp ../YzHelper.cpp -lgtest -lpthread 
debug: test.cpp ../message/message.h ../message/message.cpp ../YzJni.cpp ../YzJni.h ../YzHelper.h ../YzHelper.cpp
	g++ -g -o test -Wall test.cpp ../message/message.cpp ../YzJni.cpp ../YzHelper.cpp -lgtest -lpthread -I"/usr/lib/jvm/default-java/include/" -I"/usr/lib/jvm/default-java/include/linux" -lc

