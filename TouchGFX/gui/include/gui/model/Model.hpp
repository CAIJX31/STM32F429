#ifndef MODEL_HPP
#define MODEL_HPP

#include <stdint.h>
#include <cstddef>

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();
protected:
    ModelListener* modelListener;
    ModelListener* lastFrequencyListener;
    uint32_t lastFrequencyHz;
    float lastPeakToPeakMv;
};

#endif // MODEL_HPP
