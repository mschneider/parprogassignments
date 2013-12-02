all: cellnoise

cellnoise: worley.cpp
	g++ -4.8 -fopenmp worley.cpp -lm -o worley
submission:
	tar -c worley.cpp Makefile >submission.tar

