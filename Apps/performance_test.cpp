#include<memory>
#include<vector>
#include<functional>
#include<cmath>
#include<iomanip>
#include<fstream>
#include<getopt.h>
#include<map>
#include"../Source/NTRUencryption.hpp"

class NTRUperformanceTester{
public:
    enum struct TestCategory{                                                   // -Enumerating the different performance tests that can be done
        NONE,                                                                   // -No tests, useful when asking for help
        KEY_GENERATION,                                                         // -Time for: Key generation
        ENCRYPTION,                                                             //  Encryption
        DECRYPTION,                                                             //  Decryption
        DATA_ANALYSIS,                                                          // -Data analysis on encrypted data
        ALL
    };
    struct Configuration{
        TestCategory category = TestCategory::NONE;
        std::string privateKeyFile = "";                                        // -[If available]: Retreaving key from file
        std::string messageFile = "";                                           // -Retreaving message from file
        std::string outputFile = "";                                            // -Writing output file
        size_t iterations = 0;
        bool verbose = false;                                                   // -Detailed output for diagnostic purposes
        bool jsonOutput = false;
    };
    struct TestResult{                                                          // -Test result structure. Intended for key generation, encryption and decryption
        std::string testName = "";                                              //  time
        double maximum = 0.0;
        double minimum = 0.0;
        double average = 0.0;
        double standardDeviation = 0.0;
        double averageAbsoluteDeviation = 0.0;
        std::string units = "microseconds";
        bool success = false;
        std::string errorMessage = "";
    };
    struct DataAnalysisResult{                                                  // -Data analysis. Intended for encrypted data.
        double entropy = 0.0;
        double correlation = 0.0;
        double xiSquare = 0.0;
        bool success = false;
        std::string errorMessage = "";
    };

private:
    Configuration config_;
    std::unique_ptr<NTRU::Encryption> encryption_ = nullptr;
    std::unique_ptr<NTRU::ZpPolynomial> testMessage_ = nullptr;
    std::vector<TestResult> results_ = {};
    DataAnalysisResult dataAnalysis_ = {};

    class ResourceManagement{                                                   // -Safe and generic way to create resources
    public:
        template<typename T>                                                    // -Declaring a template function. <typename T> is C++ version of "for all types T"
        static std::unique_ptr<T> createSafe(std::function<T*()> creator, const std::string& errorMsg){ // -std::function<T*()> callable object wrapper. Invoke any
            try{                                                                //  callable object with no arguments and with return type T*; creator will perform
                T* ptr = creator();                                             //  the actual allocation, or return an existing pointer.
                if(!ptr) throw std::runtime_error(errorMsg + ": null pointer returned");
                return std::unique_ptr<T>(ptr);                                 // -Return a unique pointer (unique ownership to the referred data)
            } catch(const std::exception& e){
                throw std::runtime_error(errorMsg + ": " + e.what());
            }
        }
    };

public:
    explicit NTRUperformanceTester(const Configuration& config): config_(config) {} // -Not allowing implicit type conversion

    bool run(){                                                                 // -Main execution method
        try{
            if(!this->initialize()){                                            // -If initialization fails, return false.
                return false;                                                   //  ...
            }
            this->executeTest();
            this->outputResults();
            return true;                                                        // -Everything Ok
        }catch(const std::exception& exp){
            std::cerr << "Error during test execution: " << exp.what() << std::endl;
            return false;
        }
    }

private:
    bool initialize(){                                                          // -Initialize encryption_ and testMessage_ objects
        try{
            if(!this->config_.privateKeyFile.empty()){                          // -If a private key file is provided, build from file.
                this->encryption_ = ResourceManagement::createSafe<NTRU::Encryption>(
                    [this](){ return new NTRU::Encryption(this->config_.privateKeyFile.c_str()); }, // -Lambda function as creator
                    "Failed to create encryption object from private key file"  // -Message in case of not being capable of create encryption object
                );
                if(!this->encryption_->validPrivateKeyAvailable()) {            // -Guarging against invalid private key provided
                    throw std::runtime_error("Invalid private key provided");
                }
            } else if(this->config_.category != TestCategory::KEY_GENERATION){  // -No private key file provided case
                this->encryption_ = ResourceManagement::createSafe<NTRU::Encryption>(
                    [this](){ return new NTRU::Encryption(); },                 // -Building keys automatically
                    "Failed to create encryption object"
                );
            }
            if(!this->config_.messageFile.empty()){                             // -Case: Test message provided from file
                this->testMessage_ = ResourceManagement::createSafe<NTRU::ZpPolynomial>(
                    [this](){ return new NTRU::ZpPolynomial(NTRU::Encryption::ZpPolynomialPlainTextFromFile(this->config_.messageFile.c_str())); },
                    "Failed to load message from file"
                );
            } else if(this->config_.category != TestCategory::KEY_GENERATION){  // -Case: No file provide for test message
                this->testMessage_ = ResourceManagement::createSafe<NTRU::ZpPolynomial>(
                    [this](){ return new NTRU::ZpPolynomial(NTRU::ZpPolynomial::randomTernary()); }, // -Automatically generating a test message
                    "Failed to create test message"
                );
            }
            return true;                                                        // -success. Encryption object and testMessage object created
        } catch(const std::exception& exp){
            std::cerr << "Initialization failed" << exp.what() << std::endl;
            return false;
        }
    }

    void executeTest(){
        switch(config_.category){
            case TestCategory::NONE:                                            // -Do nothing
                break;
            case TestCategory::KEY_GENERATION:
                this->runKeyGenerationTest();
                break;
            case TestCategory::ENCRYPTION:
                this->runEncryptionTest();
                break;
            case TestCategory::DECRYPTION:
                this->runDecryptionTest();
                break;
            case TestCategory::DATA_ANALYSIS:
                this->runDataAnalysisTest();
                break;
            case TestCategory::ALL:
                this->runAllTest();
                break;
        }
    }

    void runKeyGenerationTest(){
        if(config_.verbose) std::cout << "Running key generation performance test...\n";
        try{
            NTRU::Encryption::Statistics::Time stats = NTRU::Encryption::Statistics::Time::keyGeneration(); // -Performance test is running here
            TestResult result;                                                  // -If no exception thrown, proceed to organize the results.
            result.testName = "Key Generation";
            result.maximum = stats.getMaximum();
            result.minimum = stats.getMinimum();
            result.average = stats.getAverage();
            result.standardDeviation = std::sqrt(stats.getVariance());
            result.averageAbsoluteDeviation = stats.getAAD();
            result.units = "microseconds";
            result.success = true;

            this->results_.push_back(result);
        } catch(const std::exception& exp){                                     // -Signaling failure, writing exception message
            TestResult result;
            result.testName = "Key Generation";
            result.success = false;
            result.errorMessage = exp.what();
            results_.push_back(result);
        }
    }

    void runEncryptionTest(){
        if(this->config_.verbose) std::cout << "Running encryption performance test...\n";
        try{
            NTRU::Encryption::Statistics::Time stats = NTRU::Encryption::Statistics::Time::ciphering(this->encryption_.get(), this->testMessage_.get());

            TestResult result;                                                  // -If no exception thrown, proceed to organize the results.
            result.testName = "Encryption";
            result.maximum = stats.getMaximum();
            result.minimum = stats.getMinimum();
            result.average = stats.getAverage();
            result.standardDeviation = std::sqrt(stats.getVariance());
            result.averageAbsoluteDeviation = stats.getAAD();
            result.units = "microseconds";
            result.success = true;
            this->results_.push_back(result);
        } catch(const std::exception& exp){                                     // -Signaling failure, writing exception message
            TestResult result;
            result.testName = "Encryption";
            result.success = false;
            result.errorMessage = exp.what();
            results_.push_back(result);
        }
    }

    void runDecryptionTest() {
        if (this->config_.verbose) {
            std::cout << "Running decryption performance test..." << std::endl;
        }

        try {
            NTRU::Encryption::Statistics::Time stats = NTRU::Encryption::Statistics::Time::deciphering(encryption_.get(), nullptr);

            TestResult result;                                                  // -If no exception thrown, proceed to organize the results.
            result.testName = "Decryption";
            result.maximum = stats.getMaximum();
            result.minimum = stats.getMinimum();
            result.average = stats.getAverage();
            result.standardDeviation = std::sqrt(stats.getVariance());
            result.averageAbsoluteDeviation = stats.getAAD();
            result.units = "microseconds";
            result.success = true;

            results_.push_back(result);

        } catch (const std::exception& e) {
            TestResult result;
            result.testName = "Decryption";
            result.success = false;
            result.errorMessage = e.what();
            results_.push_back(result);
        }
    }

    void runDataAnalysisTest(){
        if(this->config_.verbose) std::cout << "Running data analysis test..." << std::endl;
        try{
            NTRU::Encryption::Statistics::Data stats = NTRU::Encryption::Statistics::Data::encryption(this->encryption_.get());

            this->dataAnalysis_.entropy = stats.getEntropy();                   // -If no exception thrown, proceed to organize the results.
            this->dataAnalysis_.correlation = stats.getCorrelation();
            this->dataAnalysis_.xiSquare = stats.getXiSquare();
            this->dataAnalysis_.success = true;
        } catch(const std::exception& exp){
            this->dataAnalysis_.success = false;
            this->dataAnalysis_.errorMessage = exp.what();
        }
    }

    void runAllTest(){
        this->runKeyGenerationTest();
        if(this->encryption_){
            this->runEncryptionTest();
            this->runDecryptionTest();
            this->runDataAnalysisTest();
        }
    }

    void outputResults() const{
        if(this->config_.jsonOutput) this->outputJsonResults();
        else this->outputTextResults();
        if(!this->config_.outputFile.empty()) this->saveResultsToFile();
    }

    void outputTextResults() const{
        std::cout << std::fixed << std::setprecision(5) << std::endl;
        for(const TestResult& result : this->results_){                         // Performance tests. Iterating through each TestResult element of results_ vector.
            if(result.success){
                std::cout << "\n" << result.testName << " Performance Statistics (" << result.units << "):\n";
                std::cout << "Maximum: " << result.maximum << "\n";
                std::cout << "Minimum: " << result.minimum << "\n";
                std::cout << "Average: " << result.average << "\n";
                std::cout << "Standard Deviation: " << result.standardDeviation << "\n";
                std::cout << "Average Absolute Deviation: " << result.averageAbsoluteDeviation << std::endl;
            } else{
                std::cout << result.testName << " Test failed: " << result.errorMessage << std::endl;
            }
        }
        if(this->dataAnalysis_.success){
            std::cout << "\nEncrypted data analysis:\n";
            std::cout << "Entropy: " << this->dataAnalysis_.entropy << "\n";
            std::cout << "Correlation: " << this->dataAnalysis_.correlation << "\n";
            std::cout << "Xi-Square: " << this->dataAnalysis_.xiSquare << std::endl;
        } else if(!this->dataAnalysis_.errorMessage.empty()){
            std::cout << "\nData Analysis Failed: " << this->dataAnalysis_.errorMessage << std::endl;
        }
    }

    void outputJsonResults() const{
        std::cout << "{\n";
        std::cout << "  \"performance_results\": [\n";
        for (size_t i = 0; i < results_.size(); ++i) {
            const auto& result = results_[i];
            std::cout << "    {\n";
            std::cout << "      \"test_name\": \"" << result.testName << "\",\n";
            std::cout << "      \"success\": " << (result.success ? "true" : "false") << ",\n";
            if (result.success) {
                std::cout << "      \"maximum\": " << result.maximum << ",\n";
                std::cout << "      \"minimum\": " << result.minimum << ",\n";
                std::cout << "      \"average\": " << result.average << ",\n";
                std::cout << "      \"standard_deviation\": " << result.standardDeviation << ",\n";
                std::cout << "      \"average_absolute_deviation\": " << result.averageAbsoluteDeviation << ",\n";
                std::cout << "      \"units\": \"" << result.units << "\"\n";
            } else {
                std::cout << "      \"error\": \"" << result.errorMessage << "\"\n";
            }
            std::cout << "    }";
            if (i < results_.size() - 1) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "  ]";
        if (dataAnalysis_.success) {
            std::cout << ",\n";
            std::cout << "  \"data_analysis\": {\n";
            std::cout << "    \"entropy\": " << dataAnalysis_.entropy << ",\n";
            std::cout << "    \"correlation\": " << dataAnalysis_.correlation << ",\n";
            std::cout << "    \"xi_square\": " << dataAnalysis_.xiSquare << "\n";
            std::cout << "  }\n";
        } else {
            std::cout << "\n";
        }
        std::cout << "}\n";
    }

    void saveResultsToFile() const{
        try{
            std::ofstream file(config_.outputFile);
            if(!file.is_open()) throw std::runtime_error("Cannot open output file: " + config_.outputFile);
            std::streambuf* originalBuffer = std::cout.rdbuf();                 // Redirect cout to file temporarily
            std::cout.rdbuf(file.rdbuf());
            if(this->config_.jsonOutput) this->outputJsonResults();
            else this->outputTextResults();
            std::cout.rdbuf(originalBuffer);
        } catch(const std::exception& exp){
            std::cerr << "Error saving results to file: " << exp.what() << std::endl;
        }
    }
};

class ArgumentParser{                                                           // Command line argument parser
public:
    static NTRUperformanceTester::Configuration parseArguments(int argc, char* argv[]){
        NTRUperformanceTester::Configuration config;
        static struct option long_options[]{                                    // -Static array instace of 'option' structure.
            {"help", no_argument, 0, 'h'},
            {"category", required_argument, 0, 'c'},
            {"private-key", required_argument, 0, 'k'},
            {"message", required_argument, 0, 'm'},
            {"output", required_argument, 0, 'o'},
            {"iterations", required_argument, 0, 'i'},
            {"verbose", no_argument, 0, 'v'},
            {"json", no_argument, 0, 'j'},
            {0,0,0,0}
        };
        int option_index = 0;
        int c;
        while((c = getopt_long(argc, argv, "hc:k:m:o:i:vj", long_options, &option_index)) != -1){
            switch(c){
                case 'h':
                    printUsage(argv[0]);
                    break;
                case 'c':
                    config.category = parseCategoryString(optarg);
                    break;
                case 'k':
                    config.privateKeyFile = optarg;
                    break;
                case 'm':
                    config.messageFile = optarg;
                    break;
                case 'o':
                    config.outputFile = optarg;
                    break;
                case 'i':
                    config.iterations = (size_t)std::stoi(optarg);
                    break;
                case 'v':
                    config.verbose = true;
                    break;
                case 'j':
                    config.jsonOutput = true;
                    break;
                case '?':
                    printUsage(argv[0]);
                    exit(1);
                    break;
                default:
                    abort();
            }
        }
        return config;
    }

private:
    static void printUsage(const char* programName){
        std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
        std::cout << "Options:\n";
        std::cout << "  -h, -help               Shows this help message\n";
        std::cout << "  -c, -category CATEGORY  Test category (keygen|encrypt|decrypt|data|all)\n";
        std::cout << "  -k, -private-key FILE   Private key file\n";
        std::cout << "  -m, -message FILE       Message file\n";
        std::cout << "  -o, -output FILE        Output file\n";
        std::cout << "  -v, -verbose            Verbose output\n";
        std::cout << "  -j, -json               JSON output format\n";
        std::cout << "\nExamples:\n";
        std::cout << "  " << programName << " --category all --verbose\n";
        std::cout << "  " << programName << " --category keygen --json --output results.json\n";
        std::cout << "  " << programName << " --private-key [Your private key] --category encrypt\n";
    }

    static NTRUperformanceTester::TestCategory parseCategoryString(const std::string& catstr){
        static const std::map<std::string, NTRUperformanceTester::TestCategory> categoryMap = { // -Associative container {<T1> key, <T2>value}.
            {"keygen", NTRUperformanceTester::TestCategory::KEY_GENERATION},    // -List initialization
            {"encrypt", NTRUperformanceTester::TestCategory::ENCRYPTION},
            {"decrypt", NTRUperformanceTester::TestCategory::DECRYPTION},
            {"data", NTRUperformanceTester::TestCategory::DATA_ANALYSIS},
            {"all", NTRUperformanceTester::TestCategory::ALL}
        };
        std::map<std::string, NTRUperformanceTester::TestCategory>::const_iterator it = categoryMap.find(catstr);
        if(it != categoryMap.end()) return it->second;
        throw std::invalid_argument("Invalid category" + catstr);
    }
};

int main(int argc, char* argv[]){
    try{
        NTRUperformanceTester::Configuration config = ArgumentParser::parseArguments(argc, argv);
        NTRUperformanceTester tester(config);
        return tester.run() ? EXIT_SUCCESS : EXIT_FAILURE;
    } catch(const std::exception& exp){
        std::cerr << "Error: " << exp.what() << std::endl;
        return EXIT_FAILURE;
    }
}
