/******************************************************************//**
* \brief   Implementation og generic interface for basic darwing.
* 
* \author  gernot
* \date    2018-03-15
* \version 1.0
**********************************************************************/
#pragma once
#ifndef OpenGLBasicDraw_h_INCLUDED
#define OpenGLBasicDraw_h_INCLUDED

// includes


// render

#include <Render_IDraw.h>
#include <Render_IBuffer.h>
#include <Render_IRenderPass.h>
#include <Render_ITexture.h>
#include <Render_IFont.h>
#include <Render_IRender.h>

// OpenGL

#include <OpenGLError.h>
#include <OpenGL_Matrix_Camera.h>
#include <OpenGL_SimpleShaderProgram_temp.h>

// stl

#include <memory>
#include <set>
#include <string>


// calss declarations

/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{


//---------------------------------------------------------------------
// CBasicDraw
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   Implementation for basic drawing.
*
* @author  gernot
* @date    2018-03-15
* @version 1.0
**********************************************************************/
class CBasicDraw
  : public Render::IDraw
  , public Render::IProvider
{

  // TODO $$$ unifrom blocks (model, view, projection)
  // TODO $$$ SSOA (3 frequences)
  // TODO $$$ text + greek letters: distance fields!
  // TODO $$$ post effects (cell (toon), sketch, gamma, hdr) - book of shaders
  // TOOO $$$ meshs
  // TODO $$$ input polyline
  // TODO $$$ draw arcs, curves (nurbs, spline) by tessellation shader
  // TODO $$$ orbit controll

public:

  enum TFontID : size_t
  {
    font_sans,
    font_symbol,
    font_pcifico,
    font_alura,
    font_grandhotel,
    font_greatevibes,
    font_pixslim_2
  };

  using TProgramPtr = OpenGL::ShaderProgram*;
  using TProcess    = std::unique_ptr<Render::IRenderProcess>;
  using TProgram    = std::unique_ptr<OpenGL::ShaderProgram>;
  using TFont       = std::unique_ptr<Render::IFont>; 
  using TFontMap    = std::map<size_t, TFont>;
  
  struct TUniforms
  {
    Render::TMat44 _projection = OpenGL::Identity();
    Render::TMat44 _view       = OpenGL::Identity();
    Render::TMat44 _model      = OpenGL::Identity();
    Render::TVec2  _vp_size    = { 0.0f };
  };


  CBasicDraw( bool core_mode, unsigned int samples, float scale, bool fxaa );
  virtual ~CBasicDraw();

  using Render::ITextureLoaderProvider::NewTextureLoader;

  virtual Render::IDrawBufferPtr      NewDrawBuffer( Render::TDrawBufferUsage usage ) override;
  virtual Render::IDrawBuffer       & DrawBuffer( void ) override;
  virtual Render::IDrawBuffer       & DrawBuffer( const void *key, bool &cached ) override;
  virtual Render::IRenderProcessPtr   NewRenderProcess( void ) override;
  virtual Render::ITextureLoaderPtr   NewTextureLoader( size_t loader_binding_id ) override;

  virtual const TMat44 & Projection( void ) const override { return _uniforms._projection; } //!< get the projection matrix
  virtual const TMat44 & View( void )       const override { return _uniforms._view; }       //!< get the view matrix
  virtual const TMat44 & Model( void )      const override { return _uniforms._model; }      //!< get the model matrix

    //!< sets the background color
  virtual void BackgroundColor( const Render::TColor &bg_color ) override
  {
    // TODO $$$ process change clear color
    _bg_color = bg_color;
    InvalidateProcess();
  };

  //!< sete the size of the viewport
  virtual void ViewportSize( const TSize &vp_size ) override
  { 
    _vp_size = vp_size;
    _uniforms._vp_size = { (float)vp_size[0], (float)vp_size[1] };
    InvalidateUniforms();
  }  

  //!< set the projection matrix
  virtual void Projection( const Render::TMat44 &proj ) override
  { 
    _uniforms._projection = proj;
    InvalidateUniforms();
  }

  //!< set the view matrix
  virtual void View( const Render::TMat44 &view ) override
  { 
    _uniforms._view = view;
    InvalidateUniforms();
  }          
  
  //!< set the model matrix
  virtual void Model( const Render::TMat44 &model ) override
  { 
    _uniforms._model =
    model; InvalidateUniforms(); 
  }        

  virtual TVec3 Project( const TVec3 &pt ) const override; //!< project by projection, view and model

  virtual void Destroy( void ) override;                                   //!< destroy all internal objects and cleanup
  virtual bool Init( void ) override;                                      //!< general initializations
  virtual bool LoadFont( TFontId font_id, Render::IFont *&font ) override; //!< load and return a font by its id
  virtual bool Begin( void ) override;                                     //!< start the rendering
  virtual bool ActivateBackground( void ) override;                        //!< activate rendering to background
  virtual bool ActivateOpaque( void ) override;                            //!< activate rendering to the opaque buffer
  virtual bool ActivateTransparent( void ) override;                       //!< activate rendering to the transparent buffer
  virtual bool Finish( void ) override;                                    //!< finish the rendering
  virtual bool ClearDepth( void ) override;                                //!< interim clear of the depth buffer
 
  virtual bool Draw( Render::TPrimitive primitive_type, size_t size, size_t coords_size, const Render::t_fp *coords, const Render::TColor &color, const TStyle &style ) override;

  virtual bool DrawText2D( TFontId font_id, const char *text, float height, float width_scale, const Render::TPoint3 &pos, const Render::TColor &color ) override;

private:
                                                        
  bool Multisample( void ) const { return _samples > 1; }
  bool EnableMultisample( bool enable );

  void InvalidateProcess( void );
  void InvalidateUniforms( void );

  bool SpecifyRenderProcess( void );
  bool UpdateGeneralUniforms( void );
  bool UpdateColorUniforms( const Render::TColor &color );
  bool SetModelUniform( const float *model );
  bool SetModelUniform( const TVec3 &scale, const TVec3 &p0, const TVec3 &px, const TVec3 &xz_plane );
  void DrawScereenspace( void );

  static std::set<std::string> _ogl_extensins;
  static int                   _max_anistropic_texture_filter;

  bool                                  _initialized    = false;
  bool                                  _drawing        = false;
  bool                                  _unifroms_valid = false;
  bool                                  _core_mode      = true;
  bool                                  _fxaa           = false;
  unsigned int                          _samples        = 0;
  float                                 _fb_scale       = 1.0f;
  size_t                                _current_pass   = 0;
  TProgramPtr                           _current_prog   = nullptr;
  const size_t                          _max_buffers    = 8;
  size_t                                _nextBufferI    = 0;
  std::array<const void*, 8>            _buffer_keys    { nullptr };
  std::array<Render::IDrawBufferPtr, 8> _draw_buffers   { nullptr };
  TSize                                 _vp_size        { 0 };
  Render::TColor                        _bg_color       { 0.0f };
  TUniforms                             _uniforms;
  TProcess                              _process;
  TProgram                              _opaque_prog;
  TProgram                              _opaque_line_prog;
  TProgram                              _transp_prog;
  TProgram                              _transp_line_prog;
  TProgram                              _mixcol_prog;
  TProgram                              _finish_prog;
  TFontMap                              _fonts;
  unsigned int                          _color_texture  = 0; // TODO $$$ ITexture
  unsigned int                          _uniform_ssbo   = 0; // TODO $$$ IUniform?

  const size_t c_opaque_pass = 1; //!< pass for opque drawing
  const size_t c_tranp_pass  = 2; //!< pass for tranparent drawing
  const size_t c_back_pass   = 3; //!< pass for drawing to background
  const size_t c_mixcol_pass = 4; //!< mix color pass (put it all together)
  const size_t c_finish_pass = 5; //!< final pass (fxaa)
};


} // OpenGL

#endif // OpenGLBasicDraw_h_INCLUDED
