#ifndef __ATTENUATION_INTERFACE__
#define __ATTENUATION_INTERFACE__

namespace controls
{
    class AttenuationInterface
    {
    public:

        virtual float attenuate(float value) const = 0;
    };
}

#endif