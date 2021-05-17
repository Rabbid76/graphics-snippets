#include <pch.h>

#include <animation/rotation_animation.h>
#include <controls/spinning_controls.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

namespace controls
{
    SpinningControls::SpinningControls(const animation::TimeInterface& time, const ControlsViewInterface& view)
        : _time{ time }
        , _view{ view }
        , _drag_operation(M_PI)
    {
        // TODO no animation
        // TODO concatenated animation
        // TODO composite animation builder
        /*
        float auto_angle_x = static_cast<float>(delta_time / 13.0 * 2.0 * M_PI);
            float auto_angle_y = static_cast<float>(delta_time / 17.0 * 2.0 * M_PI);
            _transformation.apply_transformation(
                glm::rotate(
                    glm::rotate(glm::mat4(1.0f), auto_angle_x, glm::vec3(1.0f, 0.0f, 0.0f)),
                    auto_angle_y, glm::vec3(0.0f, 1.0f, 0.0f)));
        */
        _animation = std::make_unique<animation::RotationAnimation>(_time, static_cast<float>(1.0 / 13.0 * 2.0 * M_PI), glm::vec3(1.0f, 0.7f, 0.0f), nullptr);
        _animation->start();
    }

    SpinningControls& SpinningControls::start_drag(const glm::vec2& position)
    {
        _position = position;
        _hit = false;
        if (_auto_rotate_mode)
            start_rotate(position);
        else
            _hit = true;
        return *this;
    }

    SpinningControls& SpinningControls::end_drag(const glm::vec2& position)
    {
        _position = position;
        _hit = false;
        if (_active && _auto_rotate_mode)
            finish_rotate(position);

        return *this;
    }

    SpinningControls& SpinningControls::drag(const glm::vec2& position)
    {
        if (!_active && !_hit)
            return *this;
        return update_position(position);
    }

    glm::mat4 SpinningControls::update(void)
    {
        update_model();
        return get_tranformation_matrix();
    }

    SpinningControls& SpinningControls::update_model(void)
    {
        if (_drag_operation.is_dragging())
        {
            _transformation.apply_transformation(_drag_operation.get_roatation());
            return *this;
        }

        if (_animation)
        {
            _animation->update(glm::mat4(1));
            _transformation.apply_transformation(glm::mat4(_animation->get_matrix()));
            return *this;
        }

        _transformation.apply_transformation(glm::mat4(1.0f));
        return *this;
    }

    SpinningControls& SpinningControls::change_motion_mode(bool drag, bool spin, bool automatically)
    {
        bool new_drag = drag;
        bool new_auto = new_drag ? false : automatically;
        bool new_spin = new_auto ? spin : false;

        if (_drag_operation.is_dragging() == new_drag && _auto_rotate == new_auto && _auto_spin == new_spin)
            return *this;
       
        if (new_drag != _drag_operation.is_dragging())
        {
            if (new_drag)
            {
                _drag_operation.start(_time.get_time());
                if (_animation)
                    _animation->stop();
            }
            else
                _drag_operation.end();
        }

        if (new_auto && !_auto_rotate)
        {
            if (new_spin && _drag_operation.time() > 0)
            {
                 _animation = std::make_unique<animation::RotationAnimation>(_time, _drag_operation.angle(), glm::vec3(_drag_operation.axis()), _attenuation);
                _animation->start();
            }
            else
                _animation.reset();
        }

        _auto_rotate = new_auto;
        _auto_spin = new_spin;
        _transformation.update();
        
        return *this;
    }

    SpinningControls& SpinningControls::update_position(const glm::vec2& position)
    {
        _drag_operation.update(
            position,
            _view.get_viewport_rectangle(),
            _view.get_inverse_view_matrix(),
            _time.get_time());
        return *this;
    }

    SpinningControls& SpinningControls::start_rotate(const glm::vec2& position)
    {
        _start_position = position;
        _drag_operation.set_start_positon(position);
        return change_motion_mode(true, false, false);
    }

    SpinningControls& SpinningControls::finish_rotate(const glm::vec2& position)
    {
        _position = position;
        update_position(position);
        return change_motion_mode(false, true, true);
    }

    SpinningControls& SpinningControls::toggle_rotate(void)
    {
        return change_motion_mode(false, true, !_auto_rotate);
    }
}
