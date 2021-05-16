#ifndef __ANIMATION_INTERFACE__H__
#define __ANIMATION_INTERFACE__H__

#include <math/glm_include.h>

namespace animation
{
    class AnimationInterface
    {
    public:

        virtual AnimationInterface& start(void) = 0;
        virtual AnimationInterface& stop(void) = 0;
        virtual AnimationInterface& update(glm::mat4&& base_matirx) = 0;
        virtual const glm::mat4& get_matrix(void) = 0;
    };
}

#endif