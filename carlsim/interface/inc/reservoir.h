#ifndef __RESERVOIR__
#define __RESERVOIR__

#include <carlsim.h>

#include <string>

using namespace std;

class Reservoir
{
    public:
    
        ~Reservoir();
        Reservoir(CARLsim *sim, string name, int num_neurons, float exc_to_inh_ratio, float pConn, int rand_seed, int input_id);

        void create();
        int getExcID();
        int getInhID();
        int getGroupStartNID(int gID);
        int getGroupEndNID(int gID);
        void connectToReservoir();
        void randomizeWeights();
        // int getOutputConnectionID();
        void setupMonitors();
        void startMonitors();
        void stopMonitors();
        SpikeMonitor* getSpkMon(int s);

    private:

    	class Impl;
	    Impl* _impl;
        
};
#endif