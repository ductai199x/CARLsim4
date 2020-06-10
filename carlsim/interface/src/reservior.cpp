#include "reservoir.h"

#include <math.h>
#include <vector>
#include <iostream>
#include <cstdio>
#include <random>
#include <chrono>

class Reservoir::Impl {

public:
    Impl(Reservoir* resv, CARLsim *sim, string name, int num_neurons, float exc_to_inh_ratio, float pConn, int input_id, int output_id)
    {
        exc_to_inh_ratio_ = exc_to_inh_ratio;
        num_neurons_ = num_neurons;
        num_exc_ = (int) (num_neurons_/(1+exc_to_inh_ratio_)*exc_to_inh_ratio_);
        num_inh_ = num_neurons_ - num_exc_;
        sim_ = sim;
        pConn_ = pConn;
        name_ = name;
        input_id_ = input_id;
        output_id_ = output_id;

        max_weight_ = 20.0f;
        min_weight_ = 5.0f;
    }

    ~Impl() { }

    void create()
    {
        // LIF Parameters Initialization
        // int tau_mE = 10;
        // int tau_refE = 2;
        // float vTh = -60.0f;
        // float vReset = -62.0f;
        // float vInit = -60.01f;
        // float rMem = 10;

        // // Create excitory neuron group
        // exc_id_ = sim_->createGroupLIF(name_ + " exc", num_exc_, EXCITATORY_NEURON);
        // sim_->setNeuronParametersLIF(exc_id_, (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));
        // // Create inhibitory neuron group
        // inh_id_ = sim_->createGroupLIF(name_ + " inh", num_inh_, INHIBITORY_NEURON);
        // sim_->setNeuronParametersLIF(inh_id_, (int)tau_mE, (int)tau_refE, (float)vTh, (float)vReset, RangeRmem(rMem));

        // // Create excitory neuron group
        exc_id_ = sim_->createGroup(name_ + " exc", num_exc_, EXCITATORY_NEURON);
        sim_->setNeuronParameters(exc_id_, 250.0f, 2.5f, -60.0f, -25.0f, 0.01f, 2.0f, 30.0f, -55.0f, 200.0f);
        // // Create inhibitory neuron group
        inh_id_ = sim_->createGroup(name_ + " inh", num_inh_, INHIBITORY_NEURON);
        sim_->setNeuronParameters(inh_id_, 250.0f, 2.5f, -60.0f, -25.0f, 0.01f, 2.0f, 30.0f, -55.0f, 200.0f);

        // Connect neurons inside the EXC group
        RandConnRandWeight* exe_exc_conn = new RandConnRandWeight(max_weight_, min_weight_, random_seed(), pConn_, num_exc_, num_exc_);
        exc_exc_ = sim_->connect(exc_id_, exc_id_, exe_exc_conn, SYN_FIXED);

        // Connect neurons inside the INH group
        RandConnRandWeight* inh_inh_conn = new RandConnRandWeight(max_weight_, min_weight_, random_seed(), pConn_, num_inh_, num_inh_);
        inh_inh_ = sim_->connect(inh_id_, inh_id_, inh_inh_conn, SYN_FIXED);

        // Connect neurons between EXC and INH group
        RandConnRandWeight* exe_inh_conn = new RandConnRandWeight(max_weight_, min_weight_, random_seed(), pConn_, num_exc_, num_inh_);
        exc_inh_ = sim_->connect(exc_id_, inh_id_, exe_inh_conn, SYN_FIXED);

        // Connect neurons between INH and EXC group
        RandConnRandWeight* inh_exc_conn = new RandConnRandWeight(max_weight_, min_weight_, random_seed(), pConn_, num_inh_, num_exc_);
        inh_exc_ = sim_->connect(inh_id_, exc_id_, inh_exc_conn, SYN_FIXED);
    }

    int getExcID() { return exc_id_; }

    int getInhID() { return inh_id_; }

    int getGroupStartNID(int gID) { return sim_->getGroupStartNeuronId(gID); }

    int getGroupEndNID(int gID) { return sim_->getGroupEndNeuronId(gID); }

    void connectToReservoir()
    {
        FullConnRandWeight* input_exc_conn = new FullConnRandWeight(max_weight_, min_weight_, random_seed(), sim_->getGroupNumNeurons(input_id_), num_exc_);
        sim_->connect(input_id_, exc_id_, input_exc_conn, SYN_FIXED);

        FullConnRandWeight* input_inh_conn = new FullConnRandWeight(max_weight_, min_weight_, random_seed(), sim_->getGroupNumNeurons(input_id_), num_inh_);
        sim_->connect(input_id_, inh_id_, input_inh_conn, SYN_FIXED);

        FullConnRandWeight* exc_output_conn = new FullConnRandWeight(max_weight_, min_weight_, random_seed(), num_exc_, sim_->getGroupNumNeurons(output_id_));
        sim_->connect(exc_id_, output_id_, exc_output_conn, SYN_FIXED);

        FullConnRandWeight* inh_output_conn = new FullConnRandWeight(max_weight_, min_weight_, random_seed(), num_inh_, sim_->getGroupNumNeurons(output_id_));
        sim_->connect(inh_id_, output_id_, inh_output_conn, SYN_FIXED);
    }

    void setupMonitors()
    {
        spkMonExc_ = sim_->setSpikeMonitor(exc_id_, "DEFAULT");
        spkMonInh_ = sim_->setSpikeMonitor(inh_id_, "DEFAULT");
    }

    void startMonitors()
    {
        spkMonExc_->startRecording();
        spkMonInh_->startRecording();
    }

    void stopMonitors()
    {
        spkMonExc_->stopRecording();
        spkMonInh_->stopRecording();
    }

    SpikeMonitor* getSpkMon(int s) 
    {
        if (!s) return spkMonExc_;
        else return spkMonInh_;
    }

private:
    Reservoir* resv_;
    CARLsim* sim_;
    string name_;
    int num_neurons_;
    float exc_to_inh_ratio_;
    int num_exc_;
    int num_inh_;
    int exc_id_;
    int inh_id_;

    int exc_exc_;
    int inh_inh_;
    int exc_inh_;
    int inh_exc_;

    float max_weight_;
    float min_weight_;

    float pConn_;
    int input_id_;
    int output_id_;
    int output_connID_;

    SpikeMonitor* spkMonExc_;
    SpikeMonitor* spkMonInh_;

    unsigned random_seed()
    {
        return std::chrono::system_clock::now().time_since_epoch().count();
    }
};


Reservoir::Reservoir(CARLsim *sim, string name, int num_neurons, float exc_to_inh_ratio, float pConn, int input_id, int output_id)
:
_impl( new Impl(this, sim, name, num_neurons, exc_to_inh_ratio, pConn, input_id, output_id) ) {}

Reservoir::~Reservoir() { delete _impl; }

void Reservoir::create() { return _impl->create(); }

int Reservoir::getExcID() { return _impl->getExcID(); }

int Reservoir::getInhID() { return _impl->getInhID(); }

int Reservoir::getGroupStartNID(int gID) { return _impl->getGroupStartNID(gID); }

int Reservoir::getGroupEndNID(int gID) { return _impl->getGroupEndNID(gID); }

void Reservoir::connectToReservoir() { return _impl->connectToReservoir(); }

void Reservoir::setupMonitors() { return _impl->setupMonitors(); }

void Reservoir::startMonitors() { return _impl->startMonitors(); }

void Reservoir::stopMonitors() { return _impl->stopMonitors(); }

SpikeMonitor* Reservoir::getSpkMon(int s) { return _impl->getSpkMon(s); }


RandConnRandWeight::RandConnRandWeight(float max_weight, float min_weight, float rand_seed, float pConn, int numSrc, int numDest) 
{
    pConn_ = pConn;
    rand_seed_ = rand_seed;
    min_weight_ = min_weight;
    max_weight_ = max_weight;
    generator_.seed(rand_seed_);

    weightMatrix_ = new vector<vector<int>> ( numSrc , vector<int> (numDest, 0));
}
RandConnRandWeight::~RandConnRandWeight() {}

void RandConnRandWeight::connect(CARLsim* sim, int srcGrp, int i, int destGrp, int j, float& weight, float& maxWt, float& delay, bool& connected) 
{
    std::uniform_real_distribution<double> distribution_conn(0, 1);
    std::uniform_real_distribution<double> distribution_w(min_weight_, max_weight_);
    connected = ((float)distribution_conn(generator_) < pConn_);
    weight = 1.0f;
    maxWt = max_weight_;
    delay = 1;

    if (connected) {
        weight = distribution_w(generator_);
        (*weightMatrix_)[i][j] = weight;
    } else {
        weight = 0.0f;
    }
    
}

vector<vector<int>>* RandConnRandWeight::getWeightMatrix() {
    return weightMatrix_;
}


FullConnRandWeight::FullConnRandWeight(float max_weight, float min_weight, float rand_seed, int numSrc, int numDest) 
{
    rand_seed_ = rand_seed;
    min_weight_ = min_weight;
    max_weight_ = max_weight;
    generator_.seed(rand_seed_);

    weightMatrix_ = new vector<vector<int>> ( numSrc , vector<int> (numDest, 0));
}
FullConnRandWeight::~FullConnRandWeight() {}

void FullConnRandWeight::connect(CARLsim* sim, int srcGrp, int i, int destGrp, int j, float& weight, float& maxWt, float& delay, bool& connected) 
{
    std::uniform_real_distribution<double> distribution_w(min_weight_, max_weight_);
    connected = 1;
    weight = 1.0f;
    maxWt = max_weight_;
    delay = 1;

    if (connected) {
        weight = distribution_w(generator_);
        (*weightMatrix_)[i][j] = weight;
    } else {
        weight = 0.0f;
    }
    
}

vector<vector<int>>* FullConnRandWeight::getWeightMatrix() {
    return weightMatrix_;
}