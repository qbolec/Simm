all: simm2 simm
simm2: Main2.cpp *.h
	g++ -o simm2 -O3 Main2.cpp
simm: Main.cpp *.h
	g++ -o simm -O3 Main.cpp
