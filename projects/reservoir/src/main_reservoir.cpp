/* * Copyright (c) 2016 Regents of the University of California. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. The names of its contributors may not be used to endorse or promote
*    products derived from this software without specific prior written
*    permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* *********************************************************************************************** *
* CARLsim
* created by: (MDR) Micah Richert, (JN) Jayram M. Nageswaran
* maintained by:
* (MA) Mike Avery <averym@uci.edu>
* (MB) Michael Beyeler <mbeyeler@uci.edu>,
* (KDC) Kristofor Carlson <kdcarlso@uci.edu>
* (TSC) Ting-Shuo Chou <tingshuc@uci.edu>
* (HK) Hirak J Kashyap <kashyaph@uci.edu>
*
* CARLsim v1.0: JM, MDR
* CARLsim v2.0/v2.1/v2.2: JM, MDR, MA, MB, KDC
* CARLsim3: MB, KDC, TSC
* CARLsim4: TSC, HK
*
* CARLsim available from http://socsci.uci.edu/~jkrichma/CARLsim/
* Ver 12/31/2016
*/

// include CARLsim user interface
#include <carlsim.h>

// include stopwatch for timing
#include <stopwatch.h>

#include <reservoir.h>

int main() {
	// keep track of execution time
	Stopwatch watch;
	

	// ---------------- CONFIG STATE -------------------
	
	// create a network on GPU
	int numGPUs = 1;
	int randSeed = 10;
	CARLsim sim("reservoir", GPU_MODE, USER, numGPUs, randSeed);


	int num_resv_neurons = 11;
	int gSpikeGen = sim.createSpikeGeneratorGroup("input", num_resv_neurons, EXCITATORY_NEURON);
	
	Reservoir resv(&sim, "test", num_resv_neurons, 0.5, 0.2, randSeed, gSpikeGen);
	resv.create();
	resv.connectToReservoir();

	sim.setConductances(false);
	sim.setupNetwork();
	
	resv.randomizeWeights();

	resv.setupMonitors();
	resv.startMonitors();

	PoissonRate in(num_resv_neurons);
	in.setRates(30.0f);
	sim.setSpikeRate(gSpikeGen, &in);

	for (int i=0; i<2; i++) {
		sim.runNetwork(2,0);
	}

	resv.stopMonitors();

	resv.getSpkMon(0)->print();
	resv.getSpkMon(1)->print();

	// sim.connect(gin, gout, "gaussian", RangeWeight(0.05), 1.0f, RangeDelay(1), RadiusRF(3,3,1));
	
	// // sim.setIntegrationMethod(FORWARD_EULER, 2);

	// // ---------------- SETUP STATE -------------------
	// // build the network
	// watch.lap("setupNetwork");
	// 

	// // set some monitors
	// sim.setSpikeMonitor(gin,"DEFAULT");
	// SpikeMonitor* spkMon = sim.setSpikeMonitor(gout,"DEFAULT");
	// sim.setConnectionMonitor(gin,gout,"DEFAULT");




	// // ---------------- RUN STATE -------------------
	// watch.lap("runNetwork");

	// spkMon->startRecording();
	// // run for a total of 10 seconds
	// // at the end of each runNetwork call, SpikeMonitor stats will be printed
	// for (int i=0; i<10; i++) {
	// 	sim.runNetwork(1,0);
	// }

	// spkMon->stopRecording();
	// spkMon->print();
	// // print stopwatch summary
	// watch.stop();
	
	return 0;
}
