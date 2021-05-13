#include <pch.h>

#include <controls/spinning_controls.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

namespace controls
{
    SpinningControls::SpinningControls(const ControlsViewInterface& view)
        : _view{ view }
    {}

    glm::mat4 SpinningControls::get_orbit_matrix(void) const
    {
        return (_drag || (_auto_rotate && _auto_spin)) ? _current_orbit_matrix * _orbit_matrix : _orbit_matrix;
    }

    glm::mat4 SpinningControls::get_auto_model_matrix(void) const
    {
        return _auto_rotate ? _current_model_matrix * _model_matrix : _model_matrix;
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
        return get_orbit_matrix() * get_auto_model_matrix();
    }

    SpinningControls& SpinningControls::update_model(void)
    {
        
        _current_model_matrix = glm::mat4(1.0f);

        if (_drag)
        {
            _current_orbit_matrix = glm::rotate(glm::mat4(1.0f), _drag_angle, _drag_axis);
            return *this;
        }

        if (!_auto_rotate)
            return *this;

        double current_time = _view.get_time();
        double delta_time = current_time - _rotate_start_time;
        if (!_auto_spin)
        {
            float auto_angle_x = static_cast<float>(delta_time / 13.0 * 2.0 * M_PI);
            float auto_angle_y = static_cast<float>(delta_time / 17.0 * 2.0 * M_PI);
            _current_model_matrix = glm::rotate(
                glm::rotate(glm::mat4(1.0f), auto_angle_x, glm::vec3(1.0f, 0.0f, 0.0f)),
                auto_angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
            return *this;
        }

        if (_drag_time > 0)
        {
            float angle = static_cast<float>(delta_time * _drag_angle / _drag_time);
            if (std::fabs(_attenuation[0]) > 0)
                angle /= _attenuation[0] + _attenuation[1] * angle + _attenuation[2] * angle * angle;
            _current_orbit_matrix = glm::rotate(glm::mat4(1.0f), angle, _drag_axis);
        }

        return *this;
    }

    SpinningControls& SpinningControls::change_motion_mode(bool drag, bool spin, bool automatically)
    {
        bool new_drag = drag;
        bool new_auto = new_drag ? false : automatically;
        bool new_spin = new_auto ? spin : false;

        if (_drag == new_drag && _auto_rotate == new_auto && _auto_spin == new_spin)
            return *this;
       
        if (new_drag && !_drag)
        {
            _drag_start_time = _view.get_time();
            _drag_angle = 0;
            _drag_time = 0;
        }

        if (new_auto && !_auto_rotate)
            _rotate_start_time = _view.get_time();

        _drag = new_drag;
        _auto_rotate = new_auto;
        _auto_spin = new_spin;
        _orbit_matrix = _current_orbit_matrix * _orbit_matrix;
        _current_orbit_matrix = glm::mat4(1.0f);
        _model_matrix = _current_model_matrix * _model_matrix;
        _current_model_matrix = glm::mat4(1.0f);
        
        return *this;
    }

    SpinningControls& SpinningControls::update_position(const glm::vec2& position)
    {
        if (_drag == false)
            return *this;

        _position = position;
        auto viewport_rectangle = _view.get_viewport_rectangle();
        auto distance = _position - _start_position;
        auto viewport_diagonal = glm::vec2(
            viewport_rectangle[2] - viewport_rectangle[0], 
            viewport_rectangle[3] - viewport_rectangle[2]);

        float dx = distance.x / viewport_diagonal.x;
        float dy = distance.y / viewport_diagonal.y;
        float len_2 = dx * dx + dy * dy;
        if (len_2 <= 0.0)
            return *this;

        float len = std::sqrt(len_2);
        _drag_angle = M_PI * len;
        glm::mat4 inverse_view = _view.get_inverse_view_matrix();
        _drag_axis = glm::mat3(inverse_view) * glm::vec3(-dy / len, dx / len, 0);
        _drag_time = _view.get_time() - _drag_start_time;
        
        return *this;
    }

    SpinningControls& SpinningControls::start_rotate(const glm::vec2& position)
    {
        _start_position = position;
        return change_motion_mode(true, false, false);
    }

    SpinningControls& SpinningControls::finish_rotate(const glm::vec2& position)
    {
        update_position(position);
        return change_motion_mode(false, true, true);
    }

    SpinningControls& SpinningControls::toggle_rotate(void)
    {
        return change_motion_mode(false, true, !_auto_rotate);
    }
}
