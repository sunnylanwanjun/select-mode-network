#ifndef __TIMESTAMPH__
#define __TIMESTAMPH__
#include <chrono>
class Timestamp {
public:
	Timestamp() {
		Update();
	}
	~Timestamp() {

	}
	void Update() {
		_begin = std::chrono::high_resolution_clock::now();
	}
	long long GetWS() {
		return std::chrono::duration_cast<std::chrono::microseconds>
			(std::chrono::high_resolution_clock::now() - _begin).count();
	}
	double GetHS() {
		return GetWS()*0.001;
	}
	double GetS() {
		return GetWS()*0.000001;
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _begin;
};
#endif