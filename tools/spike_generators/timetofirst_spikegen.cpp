#include <timetofirst_spikegen.h>
#include <string.h>
#include <iostream>

using namespace std;

void TimeToFirstSpike::newSpikeVector(vector<float>* spikeVector) 
{
	spikeVector_ = spikeVector;
	memset(isSpikedArr_.get(), 0, (*spikeVector_).size()*sizeof(*isSpikedArr_.get()));
	targetSpkTimes_.get()->clear();
}

int TimeToFirstSpike::nextSpikeTime(CARLsim*s, int grpId, int nId, int currentTime, int lastScheduledSpikeTime, int endOfTimeSlice)
{
	if (isSpikedArr_.get()[nId] == 0)
	{
		isSpikedArr_.get()[nId] = 1;
		int scheduleTime = currentTime + (endOfTimeSlice - currentTime)/(max_intensity_)*(max_intensity_ - (*spikeVector_)[nId]);
		cout << (*spikeVector_)[nId] << " --> " << scheduleTime << ", " << endOfTimeSlice << endl;
		targetSpkTimes_.get()->insert({nId, scheduleTime});
		return scheduleTime;
	}
	else
	{ 
		return endOfTimeSlice;
	}
}

map<int, int>* TimeToFirstSpike::getTargetSpkTimes()
{
    return targetSpkTimes_.get();
}