CXX = g++
CXXFLAGS = -Wall -std=c++17
CLIENTPATH = client/
SERVERPATH = server/



CPP_BSONCXXX_PATH = /usr/local/include/bsoncxx/v_noabi
CPP_MONGOCXX_PATH = /usr/local/include/mongocxx/v_noabi/

CPP_LOCAL_LIB_PATH = /usr/local/lib

${SERVERPATH}lib/%.o: ${SERVERPATH}src/%.cpp
	$(CXX) -c  $< -o $@ ${CXXFLAGS} -I${CPP_MONGOCXX_PATH}  -I${CPP_BSONCXXX_PATH}  -L${CPP_LOCAL_LIB_PATH}  -I${SERVERPATH}include/ 

${CLIENTPATH}lib/%.o: ${CLIENTPATH}src/%.cpp
	$(CXX) -c $< -o $@ ${CXXFLAGS} -L${CPP_LOCAL_LIB_PATH} -I${CLIENTPATH}include/ -I${SERVERPATH}include/ 
	

simpleClient: ${CLIENTPATH}lib/simpleClient.o
	$(CXX) ${CLIENTPATH}lib/simpleClient.o -o ${CLIENTPATH}bin/simpleClientOutput -lncurses

server: ${SERVERPATH}lib/server.o
	$(CXX) ${SERVERPATH}lib/server.o -o ${SERVERPATH}bin/serverOutput 

simpleServer: ${SERVERPATH}lib/simpleServer.o
	$(CXX) ${SERVERPATH}lib/simpleServer.o -lpthread -o  ${SERVERPATH}bin/simpleServerOutput

registration: ${SERVERPATH}lib/registration.o
	$(CXX) ${SERVERPATH}lib/registration.o -I${CPP_MONGOCXX_PATH}  -I${CPP_BSONCXXX_PATH}  -L${CPP_LOCAL_LIB_PATH} -lmongocxx -lbsoncxx -lpthread -o ${SERVERPATH}bin/registrationOutput  

include auth/*

registrationTest: ${SERVERPATH}bin/registrationOutput
	${SERVERPATH}bin/./registrationOutput $(username) $(password) $(cluster)


clean:
	rm -rf ${SERVERPATH}lib/*.o ${SERVERPATH}bin/* ${CLIENTPATH}lib/*.o ${CLIENTPATH}bin/* 