/*
 * PSIClusterNetworkLoaderTester.cpp
 *
 *  Created on: Mar 30, 2013
 *      Author: jaybilly
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Regression

#include <boost/test/included/unit_test.hpp>
#include <PSIClusterNetworkLoader.h>
#include <memory>
#include <typeinfo>
#include <limits>

using namespace std;
using namespace xolotlCore;

/**
 * This suite is responsible for testing the PSIClusterNetworkLoader. It
 * creates a string stream that contains each of the available PSICluster types
 * and checks that the loader returns a list with each type in it.
 */BOOST_AUTO_TEST_SUITE (PSIClusterNetworkLoader_testSuite)

/** This operation checks the loader. */
BOOST_AUTO_TEST_CASE(checkLoading) {

	// Local Declarations
	shared_ptr<stringstream> networkStream(
			new stringstream(stringstream::in | stringstream::out));
	string singleHeString = "1 0 0 0.0 Infinity Infinity 8.269996 0.999 1.34\n";
	string singleVString =
			"0 50 0 Infinity 2.49000002 Infinity Infinity 0.888 2.345\n";
	string singleIString = "0 0 1 Infinity Infinity Infinity Infinity 0.7777 3.456\n";
	string mixedString =
			"1 50 0 6.160001 2.4900002 Infinity Infinity 6.789 4.5678\n";
	PSIClusterNetworkLoader loader = PSIClusterNetworkLoader();

	// Load the network stream. This simulates a file with single He, single
	// V, single I and one mixed-species cluster.
	*networkStream << singleHeString << singleVString << singleIString
			<< mixedString;

	// Diagnostic information
	// @formatter:off
	BOOST_TEST_MESSAGE("CLUSTER DATA"
			<< "\nHe: "
			<< singleHeString
			<< "\nV: "
			<< singleVString
			<< "\nI: "
			<< singleIString
			<< "\n Mixed: "
			<< mixedString
			<< "\nFull Network data: \n"
			<< (*networkStream).str());
	// @formatter:off

	// Setup the Loader
	loader.setInputstream(networkStream);

	// Load the network
	shared_ptr<ReactionNetwork> network = loader.load();

	// Check the network. It should not be empty
	std::map<std::string, std::string> props = *(network->properties);
	std::vector<std::shared_ptr<Reactant>> reactants = *(network->reactants);
	BOOST_TEST_MESSAGE("Network Size = " << reactants.size());
	BOOST_TEST_MESSAGE("Number of properties = " << props.size());
	BOOST_REQUIRE(!props.empty());
	BOOST_REQUIRE(!reactants.empty());
	// It should have four reactants
	BOOST_REQUIRE_EQUAL(4, reactants.size());
	// It should have six properties
	BOOST_REQUIRE_EQUAL(7, props.size());

	// Check the properties
	BOOST_TEST_MESSAGE("Maximum He Cluster Size = " << props["maxHeClusterSize"]);
	BOOST_REQUIRE(strtol(props["maxHeClusterSize"].c_str(),NULL,10) == 1);
	BOOST_TEST_MESSAGE("Maximum V Cluster Size = " << props["maxVClusterSize"]);
	BOOST_REQUIRE(strtol(props["maxVClusterSize"].c_str(),NULL,10) == 50);
	BOOST_TEST_MESSAGE("Maximum Interstitial Cluster Size = " << props["maxIClusterSize"]);
	BOOST_REQUIRE(strtol(props["maxIClusterSize"].c_str(),NULL,10) == 1);
	BOOST_TEST_MESSAGE("Number of He clusters = " << props["numHeClusters"]);
	BOOST_REQUIRE(strtol(props["numHeClusters"].c_str(),NULL,10) == 1);
	BOOST_TEST_MESSAGE("Number of V clusters = " << props["numVClusters"]);
	BOOST_REQUIRE(strtol(props["numVClusters"].c_str(),NULL,10) == 1);
	BOOST_TEST_MESSAGE("Number of I clusters = " << props["numIClusters"]);
	BOOST_REQUIRE(strtol(props["numIClusters"].c_str(),NULL,10) == 1);
	BOOST_TEST_MESSAGE("Number of mixed clusters = " << props["numMixedClusters"]);
	BOOST_REQUIRE(strtol(props["numMixedClusters"].c_str(),NULL,10) == 1);
//
//	shared_ptr<stringstream> networkStream(
//			new stringstream(stringstream::in | stringstream::out));
//	string singleHeString = "1 0 0 0.0 Infinity Infinity 8.269996 0.999 1.34\n";
//	string singleVString =
//			"0 50 0 Infinity 2.49000002 Infinity Infinity 0.888 2.345\n";
//	string singleIString = "0 0 1 Infinity Infinity Infinity Infinity 0.7777 3.456\n";
//	string mixedString =
//			"1 50 0 6.160001 2.4900002 Infinity Infinity 6.789 4.5678\n";

	// Check the reactants - He first
	std::shared_ptr<PSICluster> heCluster = static_pointer_cast<PSICluster>(reactants.at(0));
	BOOST_REQUIRE(heCluster->getSize() == 1);
	std::vector<double> bindingEnergies = heCluster->getBindingEnergies();
	BOOST_REQUIRE_CLOSE(bindingEnergies.at(0), 0.0, 0.00);
	BOOST_REQUIRE(bindingEnergies.at(1) == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE(bindingEnergies.at(2) == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE_CLOSE(bindingEnergies.at(3), 8.2699999, 0.0001);
	BOOST_REQUIRE_CLOSE(heCluster->getMigrationEnergy(),0.999,0.001);
	BOOST_REQUIRE_CLOSE(heCluster->getDiffusionFactor(),1.34,0.01);
	// V
	std::shared_ptr<PSICluster> vCluster = static_pointer_cast<PSICluster>(reactants.at(1));
	BOOST_REQUIRE(vCluster->getSize() == 50);
	bindingEnergies.clear();
	bindingEnergies = vCluster->getBindingEnergies();
	BOOST_REQUIRE(bindingEnergies[0] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE_CLOSE(bindingEnergies[1], 2.4900, 0.0001);
	BOOST_REQUIRE(bindingEnergies[2] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE(bindingEnergies[3] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE_CLOSE(vCluster->getMigrationEnergy(),0.888,0.001);
	BOOST_REQUIRE_CLOSE(vCluster->getDiffusionFactor(),2.345,0.001);
	// I
	std::shared_ptr<PSICluster> iCluster = static_pointer_cast<PSICluster>(reactants.at(2));
	BOOST_REQUIRE(iCluster->getSize() == 1);
	bindingEnergies.clear();
	bindingEnergies = iCluster->getBindingEnergies();
	BOOST_REQUIRE(bindingEnergies[0] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE(bindingEnergies[1] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE(bindingEnergies[2] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE(bindingEnergies[3] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE_CLOSE(iCluster->getMigrationEnergy(),0.7777,0.0001);
	BOOST_REQUIRE_CLOSE(iCluster->getDiffusionFactor(),3.456,0.001);
	// Mixed
	std::shared_ptr<PSICluster> mixedCluster = static_pointer_cast<PSICluster>(reactants.at(3));
	BOOST_REQUIRE(mixedCluster->getSize() == 51);
	bindingEnergies.clear();
	bindingEnergies = mixedCluster->getBindingEnergies();
	BOOST_REQUIRE_CLOSE(bindingEnergies[0], 6.160001, 0.00001);
	BOOST_REQUIRE_CLOSE(bindingEnergies[1], 2.4900, 0.0001);
	BOOST_REQUIRE(bindingEnergies[2] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE(bindingEnergies[3] == std::numeric_limits<double>::infinity());
	BOOST_REQUIRE_CLOSE(mixedCluster->getMigrationEnergy(),6.789,0.001);
	BOOST_REQUIRE_CLOSE(mixedCluster->getDiffusionFactor(),4.5678,0.0001);

	return;
}
BOOST_AUTO_TEST_SUITE_END()
