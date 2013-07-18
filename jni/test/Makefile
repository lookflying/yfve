test: test.cpp \
								../message/message.h \
								../message/message.cpp 
	g++ -o test -Wall test.cpp ../message/message.cpp -lgtest -lpthread 
debug: test.cpp ../message/message.h ../message/message.cpp 
	g++ -g -o test -Wall test.cpp ../message/message.cpp -lgtest -lpthread 
clean:
	rm test
