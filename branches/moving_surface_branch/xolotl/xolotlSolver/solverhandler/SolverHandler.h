#ifndef SOLVERHANDLER_H
#define SOLVERHANDLER_H

// Includes
#include "ISolverHandler.h"

namespace xolotlSolver {

/**
 * This class and its subclasses realize the ISolverHandler interface to solve the
 * advection-diffusion-reaction problem with currently supported solvers.
 */
class SolverHandler: public ISolverHandler {
protected:

	//! The name of the network file
	std::string networkName;

	//! The original network created from the network loader.
	xolotlCore::PSIClusterReactionNetwork *network;

	//! The grid step size.
	double h;

	//! The initial vacancy concentration.
	double initialVConc;

	//! The original flux handler created.
	xolotlCore::IFluxHandler *fluxHandler;

	//! The original temperature handler created.
	xolotlCore::ITemperatureHandler *temperatureHandler;

	//! The original diffusion handler created.
	xolotlCore::IDiffusionHandler *diffusionHandler;

	//! The original advection handler created.
	xolotlCore::IAdvectionHandler *advectionHandler;

	//! The number of dimensions for the problem.
	int dimension;

	//! The portion of void at the beginning of the problem.
	double portion;

	//! To know if the surface has moved.
	bool surfaceHasMoved;

public:

	/**
	 * Initialize all the physics handlers that are needed to solve the ADR equations.
     * \see ISolverHandler.h
	 */
	void initializeHandlers(std::shared_ptr<xolotlFactory::IMaterialFactory> material,
			std::shared_ptr<xolotlCore::ITemperatureHandler> tempHandler,
			xolotlCore::Options &options) {

		// Set the flux handler
		fluxHandler = (xolotlCore::IFluxHandler *) material->getFluxHandler().get();

		// Set the temperature handler
		temperatureHandler = (xolotlCore::ITemperatureHandler *) tempHandler.get();

		// Set the diffusion handler
		diffusionHandler = (xolotlCore::IDiffusionHandler *) material->getDiffusionHandler().get();

		// Set the advection handler
		advectionHandler = (xolotlCore::IAdvectionHandler *) material->getAdvectionHandler().get();

		// Set the initial vacancy concentration
		initialVConc = options.getInitialVConcentration();

		// Set the number of dimension
		dimension = options.getDimensionNumber();

		// Set the void portion
		portion = options.getVoidPortion();

		return;
	}

	/**
	 * Initialize the network and network file name.
	 * \see ISolverHandler.h
	 */
	void initializeNetwork(std::string fileName,
			xolotlCore::PSIClusterReactionNetwork *net) {

		// Set the network loader
		networkName = fileName;

		// Set the network
		network = net;

		return;
	}

	/**
	 * Get the step size.
	 * \see ISolverHandler.h
	 */
	double getStepSize() const {return h;}

	/**
	 * Get the number of dimensions of the problem.
	 * \see ISolverHandler.h
	 */
	int getDimension() const {return dimension;}

	/**
	 * If the surface is moving, the corresponding boolean will be set
	 * to true.
	 */
	void changeSurfacePosition() {surfaceHasMoved = true;}

	/**
	 * Get the flux handler.
	 * \see ISolverHandler.h
	 */
	xolotlCore::IFluxHandler *getFluxHandler() const {return fluxHandler;}

	/**
	 * Get the network.
	 * \see ISolverHandler.h
	 */
	xolotlCore::PSIClusterReactionNetwork *getNetwork() const {return network;}

}; //end class SolverHandler

} /* end namespace xolotlSolver */
#endif
