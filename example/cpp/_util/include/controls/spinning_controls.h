#ifndef __SPINNING_CONTROLS__
#define __SPINNING_CONTROLS__

#include <controls/controls_interface.h>
#include <controls/controls_view_interface.h>
#include <controls/transformation_matrix.h>
#include <controls/drag_operation.h>
#include <controls/attenuation_interface.h>
#include <math/glm_include.h>

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
        DragOperation _drag_operation;
        TransformationMatrix _model_transformation;
        TransformationMatrix _orbit_transformation;
        std::shared_ptr<AttenuationInterface> _attenuation;

        bool _active = true;
        bool _hit = false;
        bool _auto_spin = false;
        bool _auto_rotate = true;
        
        // TODO auto rotate object
        bool _auto_rotate_mode = true;
        double _rotate_start_time = 0.0;

        glm::vec2 _position{ glm::vec2(0.0f) };
        glm::vec2 _start_position{ glm::vec2(0.0f) };

    public:

        SpinningControls(const ControlsViewInterface& view);

        glm::mat4 get_orbit_matrix(void) const;
        glm::mat4 get_auto_model_matrix(void) const;

        bool auto_rotate(void) const { return _auto_rotate; }
        bool auto_spin(void) const { return _auto_spin; }

        virtual SpinningControls& set_attenution(std::shared_ptr<AttenuationInterface> attenuation) override
        { 
            _attenuation = attenuation;
            return *this;
        }

        virtual SpinningControls& start_drag(const glm::vec2& position) override;
        virtual SpinningControls& end_drag(const glm::vec2& position) override;
        virtual SpinningControls& drag(const glm::vec2& position) override;
        virtual glm::mat4 update(void) override;

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