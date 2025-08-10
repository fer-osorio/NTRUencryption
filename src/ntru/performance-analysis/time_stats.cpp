#include<iostream>
#include<memory>
#include"../../include/ntru/encryption.hpp"
#include"../../include/ntru/parameters_constants.hpp"

using namespace NTRU;
using namespace Timing;

StatisticalMeasures::Dispersion<uint32_t>
Encryption::encryption_test(const TestID& t_id,
                            const Encryption& e,
                            const RpPolynomial& msg,
                            const RqPolynomial& emsg){
    std::vector<uint32_t> time_measures;
    std::cout << t_id.get_label() << " test: Parameters N = "<< NTRU_N << ", q = " << NTRU_Q << " --------------" << std::endl;
    std::unique_ptr<Encryption> ptr_e = nullptr;
    switch(t_id.get_ID_num()){
        case TestIDnum::KEY_GENERATION:
            ptr_e = std::make_unique<Encryption>(Encryption(e));
            time_measures = functionExecutionTiming<int>([&]()->int{ ptr_e->setKeys(); return 0; }, t_id);
            break;
        case TestIDnum::CIPHERING:
            time_measures = functionExecutionTiming<int>([&]()->int{ e.encrypt(msg); return 0; }, t_id);
            break;
        case TestIDnum::DECIPHERING:
            time_measures = functionExecutionTiming<int>([&]()->int{ e.decrypt(emsg);; return 0; }, t_id);
            break;
    }
    return StatisticalMeasures::Dispersion<uint32_t>(time_measures);
}

StatisticalMeasures::Dispersion<uint32_t> Encryption::keyGenerationTime(){
    Encryption e;                                                               // -Generates encryption object automatically
    return Encryption::encryption_test(TestID(TestIDnum::KEY_GENERATION), e, RpPolynomial(), RqPolynomial());
}

StatisticalMeasures::Dispersion<uint32_t> Encryption::cipheringTime(const Encryption& e, const RpPolynomial& msg){
    return Encryption::encryption_test(TestID(TestIDnum::CIPHERING), e, msg, RqPolynomial());
}

StatisticalMeasures::Dispersion<uint32_t> Encryption::decipheringTime(const Encryption& e, const RqPolynomial& emsg){
    return Encryption::encryption_test(TestID(TestIDnum::DECIPHERING), e, RpPolynomial(), emsg);
}
