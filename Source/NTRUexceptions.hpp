#include<stdexcept>
#include<vector>
#include <sstream> // For string building

class NTRUexception : public std::runtime_error {
private:
	std::string original_message = "";					// -Message generated in the origin of exception
	std::vector<std::string> trace_stack = {};				// -Intetion: Catch, add information about current location, re-throw
public:
	explicit NTRUexception(const std::string& what_arg) : std::runtime_error(""), original_message(what_arg) {
		this->trace_stack.push_back(what_arg);				// -First entry of the trace is the original error message
	}
	void add_trace_point(const std::string& trace_info){
		this->trace_stack.push_back(trace_info);			// -Add location to the trace
	}
	const char* what() const noexcept override{				// -Override virtual function what() to get full trace. Do not throw exception
		thread_local static						// -Static instance of full_trace for each thread. Static variable is needed because
		std::string full_trace;						//  what() returns a pointer to const char that must remain valid as long as the
		std::stringstream ss;						//  exception object istself exist.
		ss << "Exception Trace (from lastest to earliest):\n";		// -Like std::cout but writes in a internal string buffer.
		for(std::reverse_iterator<std::string> it = this->trace_stack.rbegin(); it != this->trace_stack.rend(); ++it){
			ss << "  -> " << *it << "\n";				// -Giving a format to the output message
		}
		full_trace = ss.str();
		return full_trace.c_str();					// -Returning treat-safe C-string with what() method requirements met.
	}
};

class MathException: public NTRUexception{
	explicit MathException(const std::string& what_arg) : NTRUexception(what_arg) {}
};

class FileIOException: public NTRUexception{
	explicit FileIOException(const std::string& what_arg) : NTRUexception(what_arg) {}
};