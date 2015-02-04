// Includes
#include "AdvectionHandler.h"

namespace xolotlCore {

void AdvectionHandler::computeAdvection(PSIClusterReactionNetwork *network,
		double h, std::vector<double> &pos, int surfacePos, double **concVector,
		double *updatedConcOffset) {
	// Get all the reactant
	auto reactants = network->getAll();
	// Get the number of advecting cluster
	int nAdvec = indexVector.size();

	// Compute the depth from the surface
	double depth = pos[0] - (double) surfacePos * h;

	// Get the number of degrees of freedom which is the size of the network
	int dof = reactants->size();

	// Loop on them
	for (int i = 0; i < nAdvec; i++) {
		// Get the advecting cluster
		auto cluster = (PSICluster *) reactants->at(indexVector[i]);
		// Get the index of the cluster
		int index = cluster->getId() - 1;

		// Get the initial concentrations
		double oldConc = concVector[0][index]; // middle
		double oldRightConc = concVector[2][index]; // right

		// Compute the concentration as explained in the description of the method
		double conc = (3.0 * sinkStrengthVector[i] * cluster->getDiffusionCoefficient())
				/ (xolotlCore::kBoltzmann * cluster->getTemperature() * h)
				* ((oldRightConc / pow(depth + h, 4)) - (oldConc / pow(depth, 4)));

		// Update the concentration of the cluster
		updatedConcOffset[index] += conc;
	}

	return;
}

void AdvectionHandler::computePartialsForAdvection(
		PSIClusterReactionNetwork *network, double h, double *val,
		int *indices, std::vector<double> &pos, int surfacePos) {
	// Get all the reactant
	auto reactants = network->getAll();
	// And the size of the network
	int size = reactants->size();
	// Get the number of diffusing cluster
	int nAdvec = indexVector.size();

	// Compute the depth from the surface
	double depth = pos[0] - (double) surfacePos * h;

	// Loop on them
	for (int i = 0; i < nAdvec; i++) {
		// Get the diffusing cluster
		auto cluster = (PSICluster *) reactants->at(indexVector[i]);
		// Get the index of the cluster
		int index = cluster->getId() - 1;
		// Get the diffusion coefficient of the cluster
		double diffCoeff = cluster->getDiffusionCoefficient();
		// Get the sink strenght value
		double sinkStrength = sinkStrengthVector[i];

		// Set the cluster index that will be used by PetscSolver
		// to compute the row and column indices for the Jacobian
		indices[i] = index;

		// Compute the partial derivatives for advection of this cluster as
		// explained in the description of this method
		val[i * 2] = -(3.0 * sinkStrength * diffCoeff)
						/ (xolotlCore::kBoltzmann * cluster->getTemperature()
								* h * pow(depth, 4)); // middle
		val[(i * 2) + 1] = (3.0 * sinkStrength * diffCoeff)
								/ (xolotlCore::kBoltzmann * cluster->getTemperature()
										* h * pow(depth + h, 4)); // right
	}

	return;
}

}/* end namespace xolotlCore */
