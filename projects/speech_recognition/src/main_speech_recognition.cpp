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

#include "utilities.h"

#define INHIBITORY 1
#define EXCITATORY 0


poolingConnection::~poolingConnection() {}

poolingConnection::poolingConnection(int stride, int inputX, int inputY, int dest_size, int filterX, int filterY)
    :stride(stride), inputX(inputX), inputY(inputY), destX(dest_size), destY(dest_size), filterX(filterX), filterY(filterY)
{
	int j = 0,h = 0,k = 0;
	for (int y = 0; y < filterX; y++) {
		for (int x = 0; x < filterY; x++) {
			k = x + filterX*y;
			vector<int> srcConnection;
			connectionsMap.insert(pair<int, vector<int>>(k, srcConnection));
		}
	}

	k = 0;
	for (int y = 0; y < inputY; y+=stride) {
		for (int x = 0; x < inputX; x+=stride) {
			k = x + inputX*y;
			// cout << j << " - ";
			for (int fy = 0; fy < destX; fy++) {
				for (int fx = 0; fx < destX; fx++) {
					h = k + fx + inputX*fy;
					connectionsMap[j].push_back(h);
					// cout << h << "  ";
				}
			}
			j++;
			// cout << endl;
		}
	}
}

convolutionConnection::~convolutionConnection() {}

convolutionConnection::convolutionConnection(int padding, int inputX, int inputY, int dest_size, int filterX, int filterY, float weight, int neuronType)
    :padding(padding), inputX(inputX), inputY(inputY), destX(dest_size), destY(dest_size), filterX(filterX), filterY(filterY), neuronType(neuronType)
{
    for(int x=0; x<destX; x++) {
        for(int y=0; y<destY; y++) {
            int j = x*destX+y;
           
            if((x>inputX-filterX) || y>inputY-filterY)
                continue;
            
            for(int fX=0; fX<filterX; fX++) {
                for(int fY=0; fY<filterY; fY++) {
                    connectionsMap[j][(x+fX)*inputX+(y+fY)] = weight;
                }
            }
        }
    }
}

poolingFullConnection::poolingFullConnection(int inputSize, int filterNumber, vector<vector<float>> &weights, int connectionType)
    :inputSize(inputSize), filterNumber(filterNumber), weights(weights), connectionType(connectionType){}


poolingFullConnection::~poolingFullConnection() {}


int main(int argc, const char* argv[])
{
	// Get all training files in directory
	const char *training_folder_M = "/home/sweet/2-coursework/spreg487/src/processed_data/male/";
	const char *training_folder_F = "/home/sweet/2-coursework/spreg487/src/processed_data/female/";

	std::vector <std::string> training_files;
	int num_files = 300;

    if (auto dir = opendir(training_folder_M)) {
		int i = 0;
		while (auto f = readdir(dir)) {
			if (i >= num_files) break;
			if (!f->d_name || f->d_name[0] == '.')
				continue; // Skip everything that starts with a dot
			training_files.push_back(std::string(training_folder_M) + std::string(f->d_name));
			i++;
		}
		closedir(dir);
	}

	if (auto dir = opendir(training_folder_F)) {
		int i = 0;
		while (auto f = readdir(dir)) {
			if (i >= num_files) break;
			if (!f->d_name || f->d_name[0] == '.')
				continue; // Skip everything that starts with a dot
			training_files.push_back(std::string(training_folder_F) + std::string(f->d_name));
			i++;
		}
		closedir(dir);
	}

	VisualStimulus stim(training_files[0]);
	stim.print();

	// ---------------------------------------------- CONFIG STATE ---------------------------------------------- //
	// ---------------------------------------------------------------------------------------------------------- //
	CARLsim sim("spreg487", CPU_MODE, USER, 1, 123);
	Grid3D inDim(13, 99, 1);
	Grid3D convDim(9, 96, 1);
	Grid3D poolingDim(3, 32, 1);
	int numFeatureMaps = 1;
	// Random number generator (from gaussian dist)
	std::default_random_engine generator (0);
	std::normal_distribution<double> distribution (20.0,0.33);

	// -------------------- INPUT LAYER INITIALIZATION ---------------------------- START

	int gIn = sim.createSpikeGeneratorGroup("input", inDim, EXCITATORY_NEURON);
	sim.setSpikeMonitor(gIn, "DEFAULT");
	// -------------------- INPUT LAYER INITIALIZATION ---------------------------- END

	

	// -------------------- CONVOLUTIONAL LAYER INITIALIZATION -------------------- START

	// LIF Parameters Initialization
    int tau_mE = 10;
    int tau_refE = 1;
    float vTh = -78.0f;
    float vReset = -80.0f;
    float vInit = -80.0f;
    float rMem = 10;

	// set E-STDP parameters.
	float alpha_LTP=0.001f/100; float tau_LTP=20.0f;
	float alpha_LTD=0.0015f/100; float tau_LTD=20.0f;

	// homeostasis constants
	float homeoScale= 1.0; // homeostatic scaling factor
	float avgTimeScale = 5.0; // homeostatic time constant
	float targetFiringRate = 70.0;

	int *convLayers = (int*)malloc(sizeof(int)*numFeatureMaps);
	int *inputToConvIDs = (int*)malloc(sizeof(int)*numFeatureMaps);

	// Create Convolutional Layers
	for (int i=0; i<numFeatureMaps; i++) {
		convLayers[i] = sim.createGroupLIF("convolutional", convDim, EXCITATORY_NEURON);
		sim.setNeuronParametersLIF(convLayers[i], (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));
		sim.setSpikeMonitor(convLayers[i], "DEFAULT");
	}
	// -------------------- CONVOLUTIONAL LAYER INITIALIZATION -------------------- END

	// -------------------- POOLING LAYER INITIALIZATION -------------------------- START
	int *poolingLayers = (int*)malloc(sizeof(int)*numFeatureMaps);
	int *convToPoolingIDs = (int*)malloc(sizeof(int)*numFeatureMaps);
	
	// Create Pooling Layers
	SpikeMonitor* spkMon;
	for (int i=0; i<numFeatureMaps; i++) {
		poolingLayers[i] = sim.createGroupPoolingLIF("pooling", poolingDim, EXCITATORY_NEURON);
		sim.setNeuronParametersPoolingLIF(poolingLayers[i], (int)tau_mE, (int)tau_refE, (float)vTh-1, (float)vReset, RangeRmem(rMem));
		spkMon = sim.setSpikeMonitor(poolingLayers[i], "DEFAULT");
	}
	// -------------------- POOLING LAYER INITIALIZATION -------------------------- END

	// -------------------- CONNECT ALL THE LAYERS -------------------------------- START
	// Connect Input Layer to ALL Convolutional Layers
	int conv_kernel_size = 5;
	for (int i=0; i<numFeatureMaps; i++) {
		convolutionConnection* convConn = new convolutionConnection(0, inDim.numX, inDim.numY, conv_kernel_size, convDim.numX, convDim.numY, distribution(generator), EXCITATORY);
		inputToConvIDs[i] = sim.connect(gIn, convLayers[i], convConn, SYN_PLASTIC);
		sim.setESTDP(convLayers[i], true, STANDARD, ExpCurve(alpha_LTP, tau_LTP, -alpha_LTD, tau_LTP));
		// sim.setHomeostasis(convLayers[i],true,homeoScale,avgTimeScale);
		// sim.setHomeoBaseFiringRate(convLayers[i],targetFiringRate,0);
	}

	// Connect EACH Convolutional Layer to EACH Max Pooling Layer
	int pooling_kernel_size = 3;
	int stride = 3;
	for (int i=0; i<numFeatureMaps; i++) {
		poolingConnection* poolConn = new poolingConnection(stride, convDim.numX, convDim.numY, pooling_kernel_size, poolingDim.numX, poolingDim.numY);
		convToPoolingIDs[i] = sim.connect(convLayers[i], poolingLayers[i], poolConn);
	}
	// -------------------- CONNECT ALL THE LAYERS -------------------------------- END

	// Use CUBA mode
	sim.setConductances(false);

	// ---------------------------------------------- SETUP STATE ----------------------------------------------- //
	// ---------------------------------------------------------------------------------------------------------- //
	sim.setupNetwork();

	// start recording spikes associated with spkMon object
	spkMon->startRecording();


	// ---------------------------------------------- RUN STATE ------------------------------------------------- //
	// ---------------------------------------------------------------------------------------------------------- //
	// for (int n = 1; n < 2; n++)
	// {
	// 	VisualStimulus stim(training_files[n]);
	// 	for (int i=0; i<stim.getLength(); i++) {
	// 		PoissonRate* rates = stim.readFramePoisson(50.0f, 0.0f);
	// 		sim.setSpikeRate(gIn, rates);
	// 		sim.runNetwork(1,0); // run the network
	// 	}
	// }


	for (int n = 0; n < num_files/2; n++)
	{
		VisualStimulus stim(training_files[n]);
		for (int i=0; i<stim.getLength(); i++) {
			PoissonRate* rates = stim.readFramePoisson(50.0f, 0.0f);
			sim.setSpikeRate(gIn, rates);
			sim.runNetwork(1,0); // run the network
		}
	}

	for (int n = num_files; n < num_files*3/2; n++)
	{
		VisualStimulus stim(training_files[n]);
		for (int i=0; i<stim.getLength(); i++) {
			PoissonRate* rates = stim.readFramePoisson(50.0f, 0.0f);
			sim.setSpikeRate(gIn, rates);
			sim.runNetwork(1,0); // run the network
		}
	}

	spkMon->stopRecording();

	sim.startTesting();
	
	for (int n = num_files/2; n < num_files; n++)
	{
		VisualStimulus stim(training_files[n]);
		for (int i=0; i<stim.getLength(); i++) {
			PoissonRate* rates = stim.readFramePoisson(50.0f, 0.0f);
			sim.setSpikeRate(gIn, rates);
			sim.runNetwork(1,0); // run the network
		}
	}

	// for (int n = num_files+num_files/2; n < num_files*2; n++)
	for (int n = num_files*3/2; n < num_files*2; n++)
	{
		VisualStimulus stim(training_files[n]);
		for (int i=0; i<stim.getLength(); i++) {
			PoissonRate* rates = stim.readFramePoisson(50.0f, 0.0f);
			sim.setSpikeRate(gIn, rates);
			sim.runNetwork(1,0); // run the network
		}
	}

	sim.stopTesting();


	// print a summary of the spike information""
	// spkMon->print();
	// get the average firing rate of each of the neurons in group excGrpId
	// std::vector<float> excFRs = spkMon->getAllFiringRates();

	// int max = 0;
	// std::vector<float>::iterator it = excFRs.begin();
	// for (it; it != excFRs.end(); it++) {
	// 	if (*it > max){
	// 		max = *it;
	// 	}
	// }
	// printf("max: %d", max);
	
	return 0;
}