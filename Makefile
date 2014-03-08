CFLAGS= -Wformat -Wall -Wextra -Wunused-parameter -Wno-unused-result -Wno-sign-compare -std=c++11
ALLTARGETS=simm2 simmexp

all: $(ALLTARGETS)
simm2: Main2.cpp *.h
	g++ -o simm2 -O3 Main2.cpp $(CFLAGS)
simm: Main.cpp *.h
	g++ -o simm -O3 Main.cpp $(CLFAGS)
simmexp: Main-experimental.cpp *.h
	g++ -o simmexp -O3 Main-experimental.cpp $(CFLAGS)
clean:
	rm $(ALLTARGETS)
