#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include "main.h"
#include <cstddef>
#ifdef PI
#undef PI
#endif
class ModelListener
{
public:
    ModelListener() : model(0) {}
    
    virtual ~ModelListener() {}
    virtual void NotifvADCValueChanged(uint16_t Ch1,uint16_t Ch2,uint16_t Ch3,uint16_t Ch4) {}
    virtual void NotifyFrequencyMeasured(uint32_t frequencyHz) {}
    virtual void NotifyWaveform(uint32_t frequencyHz, float vppMv, const uint16_t* samples, size_t sampleCount) {}
    void bind(Model* m)
    {
        model = m;
    }
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
