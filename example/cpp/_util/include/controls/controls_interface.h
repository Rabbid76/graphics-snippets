#ifndef __CONTROLS_INTERFACE__H__
#define __CONTROLS_INTERFACE__H__

#include <controls/attenuation_interface.h>
#include <math/glm_include.h>

namespace controls
{
    class ControlsInterface
    {
    public:

        virtual ControlsInterface& set_attenution(std::shared_ptr<AttenuationInterface> attenuation) = 0;
        virtual ControlsInterface& start_drag(const glm::vec2& position) = 0;
        virtual ControlsInterface& end_drag(const glm::vec2& position) = 0;
        virtual ControlsInterface& drag(const glm::vec2& position) = 0;
        virtual glm::mat4 update(void) = 0;
    };
}

#endif