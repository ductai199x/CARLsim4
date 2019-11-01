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

#include <vector>
#include <cassert>
#include <random>

#include <string>
#include <iostream>

#include <dirent.h>

int main()
{
	std::default_random_engine generator (0);
	std::normal_distribution<double> distribution (20.0,0.33);

	// ---------------- CONFIG STATE -------------------
	CARLsim sim("smooth", GPU_MODE, USER);
	
	const char *training_folder = "/home/sweet/2-coursework/ecec487/speech_recognition/src/processed_data/";

	std::vector <std::string> training_files;

    if (auto dir = opendir(training_folder)) {
		while (auto f = readdir(dir)) {
			if (!f->d_name || f->d_name[0] == '.')
				continue; // Skip everything that starts with a dot
			training_files.push_back(std::string(training_folder) + std::string(f->d_name));
		}
		closedir(dir);
	}

	VisualStimulus stim(training_files[0]);
	stim.print();

	Grid3D inDim(stim.getWidth(), stim.getHeight(), stim.getChannels());
	Grid3D outDim(2, 1, 1);

	// Grid3D imgSmallDim(imgDim.numX, imgDim.numY, imgDim.numZ);

	int gIn = sim.createSpikeGeneratorGroup("input", inDim, EXCITATORY_NEURON);

	int numConvLayers = 16;

	int *convLayer = (int*)malloc(sizeof(int)*numConvLayers);
	int *connIDs = (int*)malloc(sizeof(int)*numConvLayers);

	double gaus_rand = 0;
	gaus_rand = distribution(generator);

	// Create Convolutional Layer
	for (int i=0; i<numConvLayers; i++) {
		convLayer[i] = sim.createGroup("convolutional", inDim, EXCITATORY_NEURON);
		sim.setNeuronParameters(convLayer[i], 0.02f, 0.2f, -65.0f, 8.0f);
		sim.setSpikeMonitor(convLayer[i], "DEFAULT");
	}

	// Connect all the layers
	connIDs[0] = sim.connect(gIn, convLayer[0], "one-to-one", RangeWeight(0, gaus_rand, gaus_rand), 1.0f, 
		RangeDelay(1));

	for (int i=0; i<numConvLayers-1; i++) {
		gaus_rand = distribution(generator);
		connIDs[i] = sim.connect(convLayer[i], convLayer[i+1], "gaussian", RangeWeight(0, gaus_rand, gaus_rand), 1.0f,
			RangeDelay(1), RadiusRF(2,2,-1), SYN_PLASTIC);

		if (i > 0) {
			sim.setHomeostasis(connIDs[i], true, 20.0f, 1.0f);
			sim.setHomeoBaseFiringRate(connIDs[i], 20.0f, 0.0f);
			sim.setSTDP(connIDs[i], true);
		}
	}

	// int gOut = sim.createGroup("output", outDim, EXCITATORY_NEURON);
	// sim.setNeuronParameters(gOut, 0.02f, 0.2f, -65.0f, 8.0f);
	// gaus_rand = distribution(generator);
	// connIDs[0] = sim.connect(convLayer[numConvLayers-1], gOut, "full", RangeWeight(0, gaus_rand, gaus_rand), 1.0f,
	// 	RangeDelay(1), RadiusRF(2,2,-1), SYN_PLASTIC);

	// sim.setHomeostasis(gOut, true);
	// sim.setHomeoBaseFiringRate(gOut, 1.0f, 0.0f);
	// sim.setSTDP(gOut, true);

	// Use CUBA mode
	sim.setConductances(false);

	// ---------------- SETUP STATE -------------------
	sim.setupNetwork();

	sim.setSpikeMonitor(gIn, "DEFAULT");
	sim.setSpikeMonitor(convLayer[numConvLayers-1], "DEFAULT");


	// ---------------- RUN STATE -------------------
	// for (int n = 0; n < training_files.size(); n++)
	for (int n = 0; n < 10; n++)
	{
		VisualStimulus stim(training_files[n]);
		for (int i=0; i<stim.getLength(); i++) {
			PoissonRate* rates = stim.readFramePoisson(50.0f, 0.0f);
			sim.setSpikeRate(gIn, rates);
			sim.runNetwork(2,0); // run the network
		}
	}
	
	return 0;
}