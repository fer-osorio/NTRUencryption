#include<cmath>
#include"../Source/NTRUencryption.hpp"

int main(int argc, const char* argv[]){
    NTRU::Encryption::KeyGenerationTimeStats t = NTRU::Encryption::keyGenTimeStats();
    std::cout << "Maximum: " << t.getMaximum() << '\n';
    std::cout << "Minimum: " << t.getMinimum() << '\n';
    std::cout << "Average: " << t.getAverage() << '\n';
    std::cout << "Standard deviation: " << sqrt(t.getMaximum()) << '\n';
    return 0;
}
