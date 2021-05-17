#ifndef __ATTENUATION_INTERFACE__H__
#define __ATTENUATION_INTERFACE__H__

namespace animation
{
    class AttenuationInterface
    {
    public:

        virtual float attenuate(float value) const = 0;
    };
}

#endif