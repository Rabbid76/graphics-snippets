/******************************************************************//**
* \brief OpenGL `std140` layout Uniform Block implementations.    
* 
*  *std140*
* 
* [OpenGL 4.6 API Core Profile Specification; 7.6.2.2 Standard Uniform Block Layout; page 144](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.core.pdf),  
* [OpenGL 4.6 API Compatibility Profile Specification; 7.6.2.2 Standard Uniform Block Layout; page 145](https://www.khronos.org/registry/OpenGL/specs/gl/glspec46.compatibility.pdf)  
* [OpenGL ES 3.2 Specification; 7.6.2.2 Standard Uniform Block Layout; page 113](https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf):
* 
* > When using the `std140` storage layout, structures will be laid out in buffer storage with their members stored in monotonically increasing order based on their location in the declaration.
* A structure and each structure member have a base offset and a base alignment, from which an aligned offset is computed by rounding the base offset up to a multiple of the base alignment.
* The base offset of the first member of a structure is taken from the aligned offset of the structure itself.
* The base offset of all other structure members is derived by taking the offset of the last basic machine unit consumed by the previous member and adding one. 
* Each structure member is stored in memory at its aligned offset. The members of a toplevel uniform block are laid out in buffer storage by treating the uniform block as a structure with a base offset of zero.
* >
* > 1. If the member is a scalar consuming N basic machine units, the base alignment is N.
* > 2. If the member is a two- or four-component vector with components consuming N basic machine units, the base alignment is 2N or 4N, respectively.
* > 3. If the member is a three-component vector with components consuming N basic machine units, the base alignment is 4N.
* > 4. If the member is an array of scalars or vectors, the base alignment and array stride are set to match the base alignment of a single array element, according to rules (1), (2), and (3), and rounded up to the base alignment of a `vec4`.
* >    The array may have padding at the end; the base offset of the member following the array is rounded up to the next multiple of the base alignment.
* > 5. If the member is a column-major matrix with C columns and R rows, the matrix is stored identically to an array of C column vectors with R components each, according to rule (4).
* > 6. If the member is an array of S column-major matrices with C columns and R rows, the matrix is stored identically to a row of S × C column vectors with R components each, according to rule (4).
* > 7. If the member is a row-major matrix with C columns and R rows, the matrix is stored identically to an array of R row vectors with C components each, according to rule (4).
* > 8. If the member is an array of S row-major matrices with C columns and R rows, the matrix is stored identically to a row of S × R row vectors with C components each, according to rule (4).
* > 9. If the member is a structure, the base alignment of the structure is N, where N is the largest base alignment value of any of its members, and rounded up to the base alignment of a `vec4`.
* >    The individual members of this substructure are then assigned offsets by applying this set of rules recursively, where the base offset of the first member of the sub-structure is equal to the aligned offset of the structure.
* >    The structure may have padding at the end; the base offset of the member following the sub-structure is rounded up to the next multiple of the base alignment of the structure.
* > 10. If the member is an array of S structures, the S elements of the array are laid out in order, according to rule (9).
* 
* \author  gernot
* \date    2018-12-17
* \version 1.0
**********************************************************************/


#include <stdafx.h>

// includes


#include <OpenGLDataBuffer_std140.h>


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-12-05
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief ctor.  
* 
* \author  gernot
* \date    2018-12-17
* \version 1.0
**********************************************************************/
CModelAndViewBuffer_std140::CModelAndViewBuffer_std140( void )
{}


/******************************************************************//**
* \brief dtor.  
* 
* \author  gernot
* \date    2018-12-17
* \version 1.0
**********************************************************************/
CModelAndViewBuffer_std140::~CModelAndViewBuffer_std140()
{}


/******************************************************************//**
* \brief Initialization of the object.   
* 
* \author  gernot
* \date    2018-12-17
* \version 1.0
**********************************************************************/
bool CModelAndViewBuffer_std140::Init( void )
{
  if ( _initialized )
    return false;

  auto data_resource = std::make_shared<Render::CModelAndView>();
  return Init(data_resource);
}


/******************************************************************//**
* \brief Initialization of the object.   
* 
* \author  gernot
* \date    2018-12-17
* \version 1.0
**********************************************************************/
bool CModelAndViewBuffer_std140::Init( 
  Render::TModelAndViewPtr data_ptr ) //!< I - data resource pointer 
{
  // TODO $$$

  // _initialized = true;
  return false;
}


/******************************************************************//**
* \brief Destroy the object resources.  
* 
* \author  gernot
* \date    2018-12-17
* \version 1.0
**********************************************************************/
void CModelAndViewBuffer_std140::Destroy( void )
{
  // TODO $$$
}


} // OpenGL