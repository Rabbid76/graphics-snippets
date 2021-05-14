#ifndef __TRANSFORMATION_MATRIX__
#define __TRNASFORMATION_MATRIX__

#include <math/glm_include.h>

namespace controls
{
    class TransformationMatrix
    {
    private:

        glm::mat4 _matrix{ glm::mat4(1.0f) };
        glm::mat4 _curren_matrix{ glm::mat4(1.0f) };
        bool current_transtormation = false;

    public:

        TransformationMatrix& update(void)
        {
            _matrix = _curren_matrix * _matrix;
            _curren_matrix = glm::mat4(1.0f);
            current_transtormation = false;
            return *this;
        }

        TransformationMatrix& apply_transformation(glm::mat4&& matrix)
        {
            _curren_matrix = matrix;
            current_transtormation = true;
            return *this;
        }

        glm::mat4 get_transformation(void) const
        {
            return current_transtormation
                ? _curren_matrix * _matrix
                : _matrix;
        }
    };
}

#endif
