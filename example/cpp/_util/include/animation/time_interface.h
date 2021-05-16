#ifndef __TIME_INTERFACE__H__
#define __TIME_INTERFACE__H__

namespace animation
{
    class TimeInterface
    {
    public:

        virtual double get_time(void) const = 0;
    };
}

#endif
