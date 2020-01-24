
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <carlsim.h>
#include <visual_stimulus.h>

#include <math.h>
#include <vector>
#include <map>
#include <cassert>
#include <iostream>
#include <cstdio>

using namespace std;

typedef vector<vector<float>> weightVector;

class convolutionConnection : public ConnectionGenerator
{
    public:
    convolutionConnection(int strideX, int strideY, int inputX, int inputY, int kernelX, int kernelY, int outputX, int outputY, vector<float>& weights, int neuronType);
    convolutionConnection(int strideX, int strideY, int inputX, int inputY, int kernelX, int kernelY, int outputX, int outputY, float weight, int neuronType);
    ~convolutionConnection();
    
    private:
    
    int padding;
    int strideX, strideY;
    int kernelX, kernelY; 
    int outputX, outputY; 
    int inputX, inputY;
    // Inhibitory(1) or Excitatory (0) neuron group.  
    int neuronType;

    map<int, map<int, float>> connectionsMap;

    vector<float> weights;

    void connect(CARLsim* sim, int srcGrp, int i, int destGrp, int j, float &weight, float& maxWt, float& delay, bool& connected)
    {
        connected=0;
        map<int, float>::iterator it;
        it = connectionsMap[j].find(i);
        if(it != connectionsMap[j].end())
        {   
            if(!neuronType && (connectionsMap[j][it->first]>0))
            {
                connected=1;
                weight = connectionsMap[j][it->first];
            }
            else if(neuronType && (connectionsMap[j][it->first]<0))
            {
                connected=1;
                weight = fabs(connectionsMap[j][it->first]);
            }
            else
            {
                connected=0;
                weight=0;
            }
        }
        maxWt = 10.0f;
        delay = 1.0f;
    }

};


class poolingConnection : public ConnectionGenerator
{
    public:
    poolingConnection(int strideX, int strideY, int inputX, int inputY, int kernelX, int kernelY, int outputX, int outputY);
    ~poolingConnection();
    
    private:
    
    int strideX, strideY;
    int kernelX, kernelY; 
    int outputX, outputY; 
    int inputX, inputY;

    map<int, vector<int>> connectionsMap;

    void connect(CARLsim* sim, int srcGrp, int i, int destGrp, int j, float &weight, float& maxWt, float& delay, bool& connected)
    {
        connected=0;
        if(find(connectionsMap[j].begin(), connectionsMap[j].end(), i) != connectionsMap[j].end())
        {
            //cout << "Connected: " << i << " " << j << endl;
            connected=1;
        }
        maxWt = 10.0f;
        delay = 1.0f;
        weight = 1;
    }

};


class poolingFullConnection : public ConnectionGenerator
{
    public:
    poolingFullConnection(int inputSize, int filterNumber, weightVector &weights, int connectionType);
    ~poolingFullConnection();
    
    private:
    
    // destination group x dimension. 
    int filterNumber;
    int inputSize;
    int connectionType; // Inhibitory=1 - excitatory = 0;  

    weightVector weights; 
    
    void connect(CARLsim* sim, int srcGrp, int i, int destGrp, int j, float &weight, float& maxWt, float& delay, bool& connected)
    {
        connected=0;
        int weightIndex = filterNumber*inputSize + i;
        weight = weights[j][weightIndex]; 
        
        if(weight>1)
        {
            weight = 1;
        }

        if(!connectionType && (weight>0))
        {
            connected=1;
        }
        else if(connectionType && (weight<0))
        {
            connected=1;
            weight = fabs(weight);
        }
        else
        {
            connected=0;
            weight=0;
        }
        
        if(weight>1)
        {
            weight = 1;
        }
        maxWt = 10.0f;
        delay = 1.0f;
    }

};

class fullConnection : public ConnectionGenerator
{
    public:
    fullConnection(weightVector &weights, int connectionType);
    ~fullConnection();
    
    private:
    
    // destination group x dimension. 
    int filterNumber;
    int inputSize;
    int connectionType; // Inhibitory=1 - excitatory = 0;  

    weightVector weights; 
    
    void connect(CARLsim* sim, int srcGrp, int i, int destGrp, int j, float &weight, float& maxWt, float& delay, bool& connected)
    {
        connected=1;
        if(weight>=1)
        {
            cout << weight << endl;
            cout << "FC" << endl;
        }
        if(!connectionType && (weight>0))
        {
            connected=1;
            weight = weights[j][i]*10;
        }
        else if(connectionType && (weight<0))
        {
            connected=1;
            weight = fabs(weights[j][i]);
        }
        else
        {
            connected=0;
            weight=0;
        }
        
        if(weight>1)
        {
            weight = 1;
        }
        maxWt = 10.0f;
        delay = 1.0f;
    }

};

class ConstantISI : public SpikeGenerator {
public:
	ConstantISI(int numNeur) {
		_numNeur = numNeur;
	}
	~ConstantISI() {}

	void updateISI(unsigned char* stimGray, float maxRateHz=50.0f, float minRateHz=0.0f) {
		_isi.clear();

		// calculate inter-spike interval (ISI) from firing rate
		for (int i=0; i<_numNeur; i++) {
			// convert grayscale value to firing rate
			float rateHz = (float)stimGray[i] / 255.0f * (maxRateHz - minRateHz) + minRateHz;

			// invert firing rate to get inter-spike interval (ISI)
			int isi = (rateHz > 0.0f) ? max(1, (int)(1000 / rateHz)) : 1000000;

			// add value to vector
			_isi.push_back(isi);
		}
	}

	int nextSpikeTime(CARLsim* sim, int grpId, int nid, int currentTime,
		int lastScheduledSpikeTime, int endOfTimeSlice)
	{
		// printf("_numNeur=%d, getGroupNumNeurons=%d\n",_numNeur, sim->getGroupNumNeurons(grpId));
		assert(_numNeur == sim->getGroupNumNeurons(grpId));

		// periodic spiking according to ISI
		return (max(currentTime, lastScheduledSpikeTime) + _isi[nid]);
	}

private:
	int _numNeur;
	std::vector<int> _isi;
};

#endif
