#include "DummyEventCounter.h"

using namespace xolotlPerf;


DummyEventCounter::DummyEventCounter(std::string aname) : EventCounter(aname) {

	name = aname;
	value = 0;
}


DummyEventCounter::~DummyEventCounter() {

}

int DummyEventCounter::getValue() {
	return 0;
}

/**
 * This operation returns the name.
 * @return the name
 */
const std::string DummyEventCounter::getName() const {
	return "";
}

void DummyEventCounter::increment(){

}