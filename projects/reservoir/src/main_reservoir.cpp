#include <carlsim.h>
#include <visual_stimulus.h>
#include <timetofirst_spikegen.h>

#include <assert.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <memory>
#include <random>
#include <map>

#include <reservoir.h>

struct LIFNeuronParam_s
{
	int tau_mE;
	int tau_refE;
	float vTh;
	float vReset;
	float vInit;
	float rMem;
}LIFNeuronParam_default = {10, 1, -78.0f, -78.5f, -78.5f, 10};

typedef struct LIFNeuronParam_s LIFNeuronParam;

void generateTargetSpikeFn(SpikeGenerator* spkGen, int* spkArr)
{

}


int main() {
		// ---------------- CONFIG STATE -------------------
	int randSeed = 15;
	CARLsim sim("reservoir", CPU_MODE, USER, 1, randSeed);

	int simTime = 50; // in ms
	int n = 2;
	int m = 2;

	std::uniform_real_distribution<float> distribution(0, 255);
	std::default_random_engine generator;
	generator.seed(100);
	vector<vector<float>> coordinates;
	vector<float> r { distribution(generator), distribution(generator) };                                                      
	for (int i = 0; i < n; i++) {
		
		coordinates.push_back(r);
	}

	int num_resv_neurons = 1000;
	float learning_rate = 10;
	// int num_resv_neurons = 100;
	TimeToFirstSpike ttfs(&coordinates[0], 255);
	int gSpikeGen = sim.createSpikeGeneratorGroup("input", m, EXCITATORY_NEURON);
	int gResvOut = sim.createGroupReservoirOutput("output", m, EXCITATORY_NEURON, &ttfs, num_resv_neurons, learning_rate);

	// int gResvOut = sim.createGroupLIF("output", m, EXCITATORY_NEURON);

	LIFNeuronParam param = LIFNeuronParam_default;
	sim.setNeuronParametersReservoirOutput(gResvOut, (int)param.tau_mE, (int)param.tau_refE, (float)param.vTh, (float)param.vReset, RangeRmem(param.rMem));
	// sim.setNeuronParametersLIF(gResvOut, (int)param.tau_mE, (int)param.tau_refE, (float)param.vTh, (float)param.vReset, RangeRmem(param.rMem));

	
	int* spkArr = new int[simTime];
	// generateTargetSpikeFn(&ttfs, spkArr);


	sim.setReservoirSpikeGenerator(gSpikeGen, &ttfs);

	SpikeMonitor* spkMonSpkGen = sim.setSpikeMonitor(gSpikeGen, "DEFAULT");
	SpikeMonitor* spkMonResvOutput = sim.setSpikeMonitor(gResvOut, "DEFAULT");
	
	Reservoir resv(&sim, "test", num_resv_neurons, 1, 0.1, gSpikeGen, gResvOut);
	resv.create();
	resv.connectToReservoir();

	sim.setConductances(false);
	sim.setupNetwork();

	resv.setupMonitors();
	// resv.startMonitors();

	// spkMonSpkGen->startRecording();
	// spkMonResvOutput->startRecording();
	
	for (int i=0; i<n; i++) {
		ttfs.newSpikeVector(&coordinates[i]);
		sim.runNetwork(0, simTime);
	}
	
	// resv.stopMonitors();

	// spkMonSpkGen->stopRecording();
	// spkMonResvOutput->stopRecording();

	// spkMonResvOutput->print();
	// spkMonSpkGen->print();

	// resv.getSpkMon(0)->print();
	// resv.getSpkMon(1)->print();

	return 0;
}
