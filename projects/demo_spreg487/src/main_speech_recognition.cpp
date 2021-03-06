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
#include <algorithm>    // For std::shuffle
#include <random>       // For std::mt19937, std::random_device
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>

#include "utilities.h"

#define INHIBITORY 1
#define EXCITATORY 0

#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/


poolingConnection::~poolingConnection() {}

poolingConnection::poolingConnection(int strideX, int strideY, int inputX, int inputY, int kernelX, int kernelY, int outputX, int outputY)
    :strideX(strideX), strideY(strideY), inputX(inputX), inputY(inputY), kernelX(kernelX), kernelY(kernelY), outputX(outputX), outputY(outputY)
{
	int j = 0,h = 0,k = 0;
	for (int y = 0; y < outputX; y++) {
		for (int x = 0; x < outputY; x++) {
			k = x + outputX*y;
			vector<int> srcConnection;
			connectionsMap.insert(pair<int, vector<int>>(k, srcConnection));
		}
	}

	k = 0;
	for (int y = 0; y < inputY; y+=strideY) {
		for (int x = 0; x < inputX; x+=strideX) {
			if (j >= outputX*outputY) return;
			k = x + inputX*y;
			for (int fy = 0; fy < kernelY; fy++) {
				for (int fx = 0; fx < kernelX; fx++) {
					h = k + fx + inputX*fy;
					connectionsMap[j].push_back(h);
				}
			}
			j++;
			if (x + kernelX >= inputX) break;
		}
	}
}

convolutionConnection::~convolutionConnection() {}

convolutionConnection::convolutionConnection(int strideX, int strideY, int inputX, int inputY, int kernelX, int kernelY, int outputX, int outputY, vector<float>& weights, int neuronType)
    :strideX(strideX), strideY(strideY), inputX(inputX), inputY(inputY), kernelX(kernelX), kernelY(kernelY), outputX(outputX), outputY(outputY), neuronType(neuronType)
{
	int j = 0,h = 0,k = 0;
	for (int y = 0; y < inputY; y+=strideY) {
		for (int x = 0; x < inputX; x+=strideX) {
			if (j >= outputX*outputY) return;
			k = x + inputX*y;
			for (int fy = 0; fy < kernelY; fy++) {
				for (int fx = 0; fx < kernelX; fx++) {
					h = k + fx + inputX*fy;
					connectionsMap[j][h] = weights[fx + kernelX*fy];
				}
			}
			j++;
			if (x + kernelX >= inputX) break;
		}
	}
}

poolingFullConnection::poolingFullConnection(int inputSize, int filterNumber, vector<vector<float>> &weights, int connectionType)
    :inputSize(inputSize), filterNumber(filterNumber), weights(weights), connectionType(connectionType){}


poolingFullConnection::~poolingFullConnection() {}

int main(int argc, const char* argv[])
{
	// Random number generator (from gaussian dist)
	std::default_random_engine generator (0);
	std::normal_distribution<float> distribution (0.5,0.1);
	// ---------------------------------------------- CONFIG STATE ---------------------------------------------- //
	// ---------------------------------------------------------------------------------------------------------- //
	CARLsim sim("demo_spreg487", CPU_MODE, SHOWTIME, 1, 123);
	
	Grid3D inDim(13, 99, 1);
	Grid3D convDim(13, 33, 1);
	Grid3D poolingDim(13, 11, 1);
	int conv_kernelX = 1;
	int conv_kernelY = 3;
	int conv_strideX = 1;
	int conv_strideY = 3;
	vector<float> conv_weights = {distribution(generator), distribution(generator), distribution(generator)};
	int pooling_kernelX = 1;
	int pooling_kernelY = 3;
	int pooling_strideX = 1;
	int pooling_strideY = 3;
	int numFeatureMaps = 1;
	

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
	float targetFiringRate = 35.0;

	int *convLayers = (int*)malloc(sizeof(int)*numFeatureMaps);
	int *inputToConvIDs = (int*)malloc(sizeof(int)*numFeatureMaps);

	// Create Convolutional Layers
	SpikeMonitor* spkMonConv;
	for (int i=0; i<numFeatureMaps; i++) {
		convLayers[i] = sim.createGroupLIF("convolutional", convDim, EXCITATORY_NEURON);
		sim.setNeuronParametersLIF(convLayers[i], (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));
		spkMonConv = sim.setSpikeMonitor(convLayers[i], "DEFAULT");
	}
	// -------------------- CONVOLUTIONAL LAYER INITIALIZATION -------------------- END

	// -------------------- POOLING LAYER INITIALIZATION -------------------------- START
	int *poolingLayers = (int*)malloc(sizeof(int)*numFeatureMaps);
	int *convToPoolingIDs = (int*)malloc(sizeof(int)*numFeatureMaps);
	
	// Create Pooling Layers
	SpikeMonitor* spkMon;
	for (int i=0; i<numFeatureMaps; i++) {
		poolingLayers[i] = sim.createGroupPoolingMaxRate("pooling", poolingDim, EXCITATORY_NEURON);
		sim.setNeuronParametersPoolingMaxRate(poolingLayers[i], (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));
		spkMon = sim.setSpikeMonitor(poolingLayers[i], "DEFAULT");
	}
	// -------------------- POOLING LAYER INITIALIZATION -------------------------- END

	// -------------------- CONNECT ALL THE LAYERS -------------------------------- START
	// Connect Input Layer to ALL Convolutional Layers
	// int strideX, int strideY, int inputX, int inputY, int kernelX, int kernelY, int outputX, int outputY
	for (int i=0; i<numFeatureMaps; i++) {
		convolutionConnection* convConn = new convolutionConnection(conv_strideX, conv_strideY, inDim.numX, inDim.numY, conv_kernelX, conv_kernelY, convDim.numX, convDim.numY, conv_weights, EXCITATORY);
		inputToConvIDs[i] = sim.connect(gIn, convLayers[i], convConn, SYN_PLASTIC);
		sim.setESTDP(convLayers[i], true, STANDARD, ExpCurve(alpha_LTP, tau_LTP, -alpha_LTD, tau_LTP));
		sim.setHomeostasis(convLayers[i],true,homeoScale,avgTimeScale);
		sim.setHomeoBaseFiringRate(convLayers[i],targetFiringRate,0);
	}

	// Connect EACH Convolutional Layer to EACH Max Pooling Layer
	for (int i=0; i<numFeatureMaps; i++) {
		poolingConnection* poolConn = new poolingConnection(pooling_strideX, pooling_strideY, convDim.numX, convDim.numY, pooling_kernelX, pooling_kernelY, poolingDim.numX, poolingDim.numY);
		convToPoolingIDs[i] = sim.connect(convLayers[i], poolingLayers[i], poolConn);
	}
	// -------------------- CONNECT ALL THE LAYERS -------------------------------- END

	// Use CUBA mode
	sim.setConductances(false);

	// ---------------------------------------------- SETUP STATE ----------------------------------------------- //
	// ---------------------------------------------------------------------------------------------------------- //
	FILE* simfId = NULL;
	simfId = fopen("/home/sweet/1-workdir/carlsim4_ductai199x/projects/speech_recognition/trained_network.dat", "rb");
	sim.loadSimulation(simfId);
	sim.setupNetwork();

	fclose(simfId);

	sim.scaleWeights(inputToConvIDs[0], 10);
	// ---------------------------------------------- TEST STATE ------------------------------------------------- //
	// ---------------------------------------------------------------------------------------------------------- //
	sim.startTesting();

	// VisualStimulus stim("/home/sweet/2-coursework/spreg487/src/demo/demo.dat");
	VisualStimulus stim("/home/sweet/2-coursework/487ecec/speech_recognition/src/processed_data/female/FCAJ0_SX129.WAV.dat");
	// VisualStimulus stim("/home/sweet/2-coursework/487ecec/speech_recognition/src/processed_data/male/MCEW0_SI1442.WAV.dat");
	// for (int j = 0; j < 20; j++) {
		for (int i=0; i<stim.getLength(); i++) {
			PoissonRate* rates = stim.readFramePoisson(50.0f, 0.0f);
			sim.setSpikeRate(gIn, rates);
			sim.runNetwork(1,0); // run the network
		}
	// }
	
	

	sim.stopTesting();

	
	
	return 0;
}