#ifndef _PARAMETERS_CONSTATNS_
#define _PARAMETERS_CONSTATNS_
#ifndef NTRU_N
    #define NTRU_N 701
#endif
#ifndef NTRU_Q
    #define NTRU_Q 8192
#endif

namespace NTRU{

enum ntru_N {_509_  = 509,  _677_  = 677,  _701_  = 701,  _821_ = 821, _1087_ = 1087, _1171_ = 1171, _1499_ = 1499 };	// All the possible values for the N
enum ntru_q {_2048_ = 2048, _4096_ = 4096, _8192_ = 8192 };			// All the possible values for the q
//enum ntru_p {_3_= 3 };

constexpr static int log2_q(ntru_q q) {                                         // -Returns logarithm base 2 of a ntru_q value
    int lg2q = 0, qq = q;
    for(; qq > 1; qq >>= 1, lg2q++) {}
    return lg2q;
}
constexpr int log2q  = log2_q((ntru_q)NTRU_Q);

}
#endif