all: worley

worley: worley.cpp
	g++-4.8 -O3 -fopenmp worley.cpp -lm -o worley

submission:
	tar -c worley.cpp Makefile >submission.tar

