#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Regression

#include <boost/test/included/unit_test.hpp>
#include <DummyAdvectionHandler.h>
#include <HDF5NetworkLoader.h>
#include <XolotlConfig.h>
#include <DummyHandlerRegistry.h>
#include <mpi.h>

using namespace std;
using namespace xolotlCore;

/**
 * This suite is responsible for testing the DummyAdvectionHandler.
 */
BOOST_AUTO_TEST_SUITE(DummyAdvectionHandler_testSuite)

/**
 * Method checking the initialization and the compute advection methods.
 */
BOOST_AUTO_TEST_CASE(checkAdvection) {
	// Initialize MPI for HDF5
	int argc = 0;
	char **argv;
	MPI_Init(&argc, &argv);

	// Create the network loader
	HDF5NetworkLoader loader =
			HDF5NetworkLoader(make_shared<xolotlPerf::DummyHandlerRegistry>());
	// Define the filename to load the network from
	string sourceDir(XolotlSourceDirectory);
	string pathToFile("/tests/testfiles/tungsten_diminutive.h5");
	string filename = sourceDir + pathToFile;
	// Give the filename to the network loader
	loader.setFilename(filename);

	// Load the network
	auto network = (PSIClusterReactionNetwork *) loader.load().get();
	// Get its size
	const int size = network->getAll()->size();

	// Create the advection handler
	DummyAdvectionHandler advectionHandler;

	// Create ofill
	int mat[size*size];
	int *ofill = &mat[0];

	// Initialize it
	advectionHandler.initialize(network, ofill);

	// Check the total number of advecting clusters, it should be 0 here
	BOOST_REQUIRE_EQUAL(advectionHandler.getNumberOfAdvecting(), 0);

	// The size parameter
	double hx = 1.0;

	// The arrays of concentration
	double concentration[3*size];
	double newConcentration[3*size];

	// Initialize their values
	for (int i = 0; i < 3*size; i++) {
		concentration[i] = (double) i * i;
		newConcentration[i] = 0.0;
	}

	// Set the temperature to 1000 K to initialize the diffusion coefficients
	auto reactants = network->getAll();
	for (int i = 0; i < size; i++) {
		auto cluster = (PSICluster *) reactants->at(i);
		cluster->setTemperature(1000.0);
	}

	// Get pointers
	double *conc = &concentration[0];
	double *updatedConc = &newConcentration[0];

	// Get the offset for the grid point in the middle
	double *concOffset = conc + size;
	double *updatedConcOffset = updatedConc + size;

	// Fill the concVector with the pointer to the middle, left, and right grid points
	double **concVector = new double*[3];
	concVector[0] = concOffset; // middle
	concVector[1] = conc; // left
	concVector[2] = conc + 2 * size; // right

	// Set the grid position
	std::vector<double> gridPosition = { hx, 0, 0 };

	// Compute the advection at this grid point
	advectionHandler.computeAdvection(network, gridPosition,
			concVector, updatedConcOffset, hx, hx);

	// Check the new values of updatedConcOffset
	BOOST_REQUIRE_CLOSE(updatedConcOffset[0], 0.0, 0.01); // Does not advect
	BOOST_REQUIRE_CLOSE(updatedConcOffset[1], 0.0, 0.01); // Does not advect
	BOOST_REQUIRE_CLOSE(updatedConcOffset[2], 0.0, 0.01); // Does not advect
	BOOST_REQUIRE_CLOSE(updatedConcOffset[3], 0.0, 0.01); // Does not advect
	BOOST_REQUIRE_CLOSE(updatedConcOffset[4], 0.0, 0.01); // Does not advect

	// Don't even test the Jacobian because there is no advecting cluster

	// Finalize MPI
	MPI_Finalize();

	return;
}

BOOST_AUTO_TEST_SUITE_END()
