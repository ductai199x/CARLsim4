/*
 * Copyright (c) 2013 Regents of the University of California. All rights reserved.
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
 * created by: 		(MDR) Micah Richert, (JN) Jayram M. Nageswaran
 * maintained by:	(MA) Mike Avery <averym@uci.edu>, (MB) Michael Beyeler <mbeyeler@uci.edu>,
 *					(KDC) Kristofor Carlson <kdcarlso@uci.edu>
 *
 * CARLsim available from http://socsci.uci.edu/~jkrichma/CARL/CARLsim
 */
#include "carlsim.h"
#include <spikegen_from_vector.h>
#include "utilities.h"
#include <visual_stimulus.h>
#include <vector>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

#define INHIBITORY 1
#define EXCITATORY 0



int ConstantISI::nextSpikeTime(CARLsim* sim, int grpId, int nid, int currentTime, int lastScheduledSpikeTime, int endOfTimeSlice)
{
        // printf("_numNeur=%d, getGroupNumNeurons=%d\n",_numNeur, sim->getGroupNumNeurons(grpId));
        assert(_numNeur == sim->getGroupNumNeurons(grpId));
        // periodic spiking according to ISI
        return (std::max(currentTime, lastScheduledSpikeTime) + _isi[nid]);
}

void ConstantISI::updateISI(unsigned char* stimGray, float maxRateHz=50.0f, float minRateHz=0.0f) 
{
    
	_isi.clear();
    
	// calculate inter-spike interval (ISI) from firing rate
    for (int i=0; i<_numNeur; i++) {
        // convert grayscale value to firing rate
        float rateHz = (float)stimGray[i] / 255.0f * (maxRateHz - minRateHz) + minRateHz;
        // invert firing rate to get inter-spike interval (ISI)
        int isi = (rateHz > 0.0f) ? std::max(1, (int)(1000 / rateHz)) : 1000000;
        // add value to vector
        _isi.push_back(isi);
    }
}

/**************************************************** Read Weights *****************************************************************/

void readWeights(int numFilters, int numNeurons, string fileName, weightVector &wVector)
{
	string line;
  	ifstream myfile (fileName);
  	if (myfile.is_open())
  	{
    	while ( getline (myfile,line) )
    	{
    	    vector<float> weight;
            std::string delimiter = " ";
			size_t pos = 0;
			std::string token;
			while ((pos = line.find(delimiter)) != std::string::npos) 
			{
    			token = line.substr(0, pos);
    			//std::cout << stof(token) << std::endl;
                weight.push_back(stof(token));
    			line.erase(0, pos + delimiter.length());

			}
            wVector.push_back(weight);
        }
    	myfile.close();
  	
        vector<vector<float>>::iterator it;
        for (it=wVector.begin(); it!=wVector.end(); it++)
        {
             //cout << "Size: " << it->size() << '\n';
        }
    }
    else cout << "Unable to open file"; 
}

/***************************************************** Convolution ******************************************************************/

convolutionConnection::~convolutionConnection()
{

}

convolutionConnection::convolutionConnection(int padding, int inputX, int destX, int filterX, vector<float> &weights, int neuronType)
    :padding(padding), inputX(inputX), inputY(inputX), destX(destX), destY(destX), filterX(filterX), filterY(filterX), neuronType(neuronType)
{
    // add dummy connections to the map. 
    for(int x=0; x<destX; x++)      
    {
        for(int y=0; y<destY; y++)      
        {
            int j = x*destX+y;              
            //map<int> srcConnection;      
            //connectionsMap.insert(pair<int, vector<int>>(j,srcConnection)); 
           
            if((x>inputX-filterX) || y>inputY-filterY)
                continue;
            
            for(int fX=0; fX<filterX; fX++) 
            {
                for(int fY=0; fY<filterY; fY++) 
                {
                    connectionsMap[j][(x+fX)*inputX+(y+fY)] = weights[fX*filterX+fY];
                }
            }
        }
    }

    
	map<int, map<int, float>>::iterator it;

	for(it= connectionsMap.begin(); it!=connectionsMap.end(); it++)
	{
  		//cout <<"DestinationNeuron: " << it->first << endl;
		map<int, float>::iterator itt;
		for(itt = it->second.begin(); itt!= it->second.end(); itt++ )
		{
			//cout << "Source Neuron: " << itt->first << " Weight: " << itt->second << endl;
		} 
	}	
}


/******************************************************** Pooling ***********************************************************************/

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
  		//cout << "DestinationNeuron: " << it->first << endl;
		vector<int>::iterator itt;
		for(itt = it->second.begin(); itt!= it->second.end(); itt++ )
		{

		//	cout << *itt << " " << endl;
		} 
	}	
}

/*****************************************************************************************************************************************/

poolingFullConnection::poolingFullConnection(int inputSize, int filterNumber, vector<vector<float>> &weights, int connectionType)
    :inputSize(inputSize), filterNumber(filterNumber), weights(weights), connectionType(connectionType){}


poolingFullConnection::~poolingFullConnection(){}

/****************************************************************************************************************************************/

fullConnection::fullConnection(weightVector &weights, int connectionType)
    :weights(weights), connectionType(connectionType){}


fullConnection::~fullConnection(){}

/****************************************************************************************************************************************/
int main(int argc, const char* argv[]) {
	// ---------------- CONFIG STATE -------------------
    
    int numGPUs = 1;
    int randSeed = 123;
    CARLsim* sim = new CARLsim("Test", CPU_MODE, USER, numGPUs, randSeed);

	// Input stimulus created from an image using the MATLAB script
	// "createStimFromImage.m":
	int i,j;
    int inputSize = 1024;
    int conv0Size = 784;
    int pool0Size = 196;
    int conv1Size = 100;
    int pool1Size = 25;
    int conv2Size = 120;
    int fc1Size = 84;
    int fc2Size = 10;

    int numConv0Filters = 6;
    int numConv1Filters = 16;

/**********************************************************************************************/
    // integer
    int tau_mE = 10;
    int tau_refE = 1;
    float vTh = -78.0f;
    float vReset = -80.0f;
    float vInit = -80.0f;
    float rMem = 10;

/************************ INPUTS ***************************************************************/

    vector <int> neuron0 = {5}; 
    VisualStimulus VS("inp_gray_28x28x1000.dat", 1);
    int videoLength = VS.getLength();
	
    string nameConv1 = "weights_validation/conv1_weights.syntxt";
    weightVector wConv1; 
    readWeights(1, 1, nameConv1, wConv1);	
   
    string nameConv2 = "weights_validation/conv2_weights.syntxt";
    weightVector wConv2;
    readWeights(1, 1, nameConv2, wConv2);

    string nameConv3 = "weights_validation/conv3_weights.syntxt";
    weightVector wConv3;
    readWeights(1, 1, nameConv3, wConv3);

    string nameFC1 = "weights_validation/fc1_weights.syntxt";
    weightVector wFC1;
    readWeights(1, 1, nameFC1, wFC1);
    
    string nameFC2 = "weights_validation/fc2_weights.syntxt";
    weightVector wFC2;
    readWeights(1, 1, nameFC2, wFC2);
    
    int gIn0 = sim->createSpikeGeneratorGroup("input0", inputSize, EXCITATORY_NEURON);
    ConstantISI constISI(1024);
    sim->setSpikeGenerator(gIn0, &constISI);
    //SpikeGeneratorFromVector SGV0(neuron0);
    //sim->setSpikeGenerator(gIn0, &SGV0);

/*****************************************Input Excitatory and Inhibitory groups*****************************************/

    int gInExt = sim-> createGroupLIF("InputExc", inputSize, EXCITATORY_NEURON);
    int gInInh = sim-> createGroupLIF("InputInh", inputSize, INHIBITORY_NEURON);
    sim->setNeuronParametersLIF((int)gInExt, (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));
    sim->setNeuronParametersLIF((int)gInInh, (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));
 
    // Connect the spike generator to the  input groups to generate excitatory and inhibitory signals 
    sim->connect(gIn0, gInExt, "one-to-one", RangeWeight(1.0f), 1.0f, RangeDelay(1));
    sim->connect(gIn0, gInInh, "one-to-one", RangeWeight(1.0f), 1.0f, RangeDelay(1));

/************************************************************************************************************************/
//         Connect the input inhibitory and excitatory signals to the conv layer 
//         and assign the connects based on the polarity of the weights
/************************************************************************************************************************/    
	
    int gConv0[numConv0Filters]; 
    for(int i=0; i<numConv0Filters; i++)
    {
        string groupName = "Conv0_" + to_string(i);
        gConv0[i] = sim->createGroupLIF(groupName, conv0Size, EXCITATORY_NEURON); 
        sim->setNeuronParametersLIF((int)gConv0[i], (int)tau_mE, (int)tau_refE, (float)-79.9f, (float)vReset, RangeRmem(rMem));
        
	    // Connect only Positive weights.
        convolutionConnection* conv0ConnExh = new convolutionConnection(0, 32, 28, 5, wConv1[i], EXCITATORY); 
	    // Connect only Negative weights. 
        convolutionConnection* conv0ConnInh = new convolutionConnection(0, 32, 28, 5, wConv1[i], INHIBITORY);

        // Connect the two groups to their respective filters.
        sim->connect(gInExt, gConv0[i], conv0ConnExh, SYN_PLASTIC);
        sim->connect(gInInh, gConv0[i], conv0ConnInh, SYN_PLASTIC);

    }

/**************************************************************************************************************************/
//         Connect the generated Convolution 0 output to the Pooling output (filter).  
/************************************************************************************************************************/    
    int gPoolExh0[numConv0Filters];
    int gPoolInh0[numConv0Filters];
    for(int i=0; i<numConv0Filters; i++)
    {   
        string exhGroupName = "PoolExt0_" + to_string(i);
        string inhGroupName = "PoolInh0_" + to_string(i);
        gPoolExh0[i] = sim->createGroupPoolingMaxRate(exhGroupName, pool0Size, EXCITATORY_NEURON);
        gPoolInh0[i] = sim->createGroupPoolingMaxRate(inhGroupName, pool0Size, INHIBITORY_NEURON);
        sim->setNeuronParametersPoolingMaxRate((int)gPoolExh0[i], (int)tau_mE, (int)tau_refE, (float)vInit, (float)-79.9f, (float)vReset, RangeRmem(rMem));
        sim->setNeuronParametersPoolingMaxRate((int)gPoolInh0[i], (int)tau_mE, (int)tau_refE, (float)vInit, (float)-79.9f, (float)vReset, RangeRmem(rMem));
	    
        // Connect Convolution outpu to the pooling output via the filters. 
        poolingConnection* pool0Conn = new poolingConnection(2, 28, 14, 2);

        sim->connect(gConv0[i], gPoolExh0[i], pool0Conn, SYN_PLASTIC);
        sim->connect(gConv0[i], gPoolInh0[i], pool0Conn, SYN_PLASTIC);
    }

/************************************************************************************************
        Connect the pooling layers to the Convolution 2 layer; 
        Connection is performed in the connection order mentioned in the paper. 
 *************************************************************************************************/
    vector<vector<int>> connMap;
    vector<int>Conn1={1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1};
    vector<int>Conn2={1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1};
    vector<int>Conn3={1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1};
    vector<int>Conn4={0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1};
    vector<int>Conn5={0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1};
    vector<int>Conn6={0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1};
  
    connMap.push_back(Conn1);
    connMap.push_back(Conn2);
    connMap.push_back(Conn3);
    connMap.push_back(Conn4);
    connMap.push_back(Conn5);
    connMap.push_back(Conn6);
    
    int gConv1[numConv1Filters]; 

    for(int j=0; j<numConv1Filters; j++)
    {
        string groupName = "Conv1_" + to_string(j);

        gConv1[j] = sim->createGroupLIF(groupName, conv1Size, EXCITATORY_NEURON); 
        sim->setNeuronParametersLIF((int)gConv1[j], (int)tau_mE, (int)tau_refE, (float)-79.9f, (float)vReset, RangeRmem(rMem));
        
    }

    for (int i=0; i<numConv0Filters; i++)
    {
        for(int j=0; j<numConv1Filters; j++)
        {
            if(connMap[i][j])
            {
                // Positive weights as EXH neurons;
                convolutionConnection* conv1ConnExh = new convolutionConnection(0, 14, 10, 5, wConv2[i], EXCITATORY);
                // Negative weights as INH neurons;
                convolutionConnection* conv1ConnInh = new convolutionConnection(0, 14, 10, 5, wConv2[i], INHIBITORY);
        
                sim->connect(gPoolExh0[i], gConv1[j], conv1ConnExh, SYN_PLASTIC);
                sim->connect(gPoolInh0[i], gConv1[j], conv1ConnInh, SYN_PLASTIC);
            }
        }
    }


/**************************************************************************************************************************/
//         Connect the generated Convolution 1 output to the Pooling 1 output (filter).  
/************************************************************************************************************************/    
    int gPoolExh1[numConv1Filters];
    int gPoolInh1[numConv1Filters];
    int connId = 0;
    for(int i=0; i<numConv1Filters; i++)
    {   
        string exhgroupName = "poolExh1_" + to_string(i);
        string inhgroupName = "poolInh1_" + to_string(i);
        gPoolExh1[i] = sim->createGroupPoolingMaxRate(exhgroupName, pool0Size, EXCITATORY_NEURON);
        gPoolInh1[i] = sim->createGroupPoolingMaxRate(inhgroupName, pool0Size, INHIBITORY_NEURON);
        sim->setNeuronParametersPoolingMaxRate((int)gPoolExh1[i], (int)tau_mE, (int)tau_refE, (float)vInit, (float)-79.9f, (float)vReset, RangeRmem(rMem));
        sim->setNeuronParametersPoolingMaxRate((int)gPoolInh1[i], (int)tau_mE, (int)tau_refE, (float)vInit, (float)-79.9f, (float)vReset, RangeRmem(rMem));
	    
        // Connect Convolution outpu to the pooling output via the filters. 
        poolingConnection* pool1Conn = new poolingConnection(2, 10, 5, 2);

        sim->connect(gConv1[i], gPoolExh1[i], pool1Conn, SYN_PLASTIC);
        sim->connect(gConv1[i], gPoolInh1[i], pool1Conn, SYN_PLASTIC);
    }

    
/************************************************************************************************
                    Fully connect the Pooling output to the 120 neurons in the Conv3 layer. 
*************************************************************************************************/

    int gConv2Exh = sim->createGroupLIF("ConvExh2", conv2Size, EXCITATORY_NEURON); 
    int gConv2Inh = sim->createGroupLIF("ConvInh2", conv2Size, INHIBITORY_NEURON); 
    sim->setNeuronParametersLIF((int)gConv2Exh, (int)tau_mE, (int)tau_refE, (float)-79.99f, (float)vReset, RangeRmem(rMem));
    sim->setNeuronParametersLIF((int)gConv2Inh, (int)tau_mE, (int)tau_refE, (float)-70.99f, (float)vReset, RangeRmem(rMem));

    for(int i=0; i<numConv1Filters; i++)
    {
        poolingFullConnection* convExh3Conn =  new poolingFullConnection(25, i, wConv3, EXCITATORY);
        poolingFullConnection* convInh3Conn =  new poolingFullConnection(25, i, wConv3, INHIBITORY);
        
        sim->connect(gPoolExh1[i], gConv2Exh, convExh3Conn, SYN_PLASTIC);
        sim->connect(gPoolExh1[i], gConv2Inh, convExh3Conn, SYN_PLASTIC);
        sim->connect(gPoolInh1[i], gConv2Exh, convInh3Conn, SYN_PLASTIC);
        sim->connect(gPoolInh1[i], gConv2Inh, convInh3Conn, SYN_PLASTIC);
    }


/************************************************************************************************
                    Connect the Conv3 layer to the FC1 layer
 *************************************************************************************************/
    
    int gFCExh1 = sim->createGroupLIF("FCExh1", fc1Size, EXCITATORY_NEURON); 
    int gFCInh1 = sim->createGroupLIF("FCInh1", fc1Size, INHIBITORY_NEURON); 
    sim->setNeuronParametersLIF((int)gFCExh1, (int)tau_mE, (int)tau_refE, (float)-79.99f, (float)vReset, RangeRmem(rMem));
    sim->setNeuronParametersLIF((int)gFCInh1, (int)tau_mE, (int)tau_refE, (float)-79.99f, (float)vReset, RangeRmem(rMem));


    fullConnection* fcExh1Conn = new fullConnection(wFC1, EXCITATORY); 
    fullConnection* fcInh1Conn = new fullConnection(wFC1, INHIBITORY);

    sim->connect(gConv2Exh, gFCExh1, fcExh1Conn, SYN_PLASTIC);
    sim->connect(gConv2Exh, gFCInh1, fcExh1Conn, SYN_PLASTIC);
    sim->connect(gConv2Inh, gFCExh1, fcInh1Conn, SYN_PLASTIC);
    sim->connect(gConv2Inh, gFCInh1, fcInh1Conn, SYN_PLASTIC);

/************************************************************************************************
                    Connect the FC1 to the FC2 layer
 *************************************************************************************************/
    
    int gFCExh2 = sim->createGroupLIF("FCExh2", fc2Size, EXCITATORY_NEURON); 
    sim->setNeuronParametersLIF((int)gFCExh2, (int)tau_mE, (int)tau_refE, (float)-79.99f, (float)vReset, RangeRmem(rMem));

    fullConnection* fcExh2Conn = new fullConnection(wFC2, EXCITATORY); 
    fullConnection* fcInh2Conn = new fullConnection(wFC2, INHIBITORY);

    int connId1 = sim->connect(gFCExh1, gFCExh2, fcExh2Conn, SYN_PLASTIC);
    int connId2 = sim->connect(gFCInh1, gFCExh2, fcInh2Conn, SYN_PLASTIC);

/************************************************************************************************
*************************************************************************************************/
    // Use CUBA mode
    sim->setConductances(false);
	
    // ---------------- SETUP STATE -------------------
	sim->setupNetwork();
    
    //SpikeMonitor* input = sim->setSpikeMonitor(gConv0[1], "DEFAULT");
    //SpikeMonitor* pool1 = sim->setSpikeMonitor(gConv0[0], "DEFAULT");
    SpikeMonitor* input = sim->setSpikeMonitor(gFCExh1, "DEFAULT");
    SpikeMonitor* pool1 = sim->setSpikeMonitor(gFCExh2, "DEFAULT");

    input->startRecording();
    pool1->startRecording();
   
    // ---------------- RUN STATE -------------------

    //ConnectionMonitor * CM1 = sim->setConnectionMonitor(gInExt,gConv0[3],"DEFAULT");
    //ConnectionMonitor * CM2 = sim->setConnectionMonitor(gConv0[0],gPool0[0],"DEFAULT");
    //CM1->setUpdateTimeIntervalSec(-1);
    //CM2->setUpdateTimeIntervalSec(-1);
    
    sim->startTesting(); 
    for (int i=0; i<10; i++)
    {  
        constISI.updateISI(VS.readFrameChar(), 500.0f, 0.0f);
        sim->runNetwork(2,0); // run the networki
        sim->scaleWeights(connId1, 20.345f, false);
        //sim->scaleWeights(connId2, 0.345f, false);
    }
    
    sim->stopTesting(); 
    input->stopRecording();
    pool1->stopRecording();
    
    input->print(true);
    pool1->print(true);

    for (int i=0; i<wFC2.size(); i++)
    {
        for(int j=0; j<wFC2[0].size(); j++)
        printf("%f,", wFC2[i][j]);

        printf("\n");
    }

    //CM1->print();
    //CM2->print();
    delete sim;
    return 0;
}
