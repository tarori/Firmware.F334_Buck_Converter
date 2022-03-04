#include <stdint.h>
#include <algorithm>

class PIDRegulator
{
public:
    PIDRegulator(float p_gain, float i_gain, float d_gain, float dt, float min_output, float max_output) : p_gain(p_gain), i_gain(i_gain), d_gain(d_gain), dt(dt), min_output(min_output), max_output(max_output)
    {
    }

    void reset()
    {
        integ = 0.0f;
    }

    float operator()(float data)
    {
        float output_p = p_gain * data;
        float output_i = i_gain * integ;
        float output_d = d_gain * (data - prev) / dt;
        float output = output_p + output_i + output_d;
        if (output < max_output && output > min_output) {
            integ += data * dt;
        }
        prev = data;
        return std::clamp(output, min_output, max_output);
    }

private:
    float p_gain;
    float i_gain;
    float d_gain;
    float dt;
    float min_output;
    float max_output;

    float integ = 0;
    float prev = 0;
};
