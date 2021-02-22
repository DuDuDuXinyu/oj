INCLUDE_PATH=-I /home/duxinyu/third_part/include -I /usr/include/jsoncpp

LIB_PATH=-L /home/duxinyu/third_part/lib -lctemplate -ljsoncpp

BIN=../bin/oj_server

$(BIN):oj_server.cpp
	g++ $^ -o $@ -g -lpthread $(INCLUDE_PATH) $(LIB_PATH)

.PHONY:clean
clean:
	rm $(BIN)
