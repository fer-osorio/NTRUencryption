#include<cmath>
#include<iomanip>
#include<limits>
#include"../Source/NTRUencryption.hpp"

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

static bool statisticCategory(int opt) { return opt >= 0 && opt < 5; }

int main(int argc, const char* argv[]){
    NTRU::Encryption* ptr_e = NULL;
    NTRU::Encryption::Statistics::Time kg;
    NTRU::Encryption::Statistics::Time ch, dc;
    NTRU::Encryption::Statistics::Data dt;
    int opt_C = retreaveValidOption(statsCategory, statisticCategory);
    switch(argc) {                                                              // -Accept one parameter, intended for the private key
        case 1:
            if(opt_C != 0){                                                     // -If no argument passed and if not just key creation statistics
                try {                                                           //  needed,the program will generate a encryption object
                    ptr_e = new NTRU::Encryption();
                } catch(const std::runtime_error& exp){
                    std::cerr << "Could not create NTRU::Encryption object.\n" << exp.what() << '\n';
                    return EXIT_FAILURE;
                }
            }
            break;
        case 2:
            try{
                ptr_e = new NTRU::Encryption(argv[1]);                          // -Creating encryption object from (supposedly) private key file
            } catch(const std::runtime_error& exp){
                std::cerr << "\nCould not create NTRU::Encryption object from file.\n\n" << exp.what() << '\n';
                return EXIT_FAILURE;
            }
            if(!ptr_e->validPrivateKeyAvailable()){                             // -Validating polynomial passed as private key.
                std::cerr << "\n\nExecutable argument must refer to a valid private key. Terminating the program with FAILURE status.\n\n";
                return EXIT_FAILURE;
            }
            break;
        default:
            std::cerr << "\n\nExecutable accept one and just one argument. Termination the program with FAILURE status.\n\n";
            return EXIT_FAILURE;
    }

    if(ptr_e != NULL) {
        std::cout << "Running test with:" << '\n';
        ptr_e->printKeys("Public key","Private key");
        std::cout << std::endl;
    }

    switch(opt_C){
        case 0:
            std::cout << "Computing Statistics::Time::keyGeneration()\n";
            kg = NTRU::Encryption::Statistics::Time::keyGeneration();
            std::cout << std::endl;
            break;
        case 1:
            std::cout << "Computing Statistics::Time::ciphering()\n";
            std::cout << "Computing Statistics::Time::deciphering()\n";
            ch = NTRU::Encryption::Statistics::Time::ciphering(ptr_e);
            dc = NTRU::Encryption::Statistics::Time::deciphering(ptr_e);
            std::cout << std::endl;
            break;
        case 2:
            std::cout << "Computing Statistics::Time::keyGeneration()\n";
            std::cout << "Computing Statistics::Time::ciphering()\n";
            std::cout << "Computing Statistics::Time::deciphering()\n";
            kg = NTRU::Encryption::Statistics::Time::keyGeneration();
            ch = NTRU::Encryption::Statistics::Time::ciphering(ptr_e);
            dc = NTRU::Encryption::Statistics::Time::deciphering(ptr_e);
            std::cout << std::endl;
            break;
        case 3:
            std::cout << "Computing Statistics::Data::encryption()\n";
            dt = NTRU::Encryption::Statistics::Data::encryption(ptr_e);
            std::cout << std::endl;
            break;
        default:
            std::cout << "Computing Statistics::Time::keyGeneration()\n";
            std::cout << "Computing Statistics::Time::ciphering()\n";
            std::cout << "Computing Statistics::Data::encryption()\n";
            std::cout << "Computing Statistics::Time::deciphering()\n";
            kg = NTRU::Encryption::Statistics::Time::keyGeneration();
            ch = NTRU::Encryption::Statistics::Time::ciphering(ptr_e);
            dc = NTRU::Encryption::Statistics::Time::deciphering(ptr_e);
            dt = NTRU::Encryption::Statistics::Data::encryption(ptr_e);
            break;
    }
    if(ptr_e != NULL) { delete ptr_e; ptr_e = NULL; }                           // Encryption object not usefull anymore.
    std::cout << std::fixed << std::setprecision(5) << std::endl;
    if(opt_C == 0 || opt_C == 2 || opt_C == 4){
        std::cout << "\nKey Generation time statistics (microseconds):" << std::endl;
        std::cout << "Maximum: " << kg.getMaximum() << '\n';
        std::cout << "Minimum: " << kg.getMinimum() << '\n';
        std::cout << "Average: " << kg.getAverage() << '\n';
        std::cout << "Standard deviation: " << sqrt(kg.getVariance()) << '\n';
        std::cout << "Average Absolute Deviation: " << kg.getAAD() << '\n' << std::endl;
    }

    if(opt_C == 1 || opt_C == 2 || opt_C == 4){
        std::cout << "Encryption time statistics (microseconds):" << std::endl;
        std::cout << "Maximum: " << ch.getMaximum() << '\n';
        std::cout << "Minimum: " << ch.getMinimum() << '\n';
        std::cout << "Average: " << ch.getAverage() << '\n';
        std::cout << "Standard deviation: " << sqrt(ch.getVariance()) << '\n';
        std::cout << "Average Absolute Deviation: " << ch.getAAD() << '\n' << std::endl;

        std::cout << "Decryption time statistics (microseconds):" << std::endl;
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

    std::cout << "\nAppendix: From ZpPolynomial to number.\n\n";
    for(int i = 0; i < 5; i++){
        NTRU::ZpPolynomial r = NTRU::ZpPolynomial::randomTernary();
        r.println("r polynomial form");
        mpz_class num_r = r.toNumber();
        std::cout << "\nr in number == " << num_r << '\n' << std::endl;
    }

    return 0;
}
