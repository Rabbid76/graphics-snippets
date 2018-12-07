/******************************************************************//**
* \brief   Freetype library font implementation.
* 
* \author  gernot
* \date    2017-11-20
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <RenderUtil_FreetypeFont.h>
#include <Render_IBuffer.h>


// FREETYPE

#include <ft2build.h>
#include <freetype/ftstroke.h>
#include FT_FREETYPE_H


// STL

#include <vector>
#include <iostream>


// preprocessor definitions

#if defined(max)
#undef max
#endif

#if defined(min)
#undef min
#endif


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


/******************************************************************//**
* \brief   FreeType glyph metrics
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
struct TFreetypeGlyph
{
  FT_Glyph_Metrics           _metrics { 0 }; //!< glyph metrics
  unsigned int               _x       = 0;   //!< glyph start x
  unsigned int               _y       = 0;   //!< glyph start y
  unsigned int               _cx      = 0;   //!< glyph width
  unsigned int               _cy      = 0;   //!< glyph height
  std::vector<unsigned char> _image;         //!< image data
};

/******************************************************************//**
* @brief   FreeType font data 
*
* @author  gernot
* @date    2018-03-18
* @version 1.0
**********************************************************************/
struct TFreetypeTFont
{
  FT_Library                  _hdl     = nullptr; //!< library handle
  FT_Face                     _face    = nullptr; //!< font face
  FT_Stroker                  _stroker = nullptr; //!< Glyph Stroker - [https://www.freetype.org/freetype2/docs/reference/ft2-glyph_stroker.html#FT_Stroker]
  unsigned int                _width         = 0; //!< total length 
  unsigned int                _max_height    = 0; //!< maximum height
  int                         _max_glyph_cy  = 0; //!< maximum glyph metrics height 
  int                         _max_glyph_y   = 0; //!< maximum glyph metrics bearing y 
  int                         _min_char      = 0; //!< minimum character
  int                         _max_char      = 0; //!< maximum character
  std::vector<TFreetypeGlyph> _glyphs;           //!< glyph information
};


/******************************************************************//**
* \brief   
* 
* \author  gernot
* \date    2018-06-08
* \version 1.0
**********************************************************************/
class TGlyphImage
  : public Render::IImageResource
{
public:

  TGlyphImage( const TFreetypeGlyph &glyph )
    : _glyph( glyph )
  {}

  virtual Render::TImageKind   Kind( void )      const override { return Render::TImageKind::diffuse; }
  virtual Render::TTextureType Type( void )      const override { return Render::TTextureType::T2D; }
  virtual Render::TImageFormat Format( void )    const override { return Render::TImageFormat::RGBA8; }
  virtual Render::TTextureSize Size( void )      const override { return {_glyph._cx, _glyph._cy, 0}; }
  virtual size_t               Layers( void )    const override { return 1; }
  virtual size_t               BPL( void )       const override { return 4*_glyph._cx; }
  virtual size_t               LineAlign( void ) const override { return 4; }
  virtual const void *         DataPtr()         const override { return _glyph._image.data(); }                                 

private:

  const TFreetypeGlyph &_glyph;
};


/******************************************************************//**
* @brief   ctor.
*
* @author  gernot
* @date    2018-03-18
* @version 1.0
**********************************************************************/
CFreetypeTexturedFont::CFreetypeTexturedFont( 
  const char *font_filename,  //!< in: path of the font file 
  int         min_char )      //!< in: first representable character in the font
  : _font_filename( font_filename )
  , _min_char( min_char )
{}


/******************************************************************//**
* @brief   dtor.
*
* @author  gernot
* @date    2018-03-18
* @version 1.0
**********************************************************************/
CFreetypeTexturedFont::~CFreetypeTexturedFont()
{}


/******************************************************************//**
* @brief   Destroy all internal objects and cleanup.
*
* @author  gernot
* @date    2018-03-18
* @version 1.0
**********************************************************************/
void CFreetypeTexturedFont::Destroy( void )
{
  _font.reset( nullptr );
  _font_texture.reset( nullptr );

  // ...
}


/******************************************************************//**
* @brief   Load the glyphs.
*
* @author  gernot
* @date    2018-03-18
* @version 1.0
**********************************************************************/
bool CFreetypeTexturedFont::Load( 
  Render::ITextureLoader &loader ) //!< in: texture loader implementation
{
  if ( _font != nullptr )
    return _valid;

  static bool create_stroke = false;

  _font = std::make_unique<TFreetypeTFont>();
  TFreetypeTFont &data = *_font.get();


  // initialize FreeType library
  FT_Error err_code = FT_Init_FreeType( &_font->_hdl );
  if ( err_code != 0 )
  {          
    std::cout << "error: failed to initialize FreeType library (error code: " << err_code << ")" << std::endl;
    throw std::runtime_error( "initialize FreeType library" );
  }

  // load the font from file
  err_code = FT_New_Face( _font->_hdl, _font_filename.c_str(), 0, &_font->_face );
  if ( err_code != 0 )
  {          
    std::cout << "error: failed to load font file " << _font_filename << " (error code: " << err_code << ")" << std::endl;
    throw std::runtime_error( "load font file" );
  }
  FT_Face face = _font->_face;

  // create a stroker
  if ( create_stroke )
  {
    err_code = FT_Stroker_New( _font->_hdl, &_font->_stroker );
    if ( err_code != 0 )
    {
        std::cerr << "Failed to create the stroker (error code: " << err_code << ")" << std::endl;
        throw std::runtime_error( "initialize FreeType library - stroker" );
    }
  }
  FT_Stroker stroker = _font->_stroker;

  // set font size
  static FT_UInt pixel_width  = 0;
  static FT_UInt pixel_height = 48;
  err_code = FT_Set_Pixel_Sizes( face, pixel_width, pixel_height );
  if ( err_code != 0 )
  {          
    std::cout << "error: failed to set font size (error code: " << err_code << ")" << std::endl;
    throw std::runtime_error( "load font file" );
  }

  FT_GlyphSlot glyph = face->glyph;

  // evaluate texture size  and metrics
  // FreeType Glyph Conventions [https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html]

  data._width    = 1;  
  data._min_char = _min_char;
  data._max_char = 256;
  data._glyphs  = std::vector<TFreetypeGlyph>( data._max_char - data._min_char );
  for ( int i = data._min_char; i < data._max_char ; ++ i )
  {
    FT_Error err_code_glyph;
    if ( create_stroke )
      err_code_glyph = FT_Load_Char( face, i, FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_NORMAL );
    else
      err_code_glyph = FT_Load_Char( face, i, FT_LOAD_RENDER ); // used before 
    if ( err_code_glyph != 0 )
      continue;

    unsigned int cx = glyph->bitmap.width;
    unsigned int cy = glyph->bitmap.rows;

    TFreetypeGlyph &glyph_data = data._glyphs[i-data._min_char];
    glyph_data._metrics = glyph->metrics;
    glyph_data._x       = data._width;
    glyph_data._y       = 1;
    glyph_data._cx      = cx;
    glyph_data._cy      = cy;

    FT_Glyph glyphDescStroke;
    err_code = FT_Get_Glyph( glyph, &glyphDescStroke );
    if ( err_code != 0 )
      continue;

    if ( create_stroke )
    {
      static double outlineThickness = 2.0;
      FT_Stroker_Set( stroker, static_cast<FT_Fixed>(outlineThickness * static_cast<float>(1 << 6)), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );
      err_code = FT_Glyph_Stroke( &glyphDescStroke, stroker, true );
      if ( err_code != 0 )
        continue;

      err_code = FT_Glyph_To_Bitmap( &glyphDescStroke, FT_RENDER_MODE_NORMAL, 0, 1);
      if ( err_code != 0 )
        continue;
    }

    //FT_Bitmap     *bitmap = &glyph->bitmap;
    FT_BitmapGlyph glyph_bitmap = (FT_BitmapGlyph)glyphDescStroke;
    FT_Bitmap     *bitmap = create_stroke ? &glyph_bitmap->bitmap : &glyph->bitmap;
                                                                   
    cx = bitmap->width;
    cy = bitmap->rows;
    glyph_data._cx = cx;
    glyph_data._cy = cy;

    // TODO $$$ use ox and oy instead of `glyph->metrics`?
    unsigned int ox = glyph_bitmap->left;
    unsigned int oy = glyph_bitmap->top;
    unsigned int ax = face->glyph->advance.x;
      
    if ( bitmap->buffer == nullptr )
      continue;

    glyph_data._image = std::vector<unsigned char>( cx * cy * 4, 0 );
    for ( unsigned int i = 0; i < cx * cy; ++ i)
    {
      unsigned char b = bitmap->buffer[i];
      unsigned char b_2 = (unsigned char)( b / 2 );
      glyph_data._image[i*4 + 0] = create_stroke ? b_2 : b;
      glyph_data._image[i*4 + 1] = create_stroke ? b_2 : b;
      glyph_data._image[i*4 + 2] = create_stroke ? b_2 : b;
      glyph_data._image[i*4 + 3] = b;
    }

    FT_Done_Glyph( glyphDescStroke );

    if ( create_stroke )
    {
      FT_Glyph glyphDescFill;
      err_code = FT_Get_Glyph( glyph, &glyphDescFill );
      if ( err_code == 0 )
        err_code = FT_Glyph_To_Bitmap( &glyphDescFill, FT_RENDER_MODE_NORMAL, 0, 1);

      FT_Bitmap *bitmap = nullptr;
      if ( err_code == 0 )
      {
        FT_BitmapGlyph glyph_bitmap = (FT_BitmapGlyph)glyphDescFill;
        bitmap = &glyph_bitmap->bitmap;
      }

      if ( err_code == 0 && bitmap )
      {
        unsigned int cx_fill = bitmap->width;
        unsigned int cy_fill = bitmap->rows;
        unsigned int offset_x = (cx - cx_fill) / 2;
        unsigned int offset_y = (cy - cy_fill) / 2;

        for ( unsigned int y = 0; y < cy_fill; ++ y )
        {
          for ( unsigned int x = 0; x < cx_fill; ++ x )
          {
            unsigned int i_source = y * cx_fill + x;
            unsigned int i_target = (y + offset_y) * cx + x + offset_x;
            unsigned char b = bitmap->buffer[i_source];
            unsigned char b_2 = (unsigned char)( b / 2 );
            
            glyph_data._image[i_target*4]   = std::max( glyph_data._image[i_target*4], b);
            glyph_data._image[i_target*4+1] = std::max( glyph_data._image[i_target*4+1], b);
            glyph_data._image[i_target*4+2] = std::max( glyph_data._image[i_target*4+2], b);
            glyph_data._image[i_target*4+3] = std::max( glyph_data._image[i_target*4+3], b);         
          }
        }
      }

      FT_Done_Glyph( glyphDescFill );
    }
    
    data._width      += cx+1;
    data._max_height  = std::max( data._max_height, cy );

    data._max_glyph_cy = std::max( data._max_glyph_cy, (int)glyph->metrics.height );
    data._max_glyph_y  = std::max( data._max_glyph_y,  (int)glyph->metrics.horiBearingY );
  }
  data._max_height += 2;

  // create texture

  Render::TTextureSize size{ data._width, data._max_height, 1 };
  Render::TStandardTextureKind std_kind = Render::T2D_RGBA_clamped_bilinear;
  //Render::TStandardTextureKind std_kind = Render::T2D_RGBA_clamped_trilinear; // may cause disappearing of text at huge minification
  _font_texture = loader.CreateTexture(size, 0, std_kind); 


  // load glyphs and copy glyphs to texture


  // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
  // TODO $$$ load fonts in different resolutions to mipmap layers
  // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

  for ( int i = data._min_char; i < data._max_char; ++ i )
  {
    TFreetypeGlyph &glyph_data = data._glyphs[i-data._min_char];
    if ( glyph_data._cx == 0 || glyph_data._cy == 0 )
      continue;

    TGlyphImage image( glyph_data );
    Render::TTexturePoint pos{ glyph_data._x, glyph_data._y, 0 };
    loader.LoadToTexture( image, *_font_texture, pos, 0 );
  }

  _font_texture->Release( 0 );

  _valid = true;
  return _valid;
}


/******************************************************************//**
* \brief   Calculates box of a string, in relation to its height
* (maximum height of the font from the bottom to the top)  
* 
* \author  gernot
* \date    2018-03-18
* \version 1.0
**********************************************************************/
bool CFreetypeTexturedFont::CalculateTextSize( 
  const char *str,      //!< in: the text
  float       height,   //!< in: the maximum height of the text from the bottom to the top  
  float      &box_x,    //!< out: with of the text
  float      &box_btm,  //!< out: height from the base line to the top 
  float      &box_top ) //!< out: height form the base line to the bottom (usually negative)
{
  if ( _font == nullptr )
    return false;

  int min_c = _font->_min_char;
  int max_c = _font->_max_char;

  std::string std_str( str );
  FT_Pos metrics_width  = 0;
  FT_Pos metrics_height = 0;
  FT_Pos metrics_top    = 0;
  for ( char c : std_str )
  {
    // get the glyph data
    int i_c = *(unsigned char*)(&c);
    if ( i_c < min_c || i_c > max_c )
      continue;
    TFreetypeGlyph &glyph = _font->_glyphs[i_c-min_c];
    
    metrics_width += glyph._metrics.horiAdvance;
    metrics_height = std::max( metrics_height, glyph._metrics.height );
    metrics_top    = std::max( metrics_top, glyph._metrics.horiBearingY );
  }

  float scale = height / (float)_font->_max_glyph_cy;

  box_x   = scale * metrics_width;
  box_top = scale * metrics_top;
  box_btm = scale * (metrics_top - metrics_height);

  return true;
}


/******************************************************************//**
* \brief   render a text 
* 
* \author  gernot
* \date    2018-03-18
* \version 1.0
**********************************************************************/
bool CFreetypeTexturedFont::Draw( 
  Render::IDrawBufferProvider &buffer_provider,   //!< in: draw library
  size_t                       textur_binding_id, //!< in: texture unit index
  const char                  *str,               //!< in: the text
  float                        height,            //!< in: the maximum height of the text from the bottom to the top 
  float                        width_scale,       //!< in: scale of the text in the y direction
  const Render::TPoint3       &pos )              //!< in: the reference position
{
  static bool debug_test = false;
  if ( debug_test )
    DebugFontTexture( buffer_provider, textur_binding_id );

  if ( _font == nullptr  || _font_texture == nullptr )
    return false;

  int min_c = _font->_min_char;
  int max_c = _font->_max_char;

  float scale_y = height / (float)_font->_max_glyph_cy;
  float scale_x = scale_y * width_scale;

  // set up vertex coordinate attribute array

  std::string std_str( str );
  FT_Pos metrics_width  = 0;
  std::vector<float> vertex_attributes; // x y z u v
  vertex_attributes.reserve( 5 * 6 * std_str.length() );
  for ( char c : std_str )
  {
    // get the glyph data
    int i_c = *(unsigned char*)(&c);
    if ( i_c < min_c || i_c > max_c )
      continue;
    TFreetypeGlyph &glyph = _font->_glyphs[i_c-min_c];
    
    // calculate font metrics vertex coordinate box
    FT_Pos metrics_coord[]{
      metrics_width + glyph._metrics.horiBearingX,                        // min x
      glyph._metrics.horiBearingY - glyph._metrics.height,                // min y
      metrics_width + glyph._metrics.horiBearingX + glyph._metrics.width, // max x
      glyph._metrics.horiBearingY                                         // max y
    };

    // increment width to the start of the next glyph
    metrics_width += glyph._metrics.horiAdvance;

    if ( glyph._metrics.width == 0 || glyph._metrics.height == 0 )
      continue;

    // calculate vertex coordinate box
    float glyph_coords[]{
      pos[0] + scale_x * (float)metrics_coord[0], pos[1] + scale_y * (float)metrics_coord[1],
      pos[0] + scale_x * (float)metrics_coord[2], pos[1] + scale_y * (float)metrics_coord[3]
    };

    // calculate texture coordinates box
    float glyph_tex_coords[]{
      (float)glyph._x / (float)_font->_width,
      (float)(glyph._y + glyph._cy) / (float)_font->_max_height,
      (float)(glyph._x + glyph._cx) / (float)_font->_width,
      (float)glyph._y / (float)_font->_max_height,
    };

    // set up vertex attribute array
    std::array<std::array<float, 5>, 4> quad{
      std::array<float, 5>{ glyph_coords[0], glyph_coords[1], pos[2], glyph_tex_coords[0], glyph_tex_coords[1] },
      std::array<float, 5>{ glyph_coords[2], glyph_coords[1], pos[2], glyph_tex_coords[2], glyph_tex_coords[1] },
      std::array<float, 5>{ glyph_coords[2], glyph_coords[3], pos[2], glyph_tex_coords[2], glyph_tex_coords[3] },
      std::array<float, 5>{ glyph_coords[0], glyph_coords[3], pos[2], glyph_tex_coords[0], glyph_tex_coords[3] }
    };
    std::array<int, 6> indices{ 0, 1, 2, 0, 2, 3 };
    for ( auto i : indices )
      vertex_attributes.insert( vertex_attributes.end(), quad[i].begin(), quad[i].end() );
  }

  // buffer specification
  Render::TVA va_id = Render::TVA::b0_xyz_uv;
  const std::vector<char> bufferdescr = Render::IDrawBuffer::VADescription( va_id );

  // create buffer
  Render::IDrawBuffer &buffer = buffer_provider.DrawBuffer();
  buffer.SpecifyVA( bufferdescr.size(), bufferdescr.data() );
  buffer.UpdateVB( 0, sizeof(float), vertex_attributes.size(), vertex_attributes.data() );
  
  // bind glyph texture 
  _font_texture->Bind( textur_binding_id );


  /*
  TODO $$$

  font shader

  - SSBO font metrics
  - SSBO (dynamic array) glyph indices
  - atomic font position 
  - instance render quad (number of letter) 

  */

  // draw_buffer
  size_t no_of_vertices = vertex_attributes.size() / 5; // 5 because of x y z u v
  buffer.DrawArray( Render::TPrimitive::triangles, 0, no_of_vertices, true ); // TODO Render::TPrimitive::trianglestrip + indices / primitive restart
  buffer.Release();

  // unbind glyph texture
  _font_texture->Release( textur_binding_id );

  return true;
}


/******************************************************************//**
* \brief   Draw the entire font texture for debug reasons.
* 
* \author  gernot
* \date    2018-03-18
* \version 1.0
**********************************************************************/
void CFreetypeTexturedFont::DebugFontTexture( 
  Render::IDrawBufferProvider &buffer_provider,     //!< in: draw library
  size_t                       textur_binding_id ) //!< in: texture unit index
{
  if ( _font == nullptr || _font_texture == nullptr )
    return;

  int min_c = _font->_min_char;
  int max_c = _font->_max_char;

  float t_0 = 0.0;
  float t_1 = 0.4f;
  float h = 10.0f * (float)_font->_max_height / (float)_font->_width;

  // setup vertex attributes (x y z u v)
  std::vector<float> vertex_attributes{
    -1.0f,  -1.0f, 0.0f, t_0, 1.0f,
     1.0f,  -1.0f, 0.0f, t_1, 1.0f,
     1.0f, h-1.0f, 0.0f, t_1, 0.0f,
    
    -1.0f,  -1.0f, 0.0f, t_0, 1.0f,
     1.0f, h-1.0f, 0.0f, t_1, 0.0f,
    -1.0f, h-1.0f, 0.0f, t_0, 0.0f,
  };

  // buffer specification
  Render::TVA va_id = Render::TVA::b0_xyz_uv;
  const std::vector<char> bufferdescr = Render::IDrawBuffer::VADescription( va_id );

  // create buffer
  Render::IDrawBuffer &buffer = buffer_provider.DrawBuffer();
  buffer.SpecifyVA( bufferdescr.size(), bufferdescr.data() );
  buffer.UpdateVB( 0, sizeof(float), vertex_attributes.size(), vertex_attributes.data() );
  
  // bind glyph texture 
  _font_texture->Bind( textur_binding_id );

  // draw_buffer
  size_t no_of_vertices = vertex_attributes.size() / 5; // 5 because of x y z u v
  buffer.DrawArray( Render::TPrimitive::triangles, 0, no_of_vertices, true ); // TODO Render::TPrimitive::trianglestrip + indices / primitive restart
  buffer.Release();

  // unbind glyph texture
  _font_texture->Release( textur_binding_id );
}


} // Render
