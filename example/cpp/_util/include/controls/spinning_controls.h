#ifndef __SPINNING_CONTROLS__
#define __SPINNING_CONTROLS__

#include <controls/controls_interface.h>
#include <controls/controls_view_interface.h>

#include <memory>
#include <chrono>

namespace controls
{
    class SpinningControls :
        public ControlsInterface
    {
    public:

        using TTime = std::chrono::high_resolution_clock::time_point;

    private:

        const ControlsViewInterface& _view;

    public:

       
    };
}

#endif