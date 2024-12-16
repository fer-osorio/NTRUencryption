#include<cmath>
#include <iomanip>
#include"../Source/NTRUencryption.hpp"

int main(int argc, const char* argv[]){
    if(argc > 1) {
        std::cout << "Executable does not support arguments," << std::endl;
        std::cout << "I will ignore: ";
        for(int i = 1; i < argc; i++) std::cout << argv[i] << ' ';
        std::cout  << std::endl;
    }
    NTRU::Encryption::Statistics::Time kg = NTRU::Encryption::Statistics::Time::keyGeneration();
    NTRU::Encryption::Statistics::Time ch = NTRU::Encryption::Statistics::Time::ciphering();
    NTRU::Encryption::Statistics::Data dt = NTRU::Encryption::Statistics::Data::encryption();
    std::cout << std::fixed << std::setprecision(5) <<std::endl;
    std::cout << "Key Generation:" << std::endl;
    std::cout << "Maximum: " << kg.getMaximum() << '\n';
    std::cout << "Minimum: " << kg.getMinimum() << '\n';
    std::cout << "Average: " << kg.getAverage() << '\n';
    std::cout << "Standard deviation: " << sqrt(kg.getMaximum()) << '\n';
    std::cout << "Average Absolute Deviation: " << kg.getAAD() << '\n' << std::endl;

    std::cout << "Encryption:" << std::endl;
    std::cout << "Maximum: " << ch.getMaximum() << '\n';
    std::cout << "Minimum: " << ch.getMinimum() << '\n';
    std::cout << "Average: " << ch.getAverage() << '\n';
    std::cout << "Standard deviation: " << sqrt(ch.getMaximum()) << '\n';
    std::cout << "Average Absolute Deviation: " << ch.getAAD() << '\n' << std::endl;

    std::cout << "Data: " << std::endl;
    std::cout << "Entropy: " << dt.getEntropy() << '\n';
    std::cout << "Correlation: " << dt.getCorrelation() << '\n' << std::endl;

    return 0;
}
