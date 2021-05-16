#ifndef __QUADRATIC_ATTENUATION__H__
#define __QUADRATIC_ATTENUATION__H__

#include <controls/attenuation_interface.h>

#include <array>
#include <cmath>

namespace controls
{
    class QudraticAttenuation
        : public AttenuationInterface
    {
    private:

        std::array<float, 3> _attenuation{ 0.0, 0.0, 0.0 };

    public:

        QudraticAttenuation(float constant, float linear, float quadratic)
            : _attenuation{ constant, linear, quadratic }
        {}


        virtual float attenuate(float value) const override
        {
            return std::fabs(_attenuation[0]) > 0
                ? value / (_attenuation[0] + _attenuation[1] * value + _attenuation[2] * value * value)
                : value;
        }
    };
}

#endif
