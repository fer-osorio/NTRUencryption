struct Timing{
private:
	double Maximum  =  0.0;
	double Minimum  =  0.0;
	double Average  = -1.0;
	double Variance =  0.0;
	double AvrAbsDev=  0.0;

	double maximum(const uint64_t time_data[], size_t size) const;
	double minimum(const uint64_t time_data[], size_t size) const;
	double average(const uint64_t time_data[], size_t size) const;
	double variance(const uint64_t time_data[], size_t size) const;
	double avrAbsDev(const uint64_t time_data[], size_t size) const;

	Timing(const uint64_t time_data[], size_t size);

public:
	Timing(){}
	double getMaximum() const{ return this->Maximum; }
	double getMinimum() const{ return this->Minimum; }
	double getAverage() const{ return this->Average; }
	double getVariance()const{ return this->Variance;}
	double getAAD()     const{ return this->AvrAbsDev; }			// -Average absolute deviation
};

struct DataAnalysis{
private:
	double Entropy = 0.0;
	double XiSquare = 0.0;
	double Correlation = 10.0;

	uint32_t byteValueFrequence[256] = {0};
	bool byteValueFrequenceStablisched = false;
	void setbyteValueFrequence(const char data[], size_t size);

	double entropy(const char data[], size_t size);
	double xiSquare(const char data[], size_t size);
	double correlation(const char data[], size_t size, size_t offset);

	DataAnalysis(const char data[], size_t size);

public:
	DataAnalysis(){}
	DataAnalysis(const DataAnalysis& d){
		this->Entropy = d.Entropy;
		this->XiSquare = d.XiSquare;
		this->Correlation = d.Correlation;
		for(int i = 0; i < 256; i++) this->byteValueFrequence[i] = d.byteValueFrequence[i];
		this->byteValueFrequenceStablisched = d.byteValueFrequenceStablisched;
	}
	DataAnalysis& operator = (const DataAnalysis& d){
		if(this!=&d){
			this->Entropy = d.Entropy;
			this->XiSquare = d.XiSquare;
			this->Correlation = d.Correlation;
			for(int i = 0; i < 256; i++) this->byteValueFrequence[i] = d.byteValueFrequence[i];
			this->byteValueFrequenceStablisched = d.byteValueFrequenceStablisched;
		}
		return *this;
	}
	double getEntropy() const{ return this->Entropy; }
	double getCorrelation() const { return this->Correlation; }
	double getXiSquare() const{ return this->XiSquare; }
};