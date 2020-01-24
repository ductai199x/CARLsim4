#ifndef __RESERVOIR__
#define __RESERVOIR__

#include <carlsim.h>
#include <visual_stimulus.h>

#include <math.h>
#include <vector>
#include <map>
#include <cassert>
#include <iostream>
#include <cstdio>
#include <string>

using namespace std;

class Reservoir
{
    public:
    
        ~Reservoir();
        Reservoir(CARLsim *sim_, string name_, int num_neurons_, float exc_to_inh_ratio_, float pConn_, int input_id_);

        void create();
        int getExcID();
        int getInhID();
        int getGroupStartNID(int gID);
        int getGroupEndNID(int gID);
        void connectToReservoir();
        int getOutputConnectionID();
        void startMonitoring();
        void stopMonitoring();
        SpikeMonitor getSpkMon(int gID);

    private:
        CARLsim sim;
        string name;
        int num_neurons;
        int exc_to_inh_ratio;
        int num_exc;
        int num_inh;
        int exc_id;
        int inh_id;
        int pConn;
        int input_id;
        int output_connID;
        
};
#endif