/******************************************************************//**
* \brief   Freetype library font implementation.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/
#pragma once
#ifndef RenderUtil_FreetypeFont_h_INCLUDED
#define RenderUtil_FreetypeFont_h_INCLUDED


// includes

#include <Render_IFont.h>
#include <Render_ITexture.h>


/******************************************************************//**
* @brief   Namespace for renderer.
*
* @author  gernot
* @date    2018-03-17
* @version 1.0
**********************************************************************/
namespace Render
{


//---------------------------------------------------------------------
// CFreetypeTextureText
//---------------------------------------------------------------------


struct TFreetypeTFont;


/******************************************************************//**
* @brief   Textured text implementation with freetype library.
*
* @author  gernot
* @date    2018-03-18
* @version 1.0
**********************************************************************/
class CFreetypeTexturedFont
  : public Render::IFont
{
public:

  using TTexturePtr = Render::ITexturePtr;
  using TFontPtr = std::unique_ptr<TFreetypeTFont>;

  CFreetypeTexturedFont( const char *font_filename, int min_char );
  virtual ~CFreetypeTexturedFont();

  virtual void Destroy( void ) override;                        //!< destroy all internal objects and cleanup
  virtual bool Load( Render::ITextureLoader &loader ) override; //!< load the glyphs

  //! calculates box of a string in relation to its height (maximum height of the font from the bottom to the top)
  virtual bool CalculateTextSize( const char *str, float height, float &box_x, float &box_btm, float &box_top ) override;

  //! render a texture based text
  virtual bool DrawText( Render::IDrawBufferProvider &buffer_provider, size_t textur_binding_id, const char *str, float height, float width_scale, const Render::TPoint3 &pos ) override;

private:

  void DebugFontTexture( Render::IDrawBufferProvider &buffer_provider, size_t textur_binding_id );

  std::string  _font_filename;
  int          _min_char      = 32;
  TFontPtr     _font;
  bool         _valid         = true;
  TTexturePtr  _font_texture;
};



} // Render

#endif // RenderUtil_FreetypeFont_h_INCLUDED