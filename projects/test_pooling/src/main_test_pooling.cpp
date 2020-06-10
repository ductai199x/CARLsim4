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
#include <visual_stimulus.h>
#include <spikegen_from_vector.h>

#include <vector>
#include <cassert>
#include <random>
#include <string>
#include <iostream>
#include <dirent.h>

int main(int argc, const char* argv[])
{

	// ---------------- CONFIG STATE -------------------
	CARLsim sim("smooth", CPU_MODE, DEVELOPER, 1, 123);

	// ----- INPUT LAYER INITIALIZATION -----
	Grid3D inDim(3, 3, 1);
	int gIn = sim.createSpikeGeneratorGroup("input", inDim, EXCITATORY_NEURON);
	// std::vector<int> spikeTimes = {2, 5, 6, 11, 14, 100, 101, 102, 103, 203, 204};
	std::vector<int> spikeTimes = {2, 5, 6, 11, 14, 100, 101};
	SpikeGeneratorFromVector SGV(spikeTimes);
	sim.setSpikeGenerator(gIn, &SGV);
	sim.setSpikeMonitor(gIn, "DEFAULT");

	int numFeatureMaps = 1;
	// Random number generator (from gaussian dist)
	std::default_random_engine generator (0);
	std::normal_distribution<double> distribution (20.0,0.33);
	double gaus_rand = distribution(generator);
	// ----- CONVOLUTIONAL LAYER INITIALIZATION -----
	Grid3D convDim(2, 2, 1);
	// LIF Parameters Initialization
    int tau_mE = 10;
    int tau_refE = 1;
    float vTh = -78.0f;
    float vReset = -80.0f;
    float vInit = -80.0f;
    float rMem = 10;

	int *convLayers = (int*)malloc(sizeof(int)*numFeatureMaps);
	int *inputToConvIDs = (int*)malloc(sizeof(int)*numFeatureMaps);

	// Create Convolutional Layers
	SpikeMonitor* spkMonConv;
	for (int i=0; i<numFeatureMaps; i++) {
		convLayers[i] = sim.createGroupLIF("convolutional", convDim, EXCITATORY_NEURON);
		sim.setNeuronParametersLIF(convLayers[i], (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));
		spkMonConv = sim.setSpikeMonitor(convLayers[i], "DEFAULT");
	}

	// ----- POOLING LAYER INITIALIZATION -----
	int numConvFilters = 2;
	Grid3D numNeuronPoolingLayer(numConvFilters,1,1);
	int *poolingLayers = (int*)malloc(sizeof(int)*numFeatureMaps);
	int *convToPoolingIDs = (int*)malloc(sizeof(int)*numFeatureMaps);

	// Create Pooling Layers
	SpikeMonitor* spkMon;
	for (int i=0; i<numFeatureMaps; i++) {
		poolingLayers[i] = sim.createGroupPoolingMaxRate("pooling", numNeuronPoolingLayer, EXCITATORY_NEURON);
		sim.setNeuronParametersPoolingMaxRate(poolingLayers[i], (int)tau_mE, (int)tau_refE, (float)vTh-1, (float)vReset, RangeRmem(rMem));
		spkMon = sim.setSpikeMonitor(poolingLayers[i], "DEFAULT");
	}

	// ----- CONNECT ALL THE LAYERS -----
	// Connect Input Layer to ALL Convolutional Layers
	for (int i=0; i<numFeatureMaps; i++) {
		gaus_rand = distribution(generator);
		inputToConvIDs[i] = sim.connect(gIn, convLayers[i], "gaussian", RangeWeight(0, gaus_rand, gaus_rand), 1.0f,
			RangeDelay(1), RadiusRF(2,2,-1), SYN_PLASTIC);
	}

	// Connect EACH Convolutional Layer to EACH Max Pooling Layer
	for (int i=0; i<numFeatureMaps; i++) {
		gaus_rand = distribution(generator);
		convToPoolingIDs[i] = sim.connect(convLayers[i], poolingLayers[i], "gaussian", RangeWeight(0, gaus_rand, gaus_rand), 1.0f, RangeDelay(1), RadiusRF(1,1,-1), SYN_PLASTIC);
		// convToPoolingIDs[i] = sim.connect(convLayers[i], poolingLayers[i], poolConn, SYN_PLASTIC);
	}

	// Use CUBA mode
	sim.setConductances(false);

	// ---------------- SETUP STATE -------------------
	sim.setupNetwork();

	// start recording spikes associated with spkMon object
	spkMon->startRecording();
	spkMonConv->startRecording();

	// ---------------- RUN STATE -------------------
	sim.runNetwork(1,0); // run the network

	spkMon->stopRecording();
	spkMonConv->stopRecording();
	// print a summary of the spike information""
	spkMonConv->print();
	spkMon->print();
	// get the average firing rate of each of the neurons in group excGrpId
	std::vector<float> excFRs = spkMon->getAllFiringRates();

	int max = 0;
	std::vector<float>::iterator it = excFRs.begin();
	for (it; it != excFRs.end(); it++) {
		if (*it > max){
			max = *it;
		}
	}
	printf("max: %d", max);
	
	return 0;
}