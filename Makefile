CXX = g++
CXXFLAGS = -Wall -std=c++17
CLIENTPATH = client/
SERVERPATH = server/



CPP_BSONCXXX_PATH = /usr/local/include/bsoncxx/v_noabi
CPP_MONGOCXX_PATH = /usr/local/include/mongocxx/v_noabi/

CPP_LOCAL_LIB_PATH = /usr/local/lib

${SERVERPATH}lib/%.o: ${SERVERPATH}src/%.cpp
	$(CXX) -c  $< -o $@ ${CXXFLAGS} -I${CPP_MONGOCXX_PATH}  -I${CPP_BSONCXXX_PATH}  -L${CPP_LOCAL_LIB_PATH}   -I${SERVERPATH}include/ 

${CLIENTPATH}lib/%.o: ${CLIENTPATH}src/%.cpp
	$(CXX) -c $< -o $@ ${CXXFLAGS} -I${CPP_MONGOCXX_PATH}  -I${CPP_BSONCXXX_PATH} -L${CPP_LOCAL_LIB_PATH} -I${CLIENTPATH}include/ -I${SERVERPATH}include/ 
	

client: ${CLIENTPATH}lib/client.o
	$(CXX) ${CLIENTPATH}lib/client.o -I${CPP_MONGOCXX_PATH}   -I${CPP_BSONCXXX_PATH}  -L${CPP_LOCAL_LIB_PATH} -lmongocxx -lbsoncxx -lpthread -o ${CLIENTPATH}bin/clientOutput -lncurses -lpthread -lssl -lcrypto

server: ${SERVERPATH}lib/server.o
	$(CXX) ${SERVERPATH}lib/server.o -I${CPP_MONGOCXX_PATH}   -I${CPP_BSONCXXX_PATH}  -L${CPP_LOCAL_LIB_PATH} -lmongocxx -lbsoncxx -lpthread -o   ${SERVERPATH}bin/serverOutput -lpthread

registrationLogin: ${SERVERPATH}lib/registrationLogin.o
	$(CXX) ${SERVERPATH}lib/registrationLogin.o -I${CPP_MONGOCXX_PATH}   -I${CPP_BSONCXXX_PATH}  -L${CPP_LOCAL_LIB_PATH} -lmongocxx -lbsoncxx -lpthread -o ${SERVERPATH}bin/registrationLoginOutput  

include auth/*

clean:
	rm -rf ${SERVERPATH}lib/*.o ${SERVERPATH}bin/* ${CLIENTPATH}lib/*.o ${CLIENTPATH}bin/* 