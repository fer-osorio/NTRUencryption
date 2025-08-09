#include<numeric>	// For std::accumulate
#include<cmath>		// For abs, sqrt, ...
#include<optional>	// For optional members
#include<vector>	// For calculations
#include<array>		// For fixed size array
#include<cstddef>	// For std::byte
#include<cstdint>	// For uint8_t, uint32_t,...
#include<string>	// For string

#ifndef STATISTICAL_MEASURES_HPP
#define STATISTICAL_MEASURES_HPP

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

namespace StatisticalMeasures{

template <typename T> class Dispersion{
private:
	std::optional<double> Maximum = std::nullopt;
	std::optional<double> Minimum = std::nullopt;
	std::optional<double> Average = std::nullopt;
	std::optional<double> Variance = std::nullopt;
	std::optional<double> AvrAbsDev = std::nullopt;				// Average Absolute Deviation

public:
	Dispersion() = default;
	explicit Dispersion(const std::vector<T>& data){
		if(data.empty()) {
			return;							// Leave members as std::nullopt
		}
		double sum = std::accumulate(data.begin(), data.end(), 0.0);
		this->Average = sum / data.size();
		double min_val = static_cast<double>(data[0]);
		double max_val = static_cast<double>(data[0]);
		for (const T& val : data) {
			if (val < min_val) min_val = val;
			if (val > max_val) max_val = val;
		}
		this->Minimum = min_val;
		this->Maximum = max_val;

		double sq_sum = 0.0;
		double abs_dev_sum = 0.0;
		for (const T& val : data) {
			sq_sum += (static_cast<double>(val) - *this->Average) * (static_cast<double>(val) - *this->Average);
			abs_dev_sum += std::abs(static_cast<double>(val) - *this->Average);
		}
		this->Variance = data.size() > 1 ? sq_sum / (data.size() - 1) : 0.0;
		this->AvrAbsDev = abs_dev_sum / data.size();
	}

	// Getters
	std::optional<double> getMaximum()  const noexcept { return this->Maximum; }
	std::optional<double> getMinimum()  const noexcept { return this->Minimum; }
	std::optional<double> getAverage()  const noexcept { return this->Average; }
	std::optional<double> getVariance() const noexcept { return this->Variance; }
	std::optional<double> getAAD()      const noexcept { return this->AvrAbsDev; }
};

class DataRandomness{								// -Specialized to handle data from raw bytes
private:
	std::optional<double> Entropy = std::nullopt;
	std::optional<double> ChiSquare = std::nullopt;
	std::optional<double> CorrelationAdjacentByte = std::nullopt;

	size_t data_size = 0;
	std::array<uint32_t, 256> byteValueFrequence{};

	void calculate_entropy() {						// -Using Shannon entropy model
		double temp_entropy = 0.0, probability;
		for (const uint32_t& freq : byteValueFrequence) {
			if (freq > 0) {
				probability = static_cast<double>(freq) / this->data_size;
				temp_entropy -= probability * std::log2(probability);
			}
		}
		this->Entropy = temp_entropy;
	}

	void calculate_ChiSquare() {
		double temp_ChiSquare = 0.0;
		for (const uint32_t& freq : byteValueFrequence){
			temp_ChiSquare += static_cast<double>(freq*freq);
		}
		temp_ChiSquare *= 256.0/this->data_size;
		temp_ChiSquare -= this->data_size;
		this->ChiSquare = temp_ChiSquare;
	}

public:
	DataRandomness() = default;						// -Rule of Zero: No need for manual copy/assignment/destructor.
										//  The compiler-generated ones are correct.
	explicit DataRandomness(const std::vector<std::byte>& data) : data_size(data.size()) {
		if(data.empty()) return;
		for(const std::byte& byte : data) {				// Establishing the frequency of each of the possible values for a byte.
			byteValueFrequence[static_cast<uint8_t>(byte)]++;
		}
		this->calculate_entropy();
		this->calculate_ChiSquare();
		this->CorrelationAdjacentByte = this->calculateCorrelation(data, 1);
	}

	std::optional<double> calculateCorrelation(const std::vector<std::byte>& data, size_t offset) const { // Correlation is (arguably) better as a method since it requires an extra parameter.
		double average = 0.0;
		double variance = 0.0;
		double covariance = 0.0;
		size_t i, j, sz = data.size();

		for(i = 0; i < sz; i++) average += static_cast<double>(data[i]);
		average /= static_cast<double>(sz);

		for(i = 0, j = offset; j < sz; i++, j++){
			variance   += (static_cast<double>(data[i]) - average)*(static_cast<double>(data[i]) - average);
			covariance += (static_cast<double>(data[i]) - average)*(static_cast<double>(data[j]) - average);
		}
		for(j = 0; i < sz; i++, j++){
			variance   += (static_cast<double>(data[i]) - average)*(static_cast<double>(data[i]) - average);
			covariance += (static_cast<double>(data[i]) - average)*(static_cast<double>(data[j]) - average);
		}
		variance   /= static_cast<double>(sz);
		covariance /= static_cast<double>(sz);
		return covariance/variance;
	}

	std::optional<double> getEntropy() const noexcept { return this->Entropy; }
	std::optional<double> getChiSquare() const noexcept { return this->ChiSquare; }
	std::optional<double> getCorrelationAdjacentByte() const noexcept { return this->CorrelationAdjacentByte; }
};
}
#endif