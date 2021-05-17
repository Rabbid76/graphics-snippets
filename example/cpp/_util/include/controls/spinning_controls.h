#ifndef __SPINNING_CONTROLS__H__
#define __SPINNING_CONTROLS__H__

#include <animation/time_interface.h>
#include <animation/animation_interface.h>
#include <animation/attenuation_interface.h>
#include <controls/controls_interface.h>
#include <controls/controls_view_interface.h>
#include <controls/transformation_matrix.h>
#include <controls/drag_operation.h>
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

        const animation::TimeInterface& _time;
        const ControlsViewInterface& _view;
        std::unique_ptr<animation::AnimationInterface> _animation;
        DragOperation _drag_operation;
        TransformationMatrix _transformation;
        std::shared_ptr<animation::AttenuationInterface> _attenuation;
        bool _auto_rotate_mode = true;
        bool _active = true;
        bool _hit = false;
        bool _auto_spin = false;
        bool _auto_rotate = true;
        glm::vec2 _position{ glm::vec2(0.0f) };
        glm::vec2 _start_position{ glm::vec2(0.0f) };

    public:

        SpinningControls(const animation::TimeInterface& time, const ControlsViewInterface& view);

        glm::mat4 get_tranformation_matrix(void) const { return _transformation.get_transformation(); }

        virtual SpinningControls& set_attenution(std::shared_ptr<animation::AttenuationInterface> attenuation) override
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