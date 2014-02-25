#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include "GPTLHardwareCounter.h"

using namespace xolotlPerf;

//GPTLHardwareCounter::GPTLHardwareCounter(std::string aname, std::vector<HardwareQuantities> hquantities) :
//		HardwareCounter(aname, hquantities) {
//
//		name = aname;
//		quantities = hquantities;
//
//}

GPTLHardwareCounter::GPTLHardwareCounter(std::string aname) : HardwareCounter(aname) {

		name = aname;

}

GPTLHardwareCounter::~GPTLHardwareCounter() {

}


const std::string GPTLHardwareCounter::getName() const {

	return name;
}

void GPTLHardwareCounter::increment(){

}

//long long GPTLHardwareCounter::getValue() const {
//
//	double papival = 0.0;
//
//	// The following documentation was taken directly from gptl.c
//	/*
//	** GPTLget_eventvalue: return PAPI-based event value for a timer. All values will be
//	** returned as doubles, even if the event is not derived.
//	**
//	** Input args:
//	** const char *timername: timer name
//	** const char *eventname: event name (must be currently enabled)
//	** int t: thread number (if < 0, the request is for the current thread)
//	**
//	** Output args:
//	** double *value: current value of the event for this timer
//	*/
////	int gret = GPTLget_eventvalue( name.c_str(), -1, &papival );
//
//	return (long long)papival;
//}


//----------------------------
//HardwareCounter::HardwareCounter() {
//	name = "";
//	values = std::vector<long long>(quantities.size(), 0);
//}

//HardwareCounter::HardwareCounter(std::string aname, const std::vector<HardwareQuantities> &hquantities) :
//		name(aname), quantities(hquantities), values(std::vector<long long>(quantities.size(), 0)) {
//}

//HardwareCounter::HardwareCounter(const HardwareCounter &other) :
//		name(other.name), quantities(other.quantities), values(other.values) {
//}



//std::shared_ptr<HardwareCounter> HardwareCounter::clone() {
//	std::shared_ptr<HardwareCounter> HardwareCounter(new HardwareCounter(*this));
//	return HardwareCounter;
//}

//std::vector<long long> HardwareCounter::getValues() const {
//	// By default the values are set to zero.
////	int valuesLength = quantities.size();
////	std::vector<long long> values = std::vector<long long>(valuesLength, 0);
//
//	//GPTLget_value to get PAPI counters
//
//	return values;
//}

