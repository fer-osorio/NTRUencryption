#include<cmath>
#include<iomanip>
#include<limits>
#include"../Source/NTRUencryption.hpp"

static const NTRU_N NTRU_N_values[]    = {    _509_,     _677_ ,    _701_,     _821_,     _1087_,     _1171_,     _1499_};// -All the possible values for the N
static const char NTRU_N_valuesList[]  = "(0) _509_, (1) _677_, (2) _701_, (3) _821_, (4) _1087_, (5) _1171_, (6) _1499_";// -Useful for CLI
static const size_t NTRU_N_amount      = sizeof(NTRU_N_values)/sizeof(NTRU_N_values[0]);

static const NTRU_q NTRU_q_values[]    = {    _2048_,     _4096_ ,     _8192_};        // -All the possible values for the q
static const char NTRU_q_valuesList[]  = "(0) _2048_, (1) _4096_ , (2) _8192_";        // -Useful for CLI
static const size_t NTRU_q_amount      = sizeof(NTRU_q_values)/sizeof(NTRU_q_values[0]);

static std::string selectNTRU_N =
std::string("Select NTRU_N parameter:\n") + NTRU_N_valuesList + "\n";

static std::string selectNTRU_q =
std::string("Select NTRU_q parameter:\n") + NTRU_q_valuesList + "\n";

static std::string statsCategory =
"What statistics do you want to get?\n(0) Key Generation time \n(1) Ciphering and deciphering time\n(2) Key Generation time, ciphering and deciphering time \n(3) Encrypted data\n(4) All\n";

static const char invalidInputMsg[] = "\nInvalid input. Try again.\n";

static int validInput_int(){
    int input;
    while(!(std::cin >> input)) {
        std::cin.clear();                                                       // -Clear bad input flag
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');     // -Discard input
        std::cout << "Invalid input; please re-enter.\n";
    }
    return input;
}

static int retreaveValidOption(std::string optionsString, bool (validOptionCriteria)(int)) { // -Will ask the user for input from a set of options
    int option;
    std::cout << optionsString;
    option = validInput_int();
    getchar();                                                                  // -Will take the "\n" left behind at the moment of press enter
    while(!validOptionCriteria(option)) {                                       // -Validating the option using the criteria specified by 'validOptionCriteria'
        std::cout << invalidInputMsg;                                           //  function. If not valid, it will reaped the process
        std::cout << optionsString;
        option = validInput_int();
        getchar();                                                              // -Will take the "\n" left behind at the moment of press enter
    }
    return option;
}

static bool NTRU_N_validOpt(int opt) { return opt >= 0 && opt < (int)NTRU_N_amount; }
static bool NTRU_q_validOpt(int opt) { return opt >= 0 && opt < (int)NTRU_q_amount; }
static bool statisticCategory(int opt) { return opt >= 0 && opt < 5; }

int main(int argc, const char* argv[]){
    if(argc > 1) {
        std::cout << "Executable does not support arguments," << std::endl;
        std::cout << "I will ignore: ";
        for(int i = 1; i < argc; i++) std::cout << argv[i] << ' ';
        std::cout  << std::endl;
    }
    int opt_N = retreaveValidOption(selectNTRU_N, NTRU_N_validOpt);
    int opt_q = retreaveValidOption(selectNTRU_q, NTRU_q_validOpt);
    int opt_C = retreaveValidOption(statsCategory, statisticCategory);

    NTRU_N N = NTRU_N_values[opt_N];
    NTRU_q q = NTRU_q_values[opt_q];

    NTRU::Encryption::Statistics::Time kg;
    NTRU::Encryption::Statistics::Time ch, dc;
    NTRU::Encryption::Statistics::Data dt;

    std::cout << '\n';

    switch(opt_C){
        case 0:
            std::cout << "Computing Statistics::Time::keyGeneration(" << N << ", " << q << ")\n";
            kg = NTRU::Encryption::Statistics::Time::keyGeneration(N,q);
            std::cout << std::endl;
            break;
        case 1:
            std::cout << "Computing Statistics::Time::ciphering(" << N << ", " << q << ")\n";
            std::cout << "Computing Statistics::Time::deciphering(" << N << ", " << q << ")\n";
            ch = NTRU::Encryption::Statistics::Time::ciphering(N,q);
            dc = NTRU::Encryption::Statistics::Time::deciphering(N,q);
            std::cout << std::endl;
            break;
        case 2:
            std::cout << "Computing Statistics::Time::keyGeneration(" << N << ", " << q << ")\n";
            std::cout << "Computing Statistics::Time::ciphering(" << N << ", " << q << ")\n";
            std::cout << "Computing Statistics::Time::deciphering(" << N << ", " << q << ")\n";
            kg = NTRU::Encryption::Statistics::Time::keyGeneration(N,q);
            ch = NTRU::Encryption::Statistics::Time::ciphering(N,q);
            dc = NTRU::Encryption::Statistics::Time::deciphering(N,q);
            std::cout << std::endl;
            break;
        case 3:
            std::cout << "Computing Statistics::Data::encryption(" << N << ", " << q << ")\n";
            dt = NTRU::Encryption::Statistics::Data::encryption(N,q);
            std::cout << std::endl;
            break;
        default:
            std::cout << "Computing Statistics::Time::keyGeneration(" << N << ", " << q << ")\n";
            std::cout << "Computing Statistics::Time::ciphering(" << N << ", " << q << ")\n";
            std::cout << "Computing Statistics::Data::encryption(" << N << ", " << q << ")\n";
            kg = NTRU::Encryption::Statistics::Time::keyGeneration(N,q);
            ch = NTRU::Encryption::Statistics::Time::ciphering(N,q);
            dc = NTRU::Encryption::Statistics::Time::deciphering(N,q);
            dt = NTRU::Encryption::Statistics::Data::encryption(N,q);
            break;
    }

    std::cout << "Parameters:: N = "<< N << ", q = " << q << " ------------------------------------" << std::endl;
    std::cout << std::fixed << std::setprecision(5) <<std::endl;

    if(opt_C == 0 || opt_C == 2 || opt_C == 4){
        std::cout << "Key Generation time statistics:" << std::endl;
        std::cout << "Maximum: " << kg.getMaximum() << '\n';
        std::cout << "Minimum: " << kg.getMinimum() << '\n';
        std::cout << "Average: " << kg.getAverage() << '\n';
        std::cout << "Standard deviation: " << sqrt(kg.getVariance()) << '\n';
        std::cout << "Average Absolute Deviation: " << kg.getAAD() << '\n' << std::endl;
    }

    if(opt_C == 1 || opt_C == 2 || opt_C == 4){
        std::cout << "Encryption time statistics:" << std::endl;
        std::cout << "Maximum: " << ch.getMaximum() << '\n';
        std::cout << "Minimum: " << ch.getMinimum() << '\n';
        std::cout << "Average: " << ch.getAverage() << '\n';
        std::cout << "Standard deviation: " << sqrt(ch.getVariance()) << '\n';
        std::cout << "Average Absolute Deviation: " << ch.getAAD() << '\n' << std::endl;

        std::cout << "Decryption time statistics:" << std::endl;
        std::cout << "Maximum: " << dc.getMaximum() << '\n';
        std::cout << "Minimum: " << dc.getMinimum() << '\n';
        std::cout << "Average: " << dc.getAverage() << '\n';
        std::cout << "Standard deviation: " << sqrt(dc.getVariance()) << '\n';
        std::cout << "Average Absolute Deviation: " << dc.getAAD() << '\n' << std::endl;
    }

    if(opt_C == 3 || opt_C == 4){
        std::cout << "Encrypted data statistics: " << std::endl;
        std::cout << "Entropy: " << dt.getEntropy() << '\n';
        std::cout << "Correlation: " << dt.getCorrelation() << '\n';
        std::cout << "XiSquare: " << dt.getXiSquare() << '\n' << std::endl;
    }

    return 0;
}
