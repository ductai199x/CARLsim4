#ifndef _TIMETOFIRST_SPIKEGEN_H_
#define _TIMETOFIRST_SPIKEGEN_H_


#include <memory>
#include <callback.h>
#include <map>
#include <vector>
#include <stdlib.h>

using namespace std;

class TimeToFirstSpike : public ReservoirSpikeGenerator
{
	public:

		TimeToFirstSpike(vector<float>* spikeVector, int max_intensity) 
		{
			spikeVector_ = spikeVector;
			max_intensity_ = (float)max_intensity + 1;

			isSpikedArr_ = unique_ptr<int>(new int[(*spikeVector_).size()]);
			targetSpkTimes_ = unique_ptr<map<int, int>>(new map<int, int>);
		}

		~TimeToFirstSpike() {};

		void newSpikeVector(vector<float>* spikeVector);
	    int nextSpikeTime(CARLsim*s, int grpId, int nId, int currentTime, int lastScheduledSpikeTime, int endOfTimeSlice);
		map<int, int>*  getTargetSpkTimes();
    
    private:

        vector<float>* spikeVector_;
		unique_ptr<int> isSpikedArr_;
		unique_ptr<map<int, int>> targetSpkTimes_;
		float max_intensity_;
		bool isRefSpike_;

};

#endif