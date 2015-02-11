// Includes
#include "Diffusion3DHandler.h"

namespace xolotlCore {

void Diffusion3DHandler::computeDiffusion(PSIClusterReactionNetwork *network,
		double **concVector, double *updatedConcOffset,
		double hxLeft, double hxRight, double sy, double sz) {
	// Get all the reactant
	auto reactants = network->getAll();
	// Get the number of diffusing cluster
	int nDiff = indexVector.size();

	// Get the number of degrees of freedom which is the size of the network
	int dof = reactants->size();

	// Loop on them
	for (int i = 0; i < nDiff; i++) {
		// Get the diffusing cluster
		auto cluster = (PSICluster *) reactants->at(indexVector[i]);
		// Get the index of the cluster
		int index = cluster->getId() - 1;

		// Get the initial concentrations
		double oldConc = concVector[0][index]; // middle
		double oldLeftConc = concVector[1][index]; // left
		double oldRightConc = concVector[2][index]; // right
		double oldBottomConc = concVector[3][index]; // bottom
		double oldTopConc = concVector[4][index]; // top
		double oldFrontConc = concVector[5][index]; // front
		double oldBackConc = concVector[6][index]; // back

		// Use a simple midpoint stencil to compute the concentration
		double conc = cluster->getDiffusionCoefficient()
						* (2.0 * (oldLeftConc + (hxLeft / hxRight) * oldRightConc
										- (1.0 + (hxLeft / hxRight)) * oldConc)
								/ (hxLeft * (hxLeft + hxRight))
								+ sy * (oldBottomConc + oldTopConc - 2.0 * oldConc)
								+ sz * (oldFrontConc + oldBackConc - 2.0 * oldConc));

		// Update the concentration of the cluster
		updatedConcOffset[index] += conc;
	}

	return;
}

void Diffusion3DHandler::computePartialsForDiffusion(
		PSIClusterReactionNetwork *network,
		double *val, int *indices, double hxLeft, double hxRight,
		double sy, double sz) {
	// Get all the reactant
	auto reactants = network->getAll();
	// And the size of the network
	int size = reactants->size();
	// Get the number of diffusing cluster
	int nDiff = indexVector.size();

	// Loop on them
	for (int i = 0; i < nDiff; i++) {
		// Get the diffusing cluster
		auto cluster = (PSICluster *) reactants->at(indexVector[i]);
		// Get the index of the cluster
		int index = cluster->getId() - 1;
		// Get the diffusion coefficient of the cluster
		double diffCoeff = cluster->getDiffusionCoefficient();

		// Set the cluster index, the PetscSolver will use it to compute
		// the row and column indices for the Jacobian
		indices[i] = index;

		// Compute the partial derivatives for diffusion of this cluster
		// for the middle, left, right, bottom, top, front, and back grid point
		val[i * 7] = - 2.0 * diffCoeff * ((1.0 / (hxLeft * hxRight)) + sy + sz); // middle
		val[(i * 7) + 1] = diffCoeff * 2.0 / (hxLeft * (hxLeft + hxRight)); // left
		val[(i * 7) + 2] = diffCoeff * 2.0 / (hxRight * (hxLeft + hxRight)); // right
		val[(i * 7) + 3] = diffCoeff * sy; // bottom
		val[(i * 7) + 4] = diffCoeff * sy; // top
		val[(i * 7) + 5] = diffCoeff * sz; // front
		val[(i * 7) + 6] = diffCoeff * sz; // back
	}

	return;
}

}/* end namespace xolotlCore */
