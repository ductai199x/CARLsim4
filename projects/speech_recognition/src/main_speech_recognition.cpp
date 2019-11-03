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


poolingConnection::~poolingConnection()
{

}

poolingConnection::poolingConnection(int stride, int inputX, int destX, int filterX)
    :stride(stride), inputX(inputX), inputY(inputX), destX(destX), destY(destX), filterX(filterX), filterY(filterX)
{
    // add dummy connections to the map. 
    for(int x=0; x<destX; x++)      
    {
        for(int y=0; y<destY; y++)      
        {
            int j = x*destX+y;              
            vector<int> srcConnection;      
            connectionsMap.insert(pair<int, vector<int>>(j,srcConnection)); 
            
            for(int fX=0; fX<filterX; fX++) 
            {
                for(int fY=0; fY<filterY; fY++) 
                {
                    connectionsMap[j].push_back((x*stride+fX)*inputX+(y*stride+fY));
                }
            }
        }
    }

    
	map<int, vector<int>>::iterator it;

	for(it= connectionsMap.begin(); it!=connectionsMap.end(); it++)
	{
  		cout << "DestinationNeuron: " << it->first << endl;
		// vector<int>::iterator itt;
		// for(itt = it->second.begin(); itt!= it->second.end(); itt++ )
		// {
		// 	std::cout << *itt << " " << endl;
		// } 
	}	
}

int main(int argc, const char* argv[])
{
	// Get all training files in directory
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

	// ---------------- CONFIG STATE -------------------
	CARLsim sim("smooth", CPU_MODE, DEVELOPER);

	// ----- INPUT LAYER INITIALIZATION -----
	Grid3D inDim(stim.getWidth(), stim.getHeight(), stim.getChannels());
	int gIn = sim.createSpikeGeneratorGroup("input", inDim, EXCITATORY_NEURON);
	sim.setSpikeMonitor(gIn, "DEFAULT");

	int numFeatureMaps = 1;
	// Random number generator (from gaussian dist)
	std::default_random_engine generator (0);
	std::normal_distribution<double> distribution (20.0,0.33);
	double gaus_rand = distribution(generator);
	// ----- CONVOLUTIONAL LAYER INITIALIZATION -----

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
	for (int i=0; i<numFeatureMaps; i++) {
		convLayers[i] = sim.createGroupLIF("convolutional", inDim, EXCITATORY_NEURON);
		sim.setNeuronParametersLIF(convLayers[i], (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));
		sim.setSpikeMonitor(convLayers[i], "DEFAULT");
	}

	// ----- POOLING LAYER INITIALIZATION -----
	int numConvFilters = 24;
	Grid3D numNeuronPoolingLayer(numConvFilters,1,1);
	int *poolingLayers = (int*)malloc(sizeof(int)*numFeatureMaps);
	int *convToPoolingIDs = (int*)malloc(sizeof(int)*numFeatureMaps);
	poolingConnection* poolConn = new poolingConnection(2, 10, 5, 2);

	// Create Pooling Layers
	SpikeMonitor* spkMon;
	for (int i=0; i<numFeatureMaps; i++) {
		poolingLayers[i] = sim.createGroupPoolingLIF("pooling", numNeuronPoolingLayer, EXCITATORY_NEURON);
		sim.setNeuronParametersPoolingLIF(poolingLayers[i], (int)tau_mE, (int)tau_refE, (float)vTh-1, (float)vReset, RangeRmem(rMem));
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
		convToPoolingIDs[i] = sim.connect(convLayers[i], poolingLayers[i], "gaussian", RangeWeight(0, gaus_rand, gaus_rand), 1.0f,
			RangeDelay(1), RadiusRF(2,2,-1), SYN_PLASTIC);
		// convToPoolingIDs[i] = sim.connect(convLayers[i], poolingLayers[i], poolConn, SYN_PLASTIC);
	}


	// Use CUBA mode
	sim.setConductances(false);

	// ---------------- SETUP STATE -------------------
	sim.setupNetwork();

	// start recording spikes associated with spkMon object
	spkMon->startRecording();

	// ---------------- RUN STATE -------------------
	// for (int n = 0; n < training_files.size(); n++)
	for (int n = 0; n < 1; n++)
	{
		VisualStimulus stim(training_files[n]);
		for (int i=0; i<stim.getLength(); i++) {
			PoissonRate* rates = stim.readFramePoisson(50.0f, 0.0f);
			sim.setSpikeRate(gIn, rates);
			sim.runNetwork(1,0); // run the network
		}
	}

	spkMon->stopRecording();
	// print a summary of the spike information""
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