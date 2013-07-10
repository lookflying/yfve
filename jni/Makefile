CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS = YfveDemo.o log4z.o Connection.o

LIBS = -lev -lpthread

TARGET =	YfveDemo

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
