#pragma once

#include <stdint.h>
#include <algorithm>

class TYPE3Regulator
{
public:
    TYPE3Regulator(float wz1, float wz2, float wp0, float wp2, float wp3, float dt, float min_output, float max_output)
        : dt(dt), min_output(min_output), max_output(max_output)
    {
        a1 = -(-12 + dt * dt * wp2 * wp3 - 2 * dt * (wp2 + wp3))
             / ((2 + dt * wp2) * (2 + dt * wp3));
        a2 = (-12 + dt * dt * wp2 * wp3 + 2 * dt * (wp2 + wp3))
             / ((2 + dt * wp2) * (2 + dt * wp3));
        a3 = (-2 + dt * wp2) * (-2 + dt * wp3)
             / ((2 + dt * wp2) * (2 + dt * wp3));
        b0 = (dt * wp0 * wp2 * wp3 * (2 + dt * wz1) * (2 + dt * wz2))
             / (2 * (2 + dt * wp2) * (2 + dt * wp3) * wz1 * wz2);
        b1 = (dt * wp0 * wp2 * wp3 * (-4 + 3 * dt * dt * wz1 * wz2 + 2 * dt * (wz1 + wz2)))
             / (2 * (2 + dt * wp2) * (2 + dt * wp3) * wz1 * wz2);
        b2 = (dt * wp0 * wp2 * wp3 * (-4 + 3 * dt * dt * wz1 * wz2 - 2 * dt * (wz1 + wz2)))
             / (2 * (2 + dt * wp2) * (2 + dt * wp3) * wz1 * wz2);
        b3 = (dt * wp0 * wp2 * wp3 * (-2 + dt * wz1) * (-2 + dt * wz2))
             / (2 * (2 + dt * wp2) * (2 + dt * wp3) * wz1 * wz2);
    }

    void reset()
    {
        x[0] = x[1] = x[2] = x[3] = 0.0f;
        y[0] = y[1] = y[2] = y[3] = 0.0f;
    }

    float operator()(float error)
    {
        x[3] = x[2];
        x[2] = x[1];
        x[1] = x[0];
        x[0] = error;

        y[3] = y[2];
        y[2] = y[1];
        y[1] = y[0];

        float u = b0 * x[0] + b1 * x[1] + b2 * x[2] + b3 * x[3];
        y[0] = u + a1 * y[1] + a2 * y[2] + a3 * y[3];
        y[0] = std::clamp(y[0], min_output, max_output);

        return y[0];
    }

    void set_actual_output(float y)
    {
        this->y[0] = y;
    }

private:
    float a1, a2, a3;
    float b0, b1, b2, b3;

    float x[4] = {0};
    float y[4] = {0};
    float dt;
    float min_output;
    float max_output;
};
