#ifndef __ROTATION_ANIMATION__H__
#define __ROTATION_ANIMATION__H__

#include <animation/animation_interface.h>
#include <animation/time_interface.h>
#include <math/glm_include.h>

namespace animation
{
    class RotationAnimation 
        : public AnimationInterface
    {
    private:

        const animation::TimeInterface& _time;
        bool _active = false;
        double _start_time = 0.0;
        double _angle_per_time;
        glm::vec3 _axis;
        glm::mat4 _rotation_matrix{ glm::mat4(1.0f) };

    public:

        RotationAnimation(const RotationAnimation&) = default;
        RotationAnimation(RotationAnimation&&) = default;

        RotationAnimation(const animation::TimeInterface& time, double angle_per_time, glm::vec3&& axis)
            : _time(time)
            , _angle_per_time(angle_per_time)
            , _axis(axis)
        {}

        virtual AnimationInterface& start(void) override
        {
            _active = true;
            return *this;
        }

        virtual AnimationInterface& stop(void) override
        {
            _active = false;
            _start_time = _time.get_time();
            return *this;
        }

        virtual AnimationInterface& update(glm::mat4&& base_matirx) override
        {
            if (!_active)
                return *this;

            double current_time = _time.get_time();
            double delta_time = current_time - _start_time;
            float angle = static_cast<float>(delta_time * _angle_per_time);
            _rotation_matrix = glm::rotate(base_matirx, angle, _axis);
            return *this;
        }

        virtual const glm::mat4& get_matrix(void) override
        {
            return _rotation_matrix;
        }
    };
}

#endif
