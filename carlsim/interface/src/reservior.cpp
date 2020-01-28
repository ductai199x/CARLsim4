


#include "reservoir.h"

#include <math.h>
#include <vector>
#include <map>
#include <cassert>
#include <iostream>
#include <cstdio>
#include <random>

class Reservoir::Impl {

public:
    Impl(Reservoir* resv, CARLsim *sim, string name, int num_neurons, float exc_to_inh_ratio, float pConn, int rand_seed, int input_id)
    {
        exc_to_inh_ratio_ = exc_to_inh_ratio;
        num_neurons_ = num_neurons;
        num_exc_ = (int) (num_neurons_/(1+exc_to_inh_ratio_)*exc_to_inh_ratio_);
        num_inh_ = num_neurons_ - num_exc_;
        sim_ = sim;
        pConn_ = pConn;
        rand_seed_ = rand_seed;
        name_ = name;
        input_id_ = input_id;

        max_weight_ = 10.0f;

        generator_.seed(rand_seed_);

    }

    ~Impl() { }

    void create()
    {
        // Create excitory neuron group
        exc_id_ = sim_->createGroup(name_ + " exc", num_exc_, EXCITATORY_NEURON);
        sim_->setNeuronParameters(exc_id_, 0.02f, 0.2f, -65.0f, 8.0f); // RS
        // Create inhibitory neuron group
        inh_id_ = sim_->createGroup(name_ + " inh", num_inh_, INHIBITORY_NEURON);
        sim_->setNeuronParameters(inh_id_, 0.1f, 0.2f, -65.0f, 2.0f); // FS

        // Connect neurons inside the EXC group
        exc_exc_ = sim_->connect(exc_id_, exc_id_, "random", RangeWeight(max_weight_), pConn_, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);

        // Connect neurons inside the INH group
        inh_inh_ = sim_->connect(inh_id_, inh_id_, "random", RangeWeight(max_weight_), pConn_, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);

        // Connect neurons between EXC and INH group
        exc_inh_ = sim_->connect(exc_id_, inh_id_, "random", RangeWeight(max_weight_), pConn_, RangeDelay(1, 2), RadiusRF(-1), SYN_FIXED);

        // Connect neurons between INH and EXC group
        inh_exc_ = sim_->connect(inh_id_, exc_id_, "random", RangeWeight(max_weight_), pConn_, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);

    }

    int getExcID() { return exc_id_; }

    int getInhID() { return inh_id_; }

    int getGroupStartNID(int gID) { return sim_->getGroupStartNeuronId(gID); }

    int getGroupEndNID(int gID) { return sim_->getGroupEndNeuronId(gID); }

    void connectToReservoir()
    {
        sim_->connect(input_id_, exc_id_, "random", RangeWeight(max_weight_), pConn_, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);
        sim_->connect(input_id_, inh_id_, "random", RangeWeight(max_weight_), pConn_, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);
    }

    void randomizeWeights()
    {
        std::uniform_real_distribution<double> distribution (0.0,10.0);
        // randomize weights for exc to exc
        for (int i = 0; i < sim_->getGroupNumNeurons(exc_id_); i++) {
            for (int j = 0; j < sim_->getGroupNumNeurons(exc_id_); j++) {
                sim_->setWeight(exc_exc_, i, j, distribution(generator_));
            }
        }

        // randomize weights for inh to inh
        for (int i = 0; i < sim_->getGroupNumNeurons(inh_id_); i++) {
            for (int j = 0; j < sim_->getGroupNumNeurons(inh_id_); j++) {
                sim_->setWeight(inh_inh_, i, j, distribution(generator_));
            }
        }

        // randomize weights for exc to inh
        for (int i = 0; i < sim_->getGroupNumNeurons(exc_id_); i++) {
            for (int j = 0; j < sim_->getGroupNumNeurons(inh_id_); j++) {
                sim_->setWeight(exc_inh_, i, j, distribution(generator_));
            }
        }

        // randomize weights for inh to exc
        for (int i = 0; i < sim_->getGroupNumNeurons(inh_id_); i++) {
            for (int j = 0; j < sim_->getGroupNumNeurons(exc_id_); j++) {
                sim_->setWeight(inh_exc_, i, j, distribution(generator_));
            }
        }
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

    int rand_seed_;
    std::default_random_engine generator_;

    int exc_exc_;
    int inh_inh_;
    int exc_inh_;
    int inh_exc_;

    float max_weight_;

    float pConn_;
    int input_id_;
    int output_connID_;

    SpikeMonitor* spkMonExc_;
    SpikeMonitor* spkMonInh_;

};


Reservoir::Reservoir(CARLsim *sim, string name, int num_neurons, float exc_to_inh_ratio, float pConn, int rand_seed, int input_id)
:
_impl( new Impl(this, sim, name, num_neurons, exc_to_inh_ratio, pConn, rand_seed, input_id) ) {}

Reservoir::~Reservoir() { delete _impl; }

void Reservoir::create() { return _impl->create(); }

int Reservoir::getExcID() { return _impl->getExcID(); }

int Reservoir::getInhID() { return _impl->getInhID(); }

int Reservoir::getGroupStartNID(int gID) { return _impl->getGroupStartNID(gID); }

int Reservoir::getGroupEndNID(int gID) { return _impl->getGroupEndNID(gID); }

void Reservoir::connectToReservoir() { return _impl->connectToReservoir(); }

void Reservoir::randomizeWeights() { return _impl->randomizeWeights(); }

void Reservoir::setupMonitors() { return _impl->setupMonitors(); }

void Reservoir::startMonitors() { return _impl->startMonitors(); }

void Reservoir::stopMonitors() { return _impl->stopMonitors(); }

SpikeMonitor* Reservoir::getSpkMon(int s) { return _impl->getSpkMon(s); }