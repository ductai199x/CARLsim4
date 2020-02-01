#ifndef __RESERVOIR__
#define __RESERVOIR__

#include <carlsim.h>
#include <random>
#include <string>

using namespace std;

class Reservoir
{
    public:
    
        ~Reservoir();
        Reservoir(CARLsim *sim, string name, int num_neurons, float exc_to_inh_ratio, float pConn, int rand_seed, int input_id, int output_id);

        void create();
        int getExcID();
        int getInhID();
        int getGroupStartNID(int gID);
        int getGroupEndNID(int gID);
        void connectToReservoir();
        // int getOutputConnectionID();
        void setupMonitors();
        void startMonitors();
        void stopMonitors();
        SpikeMonitor* getSpkMon(int s);

    private:

    	class Impl;
	    Impl* _impl;
        
};

class RandConnRandWeight : public ConnectionGenerator {
public:

    RandConnRandWeight(float max_weight, float min_weight, float rand_seed, float pConn, int numSrc, int numDest);
    ~RandConnRandWeight();
    // the pure virtual function inherited from base class
    // note that weight, maxWt, delay, and connected are passed by reference

    void connect(CARLsim* sim, int srcGrp, int i, int destGrp, int j, float& weight, float& maxWt, float& delay, bool& connected);

    vector<vector<int>>* getWeightMatrix();

private:

    vector<vector<int>>* weightMatrix_;
    float pConn_;
    float rand_seed_;
    float min_weight_;
    float max_weight_;
    std::default_random_engine generator_;

};

class FullConnRandWeight : public ConnectionGenerator {
public:

    FullConnRandWeight(float max_weight, float min_weight, float rand_seed, int numSrc, int numDest);
    ~FullConnRandWeight();
    // the pure virtual function inherited from base class
    // note that weight, maxWt, delay, and connected are passed by reference

    void connect(CARLsim* sim, int srcGrp, int i, int destGrp, int j, float& weight, float& maxWt, float& delay, bool& connected);

    vector<vector<int>>* getWeightMatrix();

private:

    vector<vector<int>>* weightMatrix_;
    float rand_seed_;
    float min_weight_;
    float max_weight_;
    std::default_random_engine generator_;

};

#endif