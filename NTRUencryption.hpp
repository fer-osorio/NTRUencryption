// Class for the implementation of the NTRUencryption algorithm.
#include<iostream>

#define p 3

struct NTRUencryption {
	private:																	// Parameters
	int* Nmod3Table = NULL;														// If 0 <= x < N, this array will save x%3 in its 'x' position (optimization)
	int* polySpace = NULL;														// Will represent a polynomial in the ring Zq[x]/(x^N-1)
};