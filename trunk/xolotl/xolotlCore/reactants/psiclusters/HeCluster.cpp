// Includes
#include "HeCluster.h"
#include <Constants.h>
#include <iostream>

using namespace xolotlCore;

HeCluster::HeCluster(int nHe) :
		PSICluster(nHe) {
	// Set the reactant name appropriately
	name = "He";
}

HeCluster::~HeCluster() {
}

std::shared_ptr<Reactant> HeCluster::clone() {
	std::shared_ptr<Reactant> reactant(new HeCluster(*this));
	return reactant;
}

void HeCluster::createReactionConnectivity() {

	// Local Declarations - Note the reference to the properties map
	std::map<std::string, std::string> props = network->getProperties();
	int indexOther, otherNumHe, otherNumV, otherNumI, networkSize =
			network->size();
	int maxHeClusterSize = std::stoi(props["maxHeClusterSize"]);
	int maxVClusterSize = std::stoi(props["maxVClusterSize"]);
	int maxHeVClusterSize = std::stoi(props["maxHeVClusterSize"]);
	int maxHeIClusterSize = std::stoi(props["maxHeIClusterSize"]);
	int numHeVClusters = std::stoi(props["numHeVClusters"]);
	int numHeIClusters = std::stoi(props["numHeIClusters"]);
	int numIClusters = std::stoi(props["numIClusters"]);
	int totalSize = 1, firstSize = 0, secondSize = 0;
	int firstIndex = -1, secondIndex = -1, reactantVecSize = 0;
	std::shared_ptr<Reactant> firstReactant, secondReactant, productReactant;
	std::map<std::string, int> composition;
	std::shared_ptr<PSICluster> psiCluster;

	/*
	 * This section fills the array of reacting pairs that combine to produce
	 * this cluster. The only reactions that produce He clusters are those He
	 * clusters that are smaller than this.size. Each cluster i combines with
	 * a second cluster of size this.size - i.size.
	 *
	 * Total size starts with a value of one so that clusters of size one are
	 * not considered in this loop.
	 */
	while (totalSize < size) {
		// Increment the base sizes
		++firstSize;
		secondSize = size - firstSize;
		// Get the first and second reactants for the reaction
		// first + second = this.
		firstReactant = network->get("He", firstSize);
		secondReactant = network->get("He", secondSize);
		// Create a ReactingPair with the two reactants
		if (firstReactant && secondReactant) {
			ReactingPair pair;
			pair.first = std::dynamic_pointer_cast<PSICluster>(firstReactant);
			pair.second = std::dynamic_pointer_cast<PSICluster>(secondReactant);
			// Add the pair to the list
			reactingPairs.push_back(pair);
		}
		// Update the total size. Do not delete this or you'll have an infinite
		// loop!
		totalSize = firstSize + secondSize;
	}

	/* ----- A*He + B*He --> (A+B)*He -----
	 * This cluster should interact with all other clusters of the same type up
	 * to the max size minus the size of this one to produce larger clusters.
	 *
	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 */
	auto reactants = network->getAll("He");
	reactantVecSize = reactants->size();
	for (int i = 0; i < reactantVecSize; i++) {
		// Get the B*He reactant, its composition and id
		firstReactant = reactants->at(i);
		composition = firstReactant->getComposition();
		indexOther = network->getReactantId(*firstReactant) - 1;
		// Get the product, (A+B)*He
		int productSize = size + composition["He"];
		productReactant = network->get("He",productSize);
		// React if the size of the product is valid and it exists in the network
		if (productSize <= maxHeClusterSize && productReactant) {
			// Connect to B*He
			reactionConnectivity[indexOther] = 1;
			// Connect to (A+B)*He
			int indexProduct = network->getReactantId(*productReactant) - 1;
			reactionConnectivity[indexProduct] = 1;
			// Push B*He onto the list of cluster that we combine with
			combiningReactants.push_back(firstReactant);
		}
	}

	/* -----  A*He + B*V --> (A*He)(B*V) -----
	 * Helium clusters can interact with any vacancy cluster so long as the sum
	 * of the number of helium atoms and vacancies does not produce a cluster
	 * with a size greater than the maximum mixed-species cluster size.
	 *
	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 */
	reactants = network->getAll("V");
	reactantVecSize = reactants->size();
	for (int i = 0; i < reactantVecSize; i++) {
		// Get the reactant, its composition and id
		firstReactant = reactants->at(i);
		composition = firstReactant->getComposition();
		indexOther = network->getReactantId(*firstReactant) - 1;
		// React if the size of the product is valid
		if ((size + composition["V"] <= maxHeVClusterSize)) {
			reactionConnectivity[indexOther] = 1;
			combiningReactants.push_back(firstReactant);
		} else
			continue;
	}

	/* ----- A*He + B*I --> (A*He)(B*I)
	 * Helium clusters can interact with any interstitial cluster so long as
	 * the sum of the number of helium atoms and interstitials does not produce
	 * a cluster with a size greater than the maximum mixed-species cluster
	 * size.

	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 */
	reactants = network->getAll("I");
	reactantVecSize = reactants->size();
	for (int i = 0; i < reactantVecSize; i++) {
		// Get the reactant, its composition and id
		firstReactant = reactants->at(i);
		composition = firstReactant->getComposition();
		indexOther = network->getReactantId(*firstReactant) - 1;
		// React if the size of the product is valid
		if ((size + composition["I"] <= maxHeIClusterSize)) {
			reactionConnectivity[indexOther] = 1;
			combiningReactants.push_back(firstReactant);
		}
	}

	/* ----- (A*He)(B*V) + C*He --> [(A+C)He](B*V) -----
	 * Helium can interact with a mixed-species cluster so long as the sum of
	 * the number of helium atoms and the size of the mixed-species cluster
	 * does not exceed the maximum mixed-species cluster size.
	 *
	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 *
	 * Find the clusters by looping over all size combinations of HeV clusters.
	 */
	if (numHeVClusters > 0) {
		reactants = network->getAll("HeV");
		reactantVecSize = reactants->size();
		for (int i = 0; i < reactantVecSize; i++) {
			// Get the reactant, and its id
			firstReactant = reactants->at(i);
			indexOther = network->getReactantId(*firstReactant) - 1;
			// React if the size of the product is valid
			psiCluster = std::dynamic_pointer_cast<PSICluster>(firstReactant);
			//std::cout << "indexOther = " << indexOther << ", i = " << i << ", numHeVClusters = " << numHeVClusters << std::endl; // FIXME!!!
			if ((size + psiCluster->getSize() <= maxHeVClusterSize)) {
				reactionConnectivity[indexOther] = 1;
				combiningReactants.push_back(firstReactant);
			}
		}
	}

	/* ----- (A*He)(B*I) + C*He --> ([A + C]*He)(B*I) -----
	 * Helium-interstitial clusters can absorb single-species helium clusters
	 * so long as the maximum cluster size limit is not violated.
	 *
	 * All of these clusters are added to the set of combining reactants
	 * because they contribute to the flux due to combination reactions.
	 *
	 * Find the clusters by looping over all size combinations of HeI clusters.
	 */
	if (numHeIClusters > 0) {
		reactants = network->getAll("HeI");
		reactantVecSize = reactants->size();
		for (int i = 0; i < reactantVecSize; i++) {
			// Get the reactant and its id
			firstReactant = reactants->at(i);
			indexOther = network->getReactantId(*firstReactant) - 1;
			psiCluster = std::dynamic_pointer_cast<PSICluster>(firstReactant);
			if ((size + psiCluster->getSize()) <= maxHeIClusterSize) {
				reactionConnectivity[indexOther] = 1; // FIXME!
				combiningReactants.push_back(firstReactant);
			}
		}
	}

	return;
}

void HeCluster::createDissociationConnectivity() {

	// Local Declarations
	std::shared_ptr<Reactant> smallerHeReactant, singleHeReactant;

	// He dissociation
	smallerHeReactant = network->get("He", size - 1);
	singleHeReactant = network->get("He", 1);
	if (smallerHeReactant && singleHeReactant) {
		// Add the two reactants to the set. He has very simple dissociation
		// rules.
		id = network->getReactantId(*smallerHeReactant);
		dissociationConnectivity[id] = 1;
		id = network->getReactantId(*singleHeReactant);
		dissociationConnectivity[id] = 1;
	}

	return;
}

bool HeCluster::isProductReactant(const Reactant & reactantI,
		const Reactant & reactantJ) {

	// Local Declarations, integers for species number for I, J reactants
	int rI_I = 0, rJ_I = 0, rI_He = 0, rJ_He = 0, rI_V = 0, rJ_V = 0;

	// Get the compositions of the reactants
	auto reactantIMap = reactantI.getComposition();
	auto reactantJMap = reactantJ.getComposition();

	// Grab the numbers for each species
	// from each Reactant
	rI_I = reactantIMap["I"];
	rJ_I = reactantJMap["I"];
	rI_He = reactantIMap["He"];
	rJ_He = reactantJMap["He"];
	rI_V = reactantIMap["V"];
	rJ_V = reactantJMap["V"];

	// We should have no interstitials, a
	// total of size Helium, and a total of
	// 0 Vacancies
	return ((rI_I + rJ_I) == 0) && ((rI_He + rJ_He) == size)
			&& ((rI_V + rJ_V) == 0);
}

std::map<std::string, int> HeCluster::getClusterMap() {
	// Local Declarations
	std::map<std::string, int> clusterMap;

	// Set the number of each species
	clusterMap["He"] = size;
	clusterMap["V"] = 0;
	clusterMap["I"] = 0;

	// Return it
	return clusterMap;
}

std::map<std::string, int> HeCluster::getComposition() const {
	// Local Declarations
	std::map<std::string, int> clusterMap;

	// Set the number of each species
	clusterMap["He"] = size;
	clusterMap["V"] = 0;
	clusterMap["I"] = 0;

	// Return it
	return clusterMap;
}

double HeCluster::getReactionRadius() {
	double FourPi = 4.0 * xolotlCore::pi;
	double aCubed = pow(xolotlCore::latticeConstant, 3);
	double termOne = pow((3.0 / FourPi) * (1.0 / 10.0) * aCubed * size,
			(1.0 / 3.0));
	double termTwo = pow((3.0 / FourPi) * (1.0 / 10.0) * aCubed, (1.0 / 3.0));
	return 0.3 + termOne - termTwo;
}
