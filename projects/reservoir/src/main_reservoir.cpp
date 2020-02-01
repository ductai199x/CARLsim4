#include <carlsim.h>
#include <visual_stimulus.h>

#include <assert.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <memory>
#include <random>

#include <reservoir.h>

class TimeToFirstSpike : public SpikeGenerator
{
	public:

		TimeToFirstSpike(vector<float>* spikeVector, int max_intensity) 
		{
			spikeVector_ = spikeVector;
			max_intensity_ = max_intensity + 1;

			isSpikedArr_ = std::unique_ptr<int>(new int[(*spikeVector_).size()]);
		}

		~TimeToFirstSpike() {};

		void newSpikeVector(vector<float>* spikeVector);
	    int nextSpikeTime(CARLsim*s, int grpId, int nId, int currentTime, int lastScheduledSpikeTime, int endOfTimeSlice);
    
    private:

        vector<float>* spikeVector_;
		std::unique_ptr<int> isSpikedArr_;
		int max_intensity_;
		bool isRefSpike_;

		
};

void TimeToFirstSpike::newSpikeVector(vector<float>* spikeVector) 
{
	spikeVector_ = spikeVector;
	memset(isSpikedArr_.get(), 0, (*spikeVector_).size()*sizeof(*isSpikedArr_.get()));
}

int TimeToFirstSpike::nextSpikeTime(CARLsim*s, int grpId, int nId, int currentTime, int lastScheduledSpikeTime, int endOfTimeSlice)
{
	if (isSpikedArr_.get()[nId] == 0)
	{
		isSpikedArr_.get()[nId] = 1;
		int scheduleTime = currentTime + (endOfTimeSlice - currentTime)/(max_intensity_)*(max_intensity_ - (*spikeVector_)[nId]);
		// cout << (*spikeVector_)[nId] << endl;
		return scheduleTime;
	}
	else
	{ 
		return endOfTimeSlice;
	}
	
}

int main() {
		// ---------------- CONFIG STATE -------------------
	int randSeed = 15;
	CARLsim sim("reservoir", CPU_MODE, USER, 1, randSeed);


	int n = 10;
	int m = 2;

	std::uniform_real_distribution<float> distribution(0, 255);
	std::default_random_engine generator;
	generator.seed(100);
	vector<vector<float>> coordinates;
	for (int i = 0; i < n; i++) {
		vector<float> r { distribution(generator), distribution(generator) };
		coordinates.push_back(r);
	}

	int num_resv_neurons = 100;
	// int num_resv_neurons = 100;
	int gSpikeGen = sim.createSpikeGeneratorGroup("input", m, EXCITATORY_NEURON);
	int gResvOut = sim.createGroupReservoirOutput("output", m, EXCITATORY_NEURON);

	TimeToFirstSpike ttfs(&coordinates[0], 255);
	sim.setSpikeGenerator(gSpikeGen, &ttfs);

	SpikeMonitor* spkMonSpkGen = sim.setSpikeMonitor(gSpikeGen, "DEFAULT");
	SpikeMonitor* spkMonResvOutput = sim.setSpikeMonitor(gResvOut, "DEFAULT");
	
	Reservoir resv(&sim, "test", num_resv_neurons, 1, 0.01, randSeed, gSpikeGen, gResvOut);
	resv.create();
	resv.connectToReservoir();

	sim.setConductances(false);
	sim.setupNetwork();

	resv.setupMonitors();
	resv.startMonitors();

	spkMonSpkGen->startRecording();
	spkMonResvOutput->startRecording();
	
	
	sim.runNetwork(1,0);
	for (int i=1; i<n; i++) {
		ttfs.newSpikeVector(&coordinates[i]);
		sim.runNetwork(0,300);
	}
	
	resv.stopMonitors();

	spkMonSpkGen->stopRecording();
	spkMonResvOutput->stopRecording();

	// spkMonResvOutput->print();
	spkMonSpkGen->print();

	// resv.getSpkMon(0)->print();
	// resv.getSpkMon(1)->print();

	return 0;
}
