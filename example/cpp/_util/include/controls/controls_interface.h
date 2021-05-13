#ifndef __CONTROLS_INTERFACE__
#define __CONTROLS_INTERFACE__

namespace controls
{
    class ControlsInterface
    {
    public:

        virtual ControlsInterface& set_attenution(float constant, float linear, float quadratic) = 0; // TODO attenuation object
        virtual ControlsInterface& start_drag(const glm::vec2& position) = 0;
        virtual ControlsInterface& end_drag(const glm::vec2& position) = 0;
        virtual ControlsInterface& drag(const glm::vec2& position) = 0;
        virtual glm::mat4 update(void) = 0;
    };
}

#endif