#include"../../include/ntru/encryption.hpp"
#include"../../include/ntru/parameters_constants.hpp"
#include<iostream>
#include<functional>
#include<chrono>
#include<memory>

#define NUMBER_OF_ROUNDS 1024U

template<typename T> static std::vector<uint32_t> run_test(std::function<T()> func, TestID tc) {
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    std::vector<uint32_t> time_measures(NUMBER_OF_ROUNDS);
    for(size_t i = 0; i < NUMBER_OF_ROUNDS; i++){
        if((i & 0xFF) == 0) std::cout << tc.get_label() << " test: Round "<< i << std::endl;
        begin = std::chrono::steady_clock::now();
        func();
        end  = std::chrono::steady_clock::now();
        time_measures[i] = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
    }
    std::cout << tc.get_label() << ". Total number of rounds completed: " << NUMBER_OF_ROUNDS << std::endl;
    return time_measures;
}

using namespace NTRU;

StatisticalMeasures::Dispersion<uint32_t>
Encryption::encryption_test(const TestID& t_id,
                            const NTRU::Encryption& e,
                            const NTRU::RpPolynomial& msg,
                            const NTRU::RqPolynomial& emsg){
    std::vector<uint32_t> time_measures;
    std::cout << t_id.get_label() << " test: Parameters N = "<< NTRU_N << ", q = " << NTRU_Q << " --------------" << std::endl;
    std::unique_ptr<NTRU::Encryption> ptr_e = nullptr;
    switch(t_id.get_ID_num()){
        case TestIDnum::KEY_GENERATION:
            ptr_e = std::make_unique<NTRU::Encryption>(NTRU::Encryption(e));
            time_measures = run_test<int>([&]()->int{ ptr_e->setKeys(); return 0; }, t_id);
            break;
        case TestIDnum::CIPHERING:
            time_measures = run_test<int>([&]()->int{ e.encrypt(msg); return 0; }, t_id);
            break;
        case TestIDnum::DECIPHERING:
            time_measures = run_test<int>([&]()->int{ e.decrypt(emsg);; return 0; }, t_id);
            break;
    }
    return StatisticalMeasures::Dispersion<uint32_t>(time_measures);
}

StatisticalMeasures::Dispersion<uint32_t> Encryption::keyGenerationTime(){
    Encryption e;                                                               // -Generates encryption object automatically
    return Encryption::encryption_test(TestID(TestIDnum::KEY_GENERATION), e, NTRU::RpPolynomial(), NTRU::RqPolynomial());
}

StatisticalMeasures::Dispersion<uint32_t> Encryption::ciphering(const NTRU::Encryption& e, const NTRU::RpPolynomial& msg){
    return Encryption::encryption_test(TestID(TestIDnum::CIPHERING), e, msg, NTRU::RqPolynomial());
}

StatisticalMeasures::Dispersion<uint32_t> Encryption::deciphering(const NTRU::Encryption& e, const NTRU::RqPolynomial& emsg){
    return Encryption::encryption_test(TestID(TestIDnum::DECIPHERING), e, NTRU::RpPolynomial(), emsg);
}
