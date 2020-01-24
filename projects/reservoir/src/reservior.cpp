


#include "reservoir.h"

Reservoir::Reservoir(CARLsim *sim_, string name_, int num_neurons_, float exc_to_inh_ratio_, float pConn_, int input_id_)
{
    exc_to_inh_ratio = exc_to_inh_ratio_;
    num_neurons = num_neurons_;
    num_exc = (int) (num_neurons/(1+exc_to_inh_ratio)*exc_to_inh_ratio);
    num_inh = num_neurons - num_exc;
    sim = *sim_;
    pConn = pConn_;
    name = name_;
    input_id = input_id_;
}

void Reservoir::create()
{
    // Create excitory neuron group
    exc_id = sim.createGroup(name + " exc", num_exc, EXCITATORY_NEURON);
    sim.setNeuronParameters(exc_id, 0.02f, 0.2f, -65.0f, 8.0f); // RS
    // Create inhibitory neuron group
    inh_id = sim.createGroup(name + " inh", num_exc, INHIBITORY_NEURON);
    sim.setNeuronParameters(inh_id, 0.1f, 0.2f, -65.0f, 2.0f); // FS

    // Connect neurons inside the EXC group
    int exc_exc = sim.connect(exc_id, exc_id, "random", RangeWeight(0.0f, 6.0f, 10.0f), pConn, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);

    // Connect neurons inside the INH group
    int inh_inh = sim.connect(inh_id, inh_id, "random", RangeWeight(0.0f, 6.0f, 10.0f), pConn, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);

    // Connect neurons between EXC and INH group
    int exc_inh = sim.connect(exc_id, inh_id, "random", RangeWeight(0.0f, 6.0f, 10.0f), pConn, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);

    // Connect neurons between INH and EXC group
    int inh_exc = sim.connect(inh_id, exc_id, "random", RangeWeight(0.0f, 6.0f, 10.0f), pConn, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);
}

int Reservoir::getExcID() { return exc_id; }

int Reservoir::getInhID() { return inh_id; }

int Reservoir::getGroupStartNID(int gID) { return sim.getGroupStartNeuronId(gID); }

int Reservoir::getGroupEndNID(int gID) { return sim.getGroupEndNeuronId(gID); }

void Reservoir::connectToReservoir()
{
    sim.connect(input_id, exc_id, "random", RangeWeight(0.0f, 6.0f, 10.0f), pConn, RangeDelay(1, 20), RadiusRF(-1), SYN_FIXED);
}