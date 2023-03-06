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
	

client: ${CLIENTPATH}lib/client.o
	$(CXX) ${CLIENTPATH}lib/client.o -o ${CLIENTPATH}bin/clientOutput -lncurses -lpthread

server: ${SERVERPATH}lib/server.o
	$(CXX) ${SERVERPATH}lib/server.o -o  ${SERVERPATH}bin/serverOutput -lpthread

registrationLogin: ${SERVERPATH}lib/RegistrationLogin.o
	$(CXX) ${SERVERPATH}lib/RegistrationLogin.o -I${CPP_MONGOCXX_PATH}  -I${CPP_BSONCXXX_PATH}  -L${CPP_LOCAL_LIB_PATH} -lmongocxx -lbsoncxx -lpthread -o ${SERVERPATH}bin/RegistrationLoginOutput  

include auth/*

clean:
	rm -rf ${SERVERPATH}lib/*.o ${SERVERPATH}bin/* ${CLIENTPATH}lib/*.o ${CLIENTPATH}bin/* 