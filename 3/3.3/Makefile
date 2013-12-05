all: cellnoise cellnoise2


cellnoise2: Cellnoise.cpp
	g++-4.8 -fopenmp CellNoise.cpp -lm -o cellnoise


cellnoise: worley.cpp
	g++-4.8 -fopenmp worley.cpp -lm -o worley
submission:
	tar -c worley.cpp Makefile >submission.tar

