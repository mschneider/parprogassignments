all: decrypt

decrypt: decrypt.cpp
	g++ -O3 --std=c++11 -fopenmp -lssl -lcrypto -lz decrypt.cpp -o decrypt

submission:
	tar -c decrypt.cpp Makefile taskCryptSolution.txt >submission.tar

