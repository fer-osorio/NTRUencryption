#include<iostream>
//#include<functional>
#include<chrono>
//#include<memory>
#include"../../include/metrics-analysis/timing.hpp"

#define NUMBER_OF_ROUNDS 1024U

using namespace Timing;

template<typename T> std::vector<uint32_t> functionExecutionTiming(std::function<T()> func, TestID TID) {
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    std::vector<uint32_t> time_measures(NUMBER_OF_ROUNDS);
    for(size_t i = 0; i < NUMBER_OF_ROUNDS; i++){
        if((i & 0xFF) == 0) std::cout << TID.get_label() << " test: Round "<< i << std::endl;
        begin = std::chrono::steady_clock::now();
        func();
        end  = std::chrono::steady_clock::now();
        time_measures[i] = (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
    }
    std::cout << TID.get_label() << ". Total number of rounds completed: " << NUMBER_OF_ROUNDS << std::endl;
    return time_measures;
}
