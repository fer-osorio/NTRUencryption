// Parameters and the modular integer groups p and q generate

enum NTRU_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821 };		// All the possible values for the N
enum NTRU_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };						// All the possible values for the q
enum NTRU_p {_3_	= 3 };

enum Z2 {_0_ = 0, _1_ = 1};														// Integers modulo 2 (binary numbers)

Z2 operator + (Z2 a,Z2 b) {														// Addition modulus 2
	if(a!=b) return _1_;
	return _0_;
}
Z2 operator - (Z2 a,Z2 b) {														// Addition and subtraction coincide in Z2. This is just for evade problems
	if(a!=b) return _1_;														// with notation
	return _0_;
}
Z2 operator * (Z2 a, Z2 b) {													// Multiplication modulus 2
	if(a==0) return _0_;
	return  b ;
}
void operator += (Z2& a, Z2 b) {
	if(a != b) a = _1_;
	else a = _0_;
}
void operator -= (Z2& a, Z2 b) {
	if(a != b) a = _1_;
	else a = _0_;
}

class Zq{
	NTRU_q q;
	const int q_1;

	public: Zq(NTRU_q _q_): q(_q_), q_1(_q_-1) {}
	public: inline NTRU_q get_q() { return q; }

	int add(int a, int b) {
		int r = a + b;
		while(r < 0) r += this->q;
		return r & q_1;
	}
	int subtract(int a, int b) {
		int r = a - b;
		while(r < 0) r += this->q;
		return r & q_1;
	}
	int product(int a, int b) {
		int r = a * b;
		while(r < 0) r += this->q;
		return r & q_1;
	}
};