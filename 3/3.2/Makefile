all: decrypt

decrypt: decrypt.cpp
	g++-4.8 -O3 --std=c++11 -fopenmp decrypt.cpp -o decrypt

submission:
	tar -c decrypt.cpp Makefile >submission.tar

