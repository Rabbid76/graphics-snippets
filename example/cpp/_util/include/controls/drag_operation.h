#ifndef __DRAG_OPERATION__H__
#define __DRAG_OPERATION__H__

#include <math/glm_include.h>

namespace controls
{
    class DragOperation
    {
    private:

        bool _is_dragging = false;
        double _angle_scale = 1.0f;
        float _angle = 0.0f;
        double _start_time = 0.0;
        double _time = 0.0;
        glm::vec3 _axis{ glm::vec3(0.0f, 0.0f, 1.0f) };
        glm::vec2 _start_position{ glm::vec3(0.0f) };

    public:

        DragOperation(double angle_scale)
            : _angle_scale(angle_scale)
        {}

        bool is_dragging(void) const { return _is_dragging; }
        double time(void) const { return _time; }
        double angle(void) const { return _angle; }
        glm::vec3 axis(void) const { return _axis; }

        glm::mat3 get_roatation(void) const
        {
            return glm::rotate(glm::mat4(1.0f), _angle, _axis);
        }

        DragOperation& set_start_positon(const glm::vec2& position)
        {
            _start_position = position;
            return *this;
        }

        DragOperation& start(double time)
        {
            _is_dragging = true;
            _start_time = time;
            _angle = 0;
            _time = 0;
            return *this;
        }

        DragOperation& end(void)
        {
            _is_dragging = false;
            return *this;
        }

        DragOperation& update(
            const glm::vec2& position, 
            const TViewportRectangle& viewport_rectangle,
            const glm::mat4& inverse_view,
            double current_time)
        {
            if (!_is_dragging)
                return *this;

            auto distance = position - _start_position;
            auto viewport_diagonal = glm::vec2(
                viewport_rectangle[2] - viewport_rectangle[0],
                viewport_rectangle[3] - viewport_rectangle[2]);

            float dx = distance.x / viewport_diagonal.x;
            float dy = distance.y / viewport_diagonal.y;
            float len_2 = dx * dx + dy * dy;
            if (len_2 <= 0.0)
                return *this;

            float len = std::sqrt(len_2);
            _angle = _angle_scale * len;
            _axis = glm::mat3(inverse_view) * glm::vec3(-dy / len, dx / len, 0);
            _time = current_time - _start_time;

            return *this;
        }
    };
}

#endif