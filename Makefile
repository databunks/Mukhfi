CXX = g++
CXXFLAGS = -Wall
SERVERPATH = server/

${SERVERPATH}lib/%.o: ${SERVERPATH}src/%.cpp
	$(CXX) -c $< -o $@ ${CXXFLAGS} -I ${SERVERPATH}include/ 

server: ${SERVERPATH}lib/Server.o
	$(CXX) ${SERVERPATH}lib/Server.o -o ${SERVERPATH}bin/serverOutput

clean:
	rm -rf ${SERVERPATH}lib/*.o ${SERVERPATH}bin/*