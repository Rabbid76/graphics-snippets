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
        bool _active = false;
        bool _hit = false;
        bool _auto_spin = false;
        bool _auto_rotate = true;
        
        // TODO auto rotate object
        bool _auto_rotate_mode = false;
        bool _auto_roatate_active = false;
        double _rotate_start_time = 0.0;

        // TODO attenuation object;
        std::array<float, 3> _attenuation{ 0.0, 0.0, 0.0 };

        // TODO: drag object
        bool _drag = false;
        float _drag_angle = 0.0f;
        double _drag_start_time = 0.0;
        double _drag_time = 0.0;
        glm::vec3 _drag_axis{ glm::vec3(0.0f) };
        glm::vec2 _position{ glm::vec2(0.0f) };
        glm::vec2 _start_position{ glm::vec2(0.0f) };

        // TODO current matrix object
        glm::mat4 _model_matrix{ glm::mat4(1.0f) };
        glm::mat4 _orbit_matrix{ glm::mat4(1.0f) };
        glm::mat4 _current_model_matrix{ glm::mat4(1.0f) };
        glm::mat4 _current_orbit_matrix{ glm::mat4(1.0f) };

    public:

        SpinningControls(const ControlsViewInterface& view);

        glm::mat4 get_orbit_matrix(void) const;
        glm::mat4 get_auto_model_matrix(void) const;

        bool auto_rotate(void) const { return _auto_rotate; }
        bool auto_spin(void) const { return _auto_spin; }

        SpinningControls& set_attenution(std::array<float, 3>&& attenuation) { _attenuation = attenuation; }

        SpinningControls& start_drag(const glm::vec2& position);
        SpinningControls& end_drag(const glm::vec2& position);
        SpinningControls& drag(const glm::vec2& position);
        glm::mat4 update(void);

    private:
        
        SpinningControls& update_model(void);
        SpinningControls& change_motion_mode(bool drag, bool spin, bool automaticall);
        SpinningControls& update_position(const glm::vec2& position);
        SpinningControls& start_rotate(const glm::vec2& position);
        SpinningControls& finish_rotate(const glm::vec2& position);
        SpinningControls& toggle_rotate(void);
    };
}

#endif