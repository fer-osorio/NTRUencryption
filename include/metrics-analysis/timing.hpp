#include<string>
#include<cstdint>
#include<functional>

#ifndef _TIMING_HPP_
#define _TIMING_HPP_

namespace Timing{

enum struct TestIDnum{ KEY_GENERATION, CIPHERING, DECIPHERING };
struct TestID{
private:
    const TestIDnum ID_num_;
    std::string label = "";
public:
    explicit TestID(TestIDnum ID_num): ID_num_(ID_num){
        switch(ID_num) {                                                        // Unique constructor. The idea to have the label uniquely determined by the TestIDnum.
        case TestIDnum::KEY_GENERATION:
            label = "Key Generation";
            break;
        case TestIDnum::CIPHERING:
            label = "Ciphering";
            break;
        case TestIDnum::DECIPHERING:
            label = "Deciphering";
            break;
        }
    }
    TestIDnum get_ID_num() const{ return this->ID_num_; }
    std::string get_label() const{ return this->label; }
};
template<typename T> std::vector<uint32_t> functionExecutionTiming(std::function<T()> func, TestID TID);

}
#endif