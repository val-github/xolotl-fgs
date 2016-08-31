// Includes
#include "VCluster.h"
#include <iostream>
#include <Constants.h>
#include <PSIClusterReactionNetwork.h>

using namespace xolotlCore;

VCluster::VCluster(int nV, std::shared_ptr<xolotlPerf::IHandlerRegistry> registry) :
		PSICluster(nV, registry) {
	// Set the reactant name appropriately
	std::stringstream nameStream;
	nameStream << "V_" << size;
	name = nameStream.str();
	// Set the typename appropriately
	typeName = "V";

	// Update the composition map
	compositionMap["V"] = size;

	// Compute the reaction radius
	// It is the same formula for HeV clusters
	reactionRadius =
			reactionRadius = pow(size, (1.0 / 3.0)) * 0.126;
}

std::shared_ptr<Reactant> VCluster::clone() {
	std::shared_ptr<Reactant> reactant(new VCluster(*this));
	return reactant;
}

void VCluster::createReactionConnectivity() {
	// Call the function from the PSICluster class to take care of the single
	// species reaction
	PSICluster::createReactionConnectivity();

	// This cluster is always V_a

	// Helium-Vacancy clustering
	// He_b + V_a --> (He_b)(V_a)
	// Get all the He clusters from the network
	auto reactants = network->getAll(heType);
	// combineClusters handles He combining with V to form HeV
	combineClusters(reactants, heVType);

	// Single Vacancy absorption by HeV clusters
	// (He_c)(V_b) + V_a --> (He_c)[V_(b+a)]
	// for a = 1
	// Only if the size of this cluster is 1
	if (size == 1) {
		// Get all the HeV clusters from the network
		reactants = network->getAll(heVType);
		// combineClusters handles HeV combining with V to form HeV
		combineClusters(reactants, heVType);
	}

	// Vacancy-Interstitial annihilation
	// I_a + V_b
	//        --> I_(a-b), if a > b
	//        --> V_(b-a), if a < b
	//        --> 0, if a = b
	// Get all the I clusters from the network
	reactants = network->getAll(iType);
	// fillVWithI handles this reaction
	fillVWithI(reactants);

	// Vacancy-Interstitial annihilation producing this cluster
	// I_b + V_(a+b) --> V_a
	// All the I clusters are already in reactants
	int reactantsSize = reactants.size();
	for (int i = 0; i < reactantsSize; i++) {
		auto firstReactant = (PSICluster *) reactants[i];
		// Get the vacancy cluster that is bigger than the interstitial
		// and can form this cluster.
		auto secondReactant = (PSICluster *) network->get(typeName, firstReactant->getSize() + size);
		// Add to the reacting pairs if the second reactant exists
		if (secondReactant) {
			// Create the pair
			// The reaction constant will be computed later, it is set to 0.0 for now
			ClusterPair pair(firstReactant, secondReactant, 0.0);
			// Add the pair to the list
			reactingPairs.push_back(pair);
			// Setup the connectivity array
			int Id = firstReactant->getId();
			setReactionConnectivity(Id);
			Id = secondReactant->getId();
			setReactionConnectivity(Id);
		}
	}

	// Interstitial reduction by Vacancy absorption in HeI clusters
	// (He_c)*(I_b) + (V_a) --> (He_c)*[I_(b-a)]
	// Get all the HeI clusters from the network
	reactants = network->getAll(heIType);
	// replaceInCompound handles this reaction
	replaceInCompound(reactants, iType);

	return;
}

void VCluster::createDissociationConnectivity() {
	// Call the function from the PSICluster class to take care of the single
	// species dissociation
	PSICluster::createDissociationConnectivity();

	// This cluster is always V_a

	// He Dissociation
	// (He_1)(V_a) --> V_a + He
	// Get the cluster with one more helium
	std::vector<int> compositionVec = { 1, size, 0 };
	auto heVClusterMoreHe = (PSICluster *) network->getCompound(heVType, compositionVec);
	// Get the single helium cluster
	auto singleCluster = (PSICluster *) network->get(heType, 1);
	// Here it is important that heVClusterMoreHe is the first cluster
	// because it is the dissociating one.
	dissociateCluster(heVClusterMoreHe, singleCluster);

	// Specific case for the single species cluster
	if (size == 1) {
		// V dissociation of HeV cluster is handled here
		// (He_c)(V_b) --> (He_c)[V_(b-a)] + V_a
		// for a = 1
		// Get all the HeV clusters of the network
		auto allHeVReactants = network->getAll(heVType);
		for (unsigned int i = 0; i < allHeVReactants.size(); i++) {
			auto cluster = (PSICluster *) allHeVReactants[i];

			// (He_c)(V_b) is the dissociating one, (He_c)[V_(b-a)] is the one
			// that is also emitted during the dissociation
			auto comp = cluster->getComposition();
			std::vector<int> compositionVec = { comp[heType], comp[vType] - size,
					0 };
			auto smallerReactant = (PSICluster *) network->getCompound(heVType, compositionVec);
			dissociateCluster(cluster, smallerReactant);
		}
	}

	return;
}

double VCluster::getEmissionFlux() const {
	// Initial declarations
	double flux = PSICluster::getEmissionFlux();

	// Compute the loss to dislocation sinks
	if (size == 1) {
		// Compute the thermal equilibrium density
		// rho_fe exp((-E_V / (k T)))
		double concEq = 84.6 * exp(-2.07/(xolotlCore::kBoltzmann * temperature));
		// rho_dis * D * (C - C_eq)
		flux += 0.0001 * diffusionCoefficient * (concentration - concEq);
	}

	return flux;
}

void VCluster::getEmissionPartialDerivatives(std::vector<double> & partials) const {
	// Initial declarations
	PSICluster::getEmissionPartialDerivatives(partials);

	// Compute the loss to dislocation sinks
	if (size == 1) {
		// bias * rho_dis * D * C
		partials[id -1] -= 0.0001 * diffusionCoefficient;
	}

	return;
}
