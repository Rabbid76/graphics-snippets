/******************************************************************//**
* \brief   Implementation of generic interface for basic drawing.
* 
* \author  gernot
* \date    2018-02-04
* \version 1.0
**********************************************************************/

// includes

#include <stdafx.h>


// OpenGL

#include <OpenGLError.h>
#include <OpenGLBasicDraw.h>
#include <OpenGLVertexBuffer.h>
#include <OpenGLFrameBuffer.h>
#include <OpenGLTextureLoader.h>


// OpenGL wrapper

#include <OpenGL_include.h>


// utility 

#include <RenderUtil_FreetypeFont.h>


// STL

#include <cassert>
#include <algorithm>
#include <cstddef>


// class implementation

/******************************************************************//**
* \brief General OpenGL namespace
**********************************************************************/
namespace OpenGL
{

std::set<std::string> CBasicDraw::_ogl_extensins;
int                   CBasicDraw::_max_anistropic_texture_filter = 0;


/******************************************************************//**
* \class OpenGL::CBasicDraw  
*
* Opaque and transparent shader:
* 
* If a rendered primitive hasn't a texture, a "white" 1*1 texture is
* set, this causes that the shader has not to be changed when switching
* between color and texture.
* Since the texture color and the color are modulated (multiplied), a
* texture can be tint by a color.
*
**********************************************************************/


//---------------------------------------------------------------------
// opaque and transparent polygon shader
//---------------------------------------------------------------------

std::string opaque_transp_sh_vert = R"(
#version 460

layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec2 in_tex;

out TVertexData
{
    vec3 pos;
    vec2 tex;
} out_data;


layout(std430, binding = 1) buffer TUniform
{
    mat4 u_proj;
    mat4 u_view;
    mat4 u_model;
    vec2 u_vp_size;
};

void main()
{
    vec4 view_pos = u_view * u_model * in_pos; 
    out_data.pos  = view_pos.xyz / view_pos.w;
    out_data.tex  = in_tex;
    gl_Position   = u_proj * view_pos;
}
)";

std::string line_sh_geom = R"(
#version 460

layout( invocations = 1 ) in;
layout( lines_adjacency ) in;
layout( triangle_strip, max_vertices = 4 ) out;

in TVertexData
{
    vec3 pos;
    vec2 tex;
} in_data[];

out TVertexData
{
    vec3 pos;
    vec2 tex;
} out_data;

layout(std430, binding = 1) buffer TUniform
{
    mat4 u_proj;
    mat4 u_view;
    mat4 u_model;
    vec2 u_vp_size;
};

uniform float u_thickness; 
uniform int   u_perspective_thickness;

void main()
{
    vec3 pos0     = in_data[1].pos;
    vec3 pos1     = in_data[2].pos;
    vec3 dirLine  = pos1 - pos0; 
    vec3 dirPred  = pos0 - in_data[0].pos;
    vec3 dirSucc  = in_data[3].pos - pos1;
    vec3 dirNorm  = normalize(vec3(-dirLine.y, dirLine.x, 0.0)); 
    vec3 dirPredN = length(dirPred.xy) < 0.0001 ? dirNorm : normalize(vec3(-dirPred.y, dirPred.x, 0.0));
    vec3 dirSuccN = length(dirSucc.xy) < 0.0001 ? dirNorm : normalize(vec3(-dirSucc.y, dirSucc.x, 0.0));
    vec3 dir0     = abs( dot(normalize(dirPred.xy), normalize(dirLine.xy)) ) > 0.99 ? dirNorm : normalize( dirNorm + dirPredN );
    vec3 dir1     = abs( dot(normalize(dirSucc.xy), normalize(dirLine.xy)) ) > 0.99 ? dirNorm : normalize( dirNorm + dirSuccN );
   
    // normalized quad positions
    vec3 normal_pos[4];
    normal_pos[0] = pos0 - dirNorm;
    normal_pos[1] = pos1 - dirNorm;
    normal_pos[2] = pos0 + dirNorm;
    normal_pos[3] = pos1 + dirNorm;

    // projected normalized quad positions
    vec4 prj_pos[4];
    for(int i=0; i<4; ++i)
        prj_pos[i] = u_proj * vec4(normal_pos[i], 1.0);

    // NDC normalized quad positions
    vec3 ndc_pos[4];
    for(int i=0; i<4; ++i)
        ndc_pos[i] = prj_pos[i].xyz / prj_pos[i].w;

    // window normalized quad positions
    vec2 wnd_pos[4];
    for(int i=0; i<4; ++i)
        wnd_pos[i] = (ndc_pos[i].xy*0.5 + 0.5) * u_vp_size;

    float thickness_scale0 = 1.0 / dot(dir0, dirNorm);
    float thickness_scale1 = 1.0 / dot(dir1, dirNorm);
    if ( u_perspective_thickness == 0 )
    {
        thickness_scale0 *= 1.0 / length(wnd_pos[2].xy - wnd_pos[0].xy);
        thickness_scale1 *= 1.0 / length(wnd_pos[3].xy - wnd_pos[1].xy);
    }
    else
    {
        thickness_scale0 *= 1.0 / min(u_vp_size.x, u_vp_size.y);
        thickness_scale1 *= 1.0 / min(u_vp_size.x, u_vp_size.y);
    }

    // miter positions
    vec3 pos[4];
    pos[0] = pos0 - dir0 * u_thickness * thickness_scale0;
    pos[1] = pos1 - dir1 * u_thickness * thickness_scale1;
    pos[2] = pos0 + dir0 * u_thickness * thickness_scale0;
    pos[3] = pos1 + dir1 * u_thickness * thickness_scale1;

    // create the vertices and the primitive

    out_data.tex = in_data[1].tex;
    
    out_data.pos = pos[0];
    gl_Position  = u_proj * vec4(pos[0], 1.0);
    EmitVertex();
    
    out_data.pos = pos[1];
    gl_Position  = u_proj * vec4(pos[1], 1.0);
    EmitVertex();

    out_data.tex = in_data[2].tex;
    
    out_data.pos = pos[2];
    gl_Position = u_proj * vec4(pos[2], 1.0);
    EmitVertex();
    
    out_data.pos = pos[3];
    gl_Position  = u_proj * vec4(pos[3], 1.0);
    EmitVertex();

    EndPrimitive();
}
)";

std::string opaque_sh_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec2 tex;
} in_data;

out vec4 frag_color;

uniform vec4 u_color;

layout (binding = 0) uniform sampler2D u_sampler_texture;

void main()
{
    vec4 col_texture  = texture(u_sampler_texture, in_data.tex.st); 
    vec4 col_modulate = u_color * col_texture;
    frag_color        = col_modulate;
}
)";


std::string transp_sh_frag = R"(
#version 460

in TVertexData
{
    vec3 pos;
    vec2 tex;
} in_data;

layout (location = 0) out vec4 transp_color;
layout (location = 1) out vec4 transp_attrib;

uniform vec4 u_color;

layout (binding = 0) uniform sampler2D u_sampler_texture;

void main()
{                      
    vec4 col_texture  = texture(u_sampler_texture, in_data.tex.st); 
    vec4 col_modulate = u_color * col_texture;

    float weight      = col_modulate.a * (1.0 - gl_FragCoord.z);
    transp_color      = vec4(col_modulate.rgb * weight, weight);
    transp_attrib     = vec4(1.0, 0.0, 0.0, col_modulate.a);
}
)";


//---------------------------------------------------------------------
// mix color shader
//---------------------------------------------------------------------


std::string mixcol_sh_vert = R"(
#version 460

layout (location = 0) in vec2 in_pos;

out TVertexData
{
    vec2 pos;
} out_data;

void main()
{
    out_data.pos = in_pos;
    gl_Position  = vec4(in_pos, 0.0, 1.0);
}
)";

std::string mixcol_sh_frag = R"(
#version 460

in TVertexData
{
    vec2 pos;
} in_data;

out vec4 frag_color;

layout (binding = 1) uniform sampler2D u_sampler_color;
layout (binding = 2) uniform sampler2D u_sampler_transp;
layout (binding = 3) uniform sampler2D u_sampler_transp_attr;

void main()
{
    ivec2 itex_xy = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));    
    vec4  col     = texelFetch(u_sampler_color,       itex_xy, 0);
    vec4  transp  = texelFetch(u_sampler_transp,      itex_xy, 0);
    vec4  t_attr  = texelFetch(u_sampler_transp_attr, itex_xy, 0);

    vec4 col_transp = vec4(0.0);
    if ( t_attr.x > 0.0 && t_attr.a > 0.0 )
    {
        vec3 colApprox      = transp.rgb / transp.a;
        float averageTransp = t_attr.w / t_attr.x;
        float alpha         = 1.0 - pow(1.0 - averageTransp, t_attr.x);
        col_transp          = vec4(colApprox, alpha); 
    }
    else
      col.rgb /= (col.a > 1.0/255.0 ? col.a : 1.0); // resolve premultiplied alpha
    
    vec4 mix_col = mix(col.rgba, col_transp.rgba, col_transp.a);   
    frag_color   = vec4(mix_col.rgb*mix_col.a, mix_col.a);
}
)";


std::string mixcol_multisamples_sh_frag = R"(
#version 460

in TVertexData
{
    vec2 pos;
} in_data;

out vec4 frag_color;

layout (binding = 1) uniform sampler2DMS u_sampler_color;
layout (binding = 2) uniform sampler2DMS u_sampler_transp;
layout (binding = 3) uniform sampler2DMS u_sampler_transp_attr;

uniform int u_no_of_samples;

void main()
{
    ivec2 itex_xy = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)); 

    vec4 col    = vec4(0.0);
    vec4 transp = vec4(0.0);
    vec4 t_attr = vec4(0.0);
   
    int no_of_samples = u_no_of_samples;
    for (int i = 0; i < no_of_samples; ++ i)
    { 
        col    += texelFetch(u_sampler_color,       itex_xy, i);
        transp += texelFetch(u_sampler_transp,      itex_xy, i);
        t_attr += texelFetch(u_sampler_transp_attr, itex_xy, i);
    }
    col    /= float(no_of_samples);
    transp /= float(no_of_samples);
    t_attr /= float(no_of_samples);

    vec4 col_transp = vec4(0.0);
    if ( t_attr.x > 0.0 && t_attr.a > 0.0 )
    {
        vec3 colApprox      = transp.rgb / transp.a;
        float averageTransp = t_attr.w / t_attr.x;
        float alpha         = 1.0 - pow(1.0 - averageTransp, t_attr.x);
        col_transp          = vec4(colApprox, alpha); 
    }
    else
      col.rgb /= (col.a > 1.0/255.0 ? col.a : 1.0); // resolve premultiplied alpha
    
    vec4 mix_col = mix(col.rgba, col_transp.rgba, col_transp.a);   
    frag_color   = vec4(mix_col.rgb*mix_col.a, mix_col.a);
}
)";


//---------------------------------------------------------------------
// final shader
//---------------------------------------------------------------------


std::string finish_sh_vert = R"(
#version 460

layout (location = 0) in vec2 in_pos;

out TVertexData
{
    vec2 pos;
} out_data;

void main()
{
    out_data.pos = in_pos;
    gl_Position  = vec4(in_pos, 0.0, 1.0);
}
)";

std::string finish_pass_sh_frag = R"(
#version 460

in TVertexData
{
    vec2 pos;
} in_data;

out vec4 frag_color;

layout (binding = 1) uniform sampler2D u_sampler_color;

layout(std430, binding = 1) buffer TUniform
{
    mat4 u_proj;
    mat4 u_view;
    mat4 u_model;
    vec2 u_vp_size;
};

void main()
{
    vec4 col   = texture(u_sampler_color, gl_FragCoord.xy/u_vp_size); 
    frag_color = vec4(col.rgb, 1.0);
}
)";

std::string finish_sh_frag = R"(
#version 460

in TVertexData
{
    vec2 pos;
} in_data;

out vec4 frag_color;

layout (binding = 1) uniform sampler2D u_sampler_color;

layout(std430, binding = 1) buffer TUniform
{
    mat4 u_proj;
    mat4 u_view;
    mat4 u_model;
    vec2 u_vp_size;
};

#define FXAA_DEBUG 0       
#define FXAA_CALCULATE_LUMA 1
#define FXAA_GREEN_AS_LUMA 0

float FxaaLuma(vec4 rgba)
{ 
  #if (FXAA_CALCULATE_LUMA == 1)
    return dot(rgba.xyz, vec3(0.299, 0.587, 0.114));
  #else
  #if (FXAA_GREEN_AS_LUMA == 1)
    return rgba.y;
  #else
    return rgba.w;
  #endif
  #endif
}

vec4 FXAA(vec2 pos, sampler2D tex, vec2 fxaaQualityRcpFrame, float fxaaQualitySubpix, float fxaaQualityEdgeThreshold, float fxaaQualityEdgeThresholdMin)
{
  #define FXAA_QUALITY_PS 12
  const float FXAA_QUALITY_P[FXAA_QUALITY_PS] = float[FXAA_QUALITY_PS](1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);
  vec2 posM  = pos.xy;
  vec4 rgbyM = textureLod(tex, posM, 0.0);

  #if (FXAA_CALCULATE_LUMA == 1)
    float lumaM = FxaaLuma(rgbyM);
    float lumaE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(1, 0)));
    float lumaS = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(0, 1)));
    float lumaSE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(1, 1)));
    float lumaNW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1, -1)));
    float lumaN = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(0, -1)));
    float lumaW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1, 0)));
  #else      
  #if (FXAA_GREEN_AS_LUMA == 1)
    #define lumaM rgbyM.y
    vec4 luma4A = textureGather(tex, posM, 1);
    vec4 luma4B = textureGatherOffset(tex, posM, ivec2(-1, -1), 1);
  #else
    #define lumaM rgbyM.w
    vec4 luma4A = textureGather(tex, posM, 3);
    vec4 luma4B = textureGatherOffset(tex, posM, ivec2(-1, -1), 3);
  #endif
  #define lumaE luma4A.z
  #define lumaS luma4A.x
  #define lumaSE luma4A.y
  #define lumaNW luma4B.w
  #define lumaN luma4B.z
  #define lumaW luma4B.x
  #endif
      
  float maxSM = max(lumaS, lumaM);
  float minSM = min(lumaS, lumaM);
  float maxESM = max(lumaE, maxSM);
  float minESM = min(lumaE, minSM);
  float maxWN = max(lumaN, lumaW);
  float minWN = min(lumaN, lumaW);
  float rangeMax = max(maxWN, maxESM);
  float rangeMin = min(minWN, minESM);
  float rangeMaxScaled = rangeMax * fxaaQualityEdgeThreshold;
  float range = rangeMax - rangeMin;
  float rangeMaxClamped = max(fxaaQualityEdgeThresholdMin, rangeMaxScaled);
  bool earlyExit = range < rangeMaxClamped;

  if(earlyExit)
    return rgbyM;

  float lumaNE = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(1, -1)));
  float lumaSW = FxaaLuma(textureLodOffset(tex, posM, 0.0, ivec2(-1, 1)));
      
  float lumaNS = lumaN + lumaS;
  float lumaWE = lumaW + lumaE;
  float subpixRcpRange = 1.0/range;
  float subpixNSWE = lumaNS + lumaWE;
  float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
  float edgeVert1 = (-2.0 * lumaM) + lumaWE;
      
  float lumaNESE = lumaNE + lumaSE;
  float lumaNWNE = lumaNW + lumaNE;
  float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
  float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;
      
  float lumaNWSW = lumaNW + lumaSW;
  float lumaSWSE = lumaSW + lumaSE;
  float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
  float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
  float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
  float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
  float edgeHorz = abs(edgeHorz3) + edgeHorz4;
  float edgeVert = abs(edgeVert3) + edgeVert4;
      
  float subpixNWSWNESE = lumaNWSW + lumaNESE;
  float lengthSign = fxaaQualityRcpFrame.x;
  bool horzSpan = edgeHorz >= edgeVert;
  float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
      
  if(!horzSpan) lumaN = lumaW;
  if(!horzSpan) lumaS = lumaE;
  if(horzSpan) lengthSign = fxaaQualityRcpFrame.y;
  float subpixB = (subpixA * (1.0/12.0)) - lumaM;

  float gradientN = lumaN - lumaM;
  float gradientS = lumaS - lumaM;
  float lumaNN = lumaN + lumaM;
  float lumaSS = lumaS + lumaM;
  bool pairN = abs(gradientN) >= abs(gradientS);
  float gradient = max(abs(gradientN), abs(gradientS));
  if(pairN) lengthSign = -lengthSign;
  float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);

  vec2 posB;
  posB.x = posM.x;
  posB.y = posM.y;
  vec2 offNP;
  offNP.x = (!horzSpan) ? 0.0 : fxaaQualityRcpFrame.x;
  offNP.y = ( horzSpan) ? 0.0 : fxaaQualityRcpFrame.y;
  if(!horzSpan) posB.x += lengthSign * 0.5;
  if( horzSpan) posB.y += lengthSign * 0.5;

  vec2 posN;
  posN.x = posB.x - offNP.x * FXAA_QUALITY_P[0];
  posN.y = posB.y - offNP.y * FXAA_QUALITY_P[0];
  vec2 posP;
  posP.x = posB.x + offNP.x * FXAA_QUALITY_P[0];
  posP.y = posB.y + offNP.y * FXAA_QUALITY_P[0];
  float subpixD = ((-2.0)*subpixC) + 3.0;
  float lumaEndN = FxaaLuma(textureLod(tex, posN, 0.0));
  float subpixE = subpixC * subpixC;
  float lumaEndP = FxaaLuma(textureLod(tex, posP, 0.0));

  if(!pairN) lumaNN = lumaSS;
  float gradientScaled = gradient * 1.0/4.0;
  float lumaMM = lumaM - lumaNN * 0.5;
  float subpixF = subpixD * subpixE;
  bool lumaMLTZero = lumaMM < 0.0;

  lumaEndN -= lumaNN * 0.5;
  lumaEndP -= lumaNN * 0.5;
  bool doneN = abs(lumaEndN) >= gradientScaled;
  bool doneP = abs(lumaEndP) >= gradientScaled;
  if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P[1];
  if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P[1];
  bool doneNP = (!doneN) || (!doneP);
  if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P[1];
  if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P[1];

  #if (FXAA_DEBUG == 1)
    float it = 0.0;
  #endif
  for ( int i = 2; doneNP && i < FXAA_QUALITY_PS; ++ i )
  {
    #if (FXAA_DEBUG == 1)
      it += 1.0/float(FXAA_QUALITY_PS-1);
    #endif
    if(!doneN) lumaEndN = FxaaLuma(textureLod(tex, posN.xy, 0.0));
    if(!doneP) lumaEndP = FxaaLuma(textureLod(tex, posP.xy, 0.0));
    if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
    if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
    doneN = abs(lumaEndN) >= gradientScaled;
    doneP = abs(lumaEndP) >= gradientScaled;
    if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P[i];
    if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P[i];
    doneNP = (!doneN) || (!doneP);
    if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P[i];
    if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P[i];
  } 

  float dstN = posM.x - posN.x;
  float dstP = posP.x - posM.x;
  if(!horzSpan) dstN = posM.y - posN.y;
  if(!horzSpan) dstP = posP.y - posM.y;

  bool goodSpanN = (lumaEndN < 0.0) != lumaMLTZero;
  float spanLength = (dstP + dstN);
  bool goodSpanP = (lumaEndP < 0.0) != lumaMLTZero;
  float spanLengthRcp = 1.0/spanLength;

  bool directionN = dstN < dstP;
  float dst = min(dstN, dstP);
  bool goodSpan = directionN ? goodSpanN : goodSpanP;
  float subpixG = subpixF * subpixF;
  float pixelOffset = (dst * (-spanLengthRcp)) + 0.5;
  float subpixH = subpixG * fxaaQualitySubpix;

  float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
  float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
  if(!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
  if( horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
   
  vec3 fxaaCol = textureLod(tex, posM, 0.0).xyz;
  #if (FXAA_DEBUG == 1)
    fxaaCol.rgb = vec3(it, dot(fxaaCol.rgb, vec3(0.299, 0.587, 0.114)), 1.0-it); 
  #endif
  return vec4(fxaaCol, lumaM);
}

void main()
{
    // float fxaaQualitySubpix
    //   Only used on FXAA Quality.
    //   This used to be the FXAA_QUALITY__SUBPIX define.
    //   It is here now to allow easier tuning.
    //   Choose the amount of sub-pixel aliasing removal.
    //   This can effect sharpness.
    //     1.00 - upper limit (softer)
    //     0.75 - default amount of filtering
    //     0.50 - lower limit (sharper, less sub-pixel aliasing removal)
    //     0.25 - almost off
    //     0.00 - completely off

    float fxaaQualitySubpix = 0.75;


    // float fxaaQualityEdgeThreshold                                  
    //   Only used on FXAA Quality.
    //   This used to be the FXAA_QUALITY__EDGE_THRESHOLD define.
    //   It is here now to allow easier tuning.
    //   The minimum amount of local contrast required to apply algorithm.
    //     0.333 - too little (faster)
    //     0.250 - low quality
    //     0.166 - default
    //     0.125 - high quality 
    //     0.063 - overkill (slower)
    
    float fxaaQualityEdgeThreshold = 0.125;


    // float fxaaQualityEdgeThresholdMin
    //   Only used on FXAA Quality.
    //   This used to be the FXAA_QUALITY__EDGE_THRESHOLD_MIN define.
    //   It is here now to allow easier tuning.
    //   Trims the algorithm from processing darks.
    //     0.0833 - upper limit (default, the start of visible unfiltered edges)
    //     0.0625 - high quality (faster)
    //     0.0312 - visible limit (slower)
    //   Special notes when using FXAA_GREEN_AS_LUMA,
    //     Likely want to set this to zero.
    //     As colors that are mostly not-green
    //     will appear very dark in the green channel!
    //     Tune by looking at mostly non-green content,
    //     then start at zero and increase until aliasing is a problem.
    
    float fxaaQualityEdgeThresholdMin = 0.0312;

    vec4 fxaa_col = FXAA(
        gl_FragCoord.xy/u_vp_size, u_sampler_color, 1.0/u_vp_size,
        fxaaQualitySubpix, fxaaQualityEdgeThreshold, fxaaQualityEdgeThresholdMin ); 
    
    frag_color = vec4(fxaa_col.rgb, 1.0);
}
)";
   

//---------------------------------------------------------------------
// CSimpleLineRenderer
//---------------------------------------------------------------------

class CSimpleLineRenderer
  : public Render::Line::IRender
{
public:

  CSimpleLineRenderer(  CBasicDraw & draw_library, Render::IDrawBufferProvider & buffer_provider )
    : _draw_library( draw_library )
    , _buffer_provider( buffer_provider )
  {}

  ~CSimpleLineRenderer() {}

  virtual void Init( void ) override;                

  virtual bool StartSuccessiveLineDrawings( void ) { return false; }
  virtual bool FinishSuccessiveLineDrawings( void ) { return false; }

  virtual CSimpleLineRenderer & SetColor( const Render::TColor & color )                       { _color = color; return *this; }
  virtual CSimpleLineRenderer & SetColor( const Render::TColor8 & color )                      { _color = Render::toColor( color ); return *this; }
  virtual CSimpleLineRenderer & SetStyle( const Render::Line::TStyle & style )                 { _style = style; return *this; }
  virtual CSimpleLineRenderer & SetArrowStyle( const Render::Line::TArrowStyle & arrow_style ) { _arrow_style = arrow_style; return *this; }
  
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const float *coords )  override;
  
  virtual bool Draw( Render::TPrimitive primitive_type, unsigned int tuple_size, size_t coords_size, const double *coords ) override { assert(false); return false; }

  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const float *x_coords, const float *y_coords ) override { assert(false); return false; }
  virtual bool Draw( Render::TPrimitive primitive_type, size_t no_of_coords, const double *x_coords, const double *y_coords ) override { assert(false); return false; }

  virtual bool StartSequence( Render::TPrimitive primitive_type ) override { assert(false); return false; }
  virtual bool EndSequence( void ) override { assert(false); return false; }
  virtual bool DrawSequence( float x, float y, float z ) override { assert(false); return false; }
  virtual bool DrawSequence( double x, double y, double z ) override { assert(false); return false; }
  virtual bool DrawSequence( unsigned int tuple_size, size_t coords_size, const float *coords ) override { assert(false); return false; }
  virtual bool DrawSequence( unsigned int tuple_size, size_t coords_size, const double *coords ) override { assert(false); return false; }

private:

  CBasicDraw                  &_draw_library;
  Render::IDrawBufferProvider &_buffer_provider;

  Render::TColor            _color;       //!< color of the line
  Render::Line::TStyle      _style;       //!< line style parameter
  Render::Line::TArrowStyle _arrow_style; //!< arrow style parameter
};


/******************************************************************//**
* \brief   Draw a line, which is specified by a list of points.
* 
* \author  gernot
* \date    2018-03-15
* \version 1.0
**********************************************************************/
bool CSimpleLineRenderer::Draw( 
  Render::TPrimitive  primitive_type, //!< in: type of the primitives the allowed enumerators are `lines`, `linestrip`, `lineloop`, `lines_adjacency` and `linestrip_adjacency`
  unsigned int        tuple_size,     //!< in: tuple size of a vertex coordinates - 2: x, y; 3: x, y, z; 4: x, y, z, w 
  size_t              coords_size,   //!< in: size of coordinate buffer
  const float        *coords )        //!< in: coordinate buffer
{
  if ( primitive_type != Render::TPrimitive::lines &&
       primitive_type != Render::TPrimitive::linestrip && 
       primitive_type != Render::TPrimitive::lineloop &&
       primitive_type != Render::TPrimitive::lines_adjacency &&
       primitive_type != Render::TPrimitive::linestrip_adjacency )
  {
    assert( false );
    return false;
  }
  
  if ( tuple_size != 2 && tuple_size != 3 && tuple_size != 4 )
  {
    assert( false );
    return false;
  }

  bool arrow_from = _arrow_style._properites.test( (int)Render::Line::TArrowStyleProperty::arrow_from );
  bool arrow_to   = _arrow_style._properites.test( (int)Render::Line::TArrowStyleProperty::arrow_to );
  
  // create indices
  Render::TPrimitive final_primitive_type = primitive_type;
  size_t             no_of_vertices       = coords_size / tuple_size;
  std::vector<unsigned short int> indices;
  indices.reserve( no_of_vertices * 2 );
  
  switch(primitive_type)
  {
    case Render::TPrimitive::lines:
      {
        for ( unsigned short int i = 0; i < (unsigned short int)no_of_vertices; i += 2 )
        {
          indices.push_back( i+1 );
          indices.push_back( i );
          indices.push_back( i+1 );
          indices.push_back( i );
        }
         
        final_primitive_type = Render::TPrimitive::lines_adjacency;
      }
      break;

    case Render::TPrimitive::linestrip:
      {
        indices.push_back( 1 );
        for ( unsigned short int i = 0; i < (unsigned short int)no_of_vertices; ++ i )
          indices.push_back( i );
        indices.push_back( (unsigned short int)(no_of_vertices-2) );

        final_primitive_type = Render::TPrimitive::linestrip_adjacency;
      }
      break;

    case Render::TPrimitive::lineloop:
      {
        indices.push_back( (unsigned short int)(no_of_vertices-1) );
        for ( unsigned short int i = 0; i < (unsigned short int)no_of_vertices; ++ i )
          indices.push_back( i );
        indices.push_back( 0 );
        indices.push_back( 1 );

        final_primitive_type = Render::TPrimitive::linestrip_adjacency;
      }
      break;
  }

  // buffer specification
  Render::TVA va_id = tuple_size == 2 ? Render::TVA::b0_xy : (tuple_size == 3 ? Render::TVA::b0_xyz : Render::TVA::b0_xyzw);
  const std::vector<char> bufferdescr = Render::IDrawBuffer::VADescription( va_id );

  // set style and context
  glEnable( GL_POLYGON_OFFSET_LINE );
  glPolygonOffset( 1.0, 1.0 );
  OPENGL_CHECK_GL_ERROR

  // create buffer
  Render::IDrawBuffer &buffer = _buffer_provider.DrawBuffer();
  buffer.SpecifyVA( bufferdescr.size(), bufferdescr.data() );
  if ( arrow_from || arrow_to )
  {
    std::vector<Render::t_fp> temp_coords( coords_size );
    std::copy( coords, coords + coords_size, temp_coords.begin() );

    // cut line at arrow (thick lines would jut out at the peak of the arrow)
    if (arrow_from)
    {
      size_t i0 = 0;
      size_t ix = tuple_size;
      glm::vec3 p0( coords[i0], coords[i0+1], tuple_size == 3 ? coords[i0+2] : 0.0f );
      glm::vec3 p1( coords[ix], coords[ix+1], tuple_size == 3 ? coords[ix+2] : 0.0f );
      glm::vec3 v0 = p0 - p1;
      float l = glm::length( v0 );
      p0 = p1 + v0 * (l-_arrow_style._size[0]) / l;
      temp_coords[i0] = p0[0];
      temp_coords[i0+1] = p0[1];
      if (tuple_size == 3)
        temp_coords[i0+2] = p0[2];
    }
    if (arrow_to)
    {
      size_t i0 = coords_size - tuple_size;
      size_t ix = coords_size - tuple_size*2;
      glm::vec3 p0( coords[i0], coords[i0+1], tuple_size == 3 ? coords[i0+2] : 0.0f );
      glm::vec3 p1( coords[ix], coords[ix+1], tuple_size == 3 ? coords[ix+2] : 0.0f );
      glm::vec3 v0 = p0 - p1;
      float l = glm::length( v0 );
      p0 = p1 + v0 * (l-_arrow_style._size[0]) / l;
      temp_coords[i0] = p0[0];
      temp_coords[i0+1] = p0[1];
      if (tuple_size == 3)
        temp_coords[i0+2] = p0[2];
    }

    buffer.UpdateVB( 0, sizeof(float), temp_coords.size(), temp_coords.data() );
  }
  else
    buffer.UpdateVB( 0, sizeof(float), coords_size, coords );
  
  // set program
  _draw_library.SetLineShader( _color, _style._width );

  // draw_buffer
  buffer.DrawElements( final_primitive_type, 2, indices.size(), indices.data(), true );
  buffer.Release();

  // draw arrows
  if ( arrow_from || arrow_to )
  {
    // set program
    _draw_library.SetPolygonShader( _color );
   
    TMat44 model_bk = _draw_library._uniforms._model;

    // TODO $$$ beautify arrow
    static const std::vector<float>arrow_vertices{ 0.0f, 0.0f, -1.0f, -0.5f, -1.0f, 0.5f };
    static const std::vector<char> arrow_buffer_decr = Render::IDrawBuffer::VADescription( Render::TVA::b0_xy );
    static const size_t no_arrow_vertices = 3;

    // create arrow buffer
    Render::IDrawBuffer &buffer = _buffer_provider.DrawBuffer();
    buffer.SpecifyVA( arrow_buffer_decr.size(), arrow_buffer_decr.data() );
    buffer.UpdateVB( 0, sizeof(float), arrow_vertices.size(), arrow_vertices.data() );

    glm::mat4 model = ToGLM( _draw_library._uniforms._model );

    // arrow at the start of the line polygon
    if ( arrow_from )
    {
      // TODO $$$ 3D !!! project to viewport
      size_t i0 = 0;
      size_t ix = tuple_size;
      _draw_library.SetModelUniform(
        TVec3{ _arrow_style._size[0], _arrow_style._size[1], 1.0f },
        TVec3{ coords[i0], coords[i0+1], tuple_size == 3 ? coords[i0+2] : 0.0f },
        TVec3{ coords[ix], coords[ix+1], tuple_size == 3 ? coords[ix+2] : 0.0f },
        TVec3{ _draw_library._uniforms._view[2][0], _draw_library._uniforms._view[2][1], - _draw_library._uniforms._view[2][2] } );
      
      buffer.DrawArray( Render::TPrimitive::trianglefan, 0, no_arrow_vertices, true );
      buffer.Release();
    }
    _draw_library._uniforms._model = model_bk;

    // arrow at the end of the line polygon
    if ( arrow_to )
    {
      // TODO $$$ 3D !!! project to viewport
      size_t i0 = coords_size - tuple_size;
      size_t ix = coords_size - tuple_size*2;
      _draw_library.SetModelUniform(
        TVec3{ _arrow_style._size[0], _arrow_style._size[1], 1.0f },
        TVec3{ coords[i0], coords[i0+1], tuple_size == 3 ? coords[i0+2] : 0.0f },
        TVec3{ coords[ix], coords[ix+1], tuple_size == 3 ? coords[ix+2] : 0.0f },
        TVec3{ _draw_library._uniforms._view[2][0], _draw_library._uniforms._view[2][1], - _draw_library._uniforms._view[2][2] } );

      buffer.DrawArray( Render::TPrimitive::trianglefan, 0, no_arrow_vertices, true );
      buffer.Release();
    }
     
    _draw_library._uniforms._model = model_bk;
    _draw_library.SetModelUniform( &_draw_library._uniforms._model[0][0] );
  }

  // reset style and context
  glDisable( GL_POLYGON_OFFSET_LINE ); 
  OPENGL_CHECK_GL_ERROR

  return true;
}


//---------------------------------------------------------------------
// CBasicDraw
//---------------------------------------------------------------------


/******************************************************************//**
* @brief   ctor
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
CBasicDraw::CBasicDraw( 
  bool         core_mode, //!< - OpenGL core, forward compatibility mode
  unsigned int samples,   //!< - samples for multisampling
  float        scale,     //!< - framebuffer scale
  bool         fxaa )     //!< - true: FXAA
  : _core_mode( core_mode )
  , _samples( samples )
  , _fb_scale( scale )
  , _fxaa( fxaa )
{}


/******************************************************************//**
* @brief   dtor
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
*********************************************************************/
CBasicDraw::~CBasicDraw()
{
  Destroy();
}


/******************************************************************//**
* \brief   Create new or return existing default draw buffer
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
Render::IDrawBuffer & CBasicDraw::DrawBuffer( void )
{
  bool cached = false;
  return DrawBuffer( nullptr, cached );
}


/******************************************************************//**
* \brief   Create new or return existing default draw buffer
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
Render::IDrawBuffer & CBasicDraw::DrawBuffer( 
  const void *key,     //!< I - key for the temporary cache
  bool       &cached ) //!< O - object was found in the cache
{
  if ( key != nullptr )
  {
    // try to find temporary buffer which already has the data
    auto keyIt = std::find( _buffer_keys.begin(), _buffer_keys.end(), key );
    if ( keyIt != _buffer_keys.end() )
    {
      size_t inx = keyIt - _buffer_keys.begin();
      if ( _nextBufferI == inx )
        _nextBufferI = _nextBufferI < _max_buffers - 1 ? _nextBufferI + 1 : 0;
      
      auto foundBuffer = _draw_buffers[inx].get();
      cached = true;
      return *foundBuffer;
    }
  }

  // ensure the buffer object is allocated
  if ( _draw_buffers[_nextBufferI] == nullptr )
    _draw_buffers[_nextBufferI] = NewDrawBuffer( Render::TDrawBufferUsage::stream_draw );
  
  // get buffer object
  auto currentBuffer = _draw_buffers[_nextBufferI].get();
  _buffer_keys[_nextBufferI] = key;
  _nextBufferI = _nextBufferI < _max_buffers - 1 ? _nextBufferI + 1 : 0;

  cached = false;
  return *currentBuffer;
}


/******************************************************************//**
* \brief   Create a new and empty draw buffer object.
* 
* \author  gernot
* \date    2017-11-26
* \version 1.0
**********************************************************************/
Render::IDrawBufferPtr CBasicDraw::NewDrawBuffer( 
  Render::TDrawBufferUsage usage ) //!< I - usage of draw buffer : static_draw, dynamic_draw or stream_draw
{
  return std::make_unique<OpenGL::CDrawBuffer>( usage, 1024 ); 
}


/******************************************************************//**
* \brief Create a new and empty render process.
* 
* \author  gernot
* \date    2018-07-02
* \version 1.0
**********************************************************************/
Render::IRenderProcessPtr CBasicDraw::NewRenderProcess( void )
{
  return std::make_unique<OpenGL::CRenderProcess>();
}


/******************************************************************//**
* \brief Provides a texture loader.
* 
* \author  gernot
* \date    2018-07-02
* \version 1.0
**********************************************************************/
Render::ITextureLoaderPtr CBasicDraw::NewTextureLoader( 
  size_t loader_binding_id ) //!< I - texture unit for loading images
{
  return std::make_unique<CTextureLoader>( loader_binding_id );
}


/******************************************************************//**
* \brief  Return line renderer interface
* 
* \author  gernot
* \date    2017-11-27
* \version 1.0
**********************************************************************/
Render::Line::IRender & CBasicDraw::LineRender( void )
{
  if ( _line_render == nullptr )
    _line_render = std::make_shared<CSimpleLineRenderer>( *this, *this );
  return *_line_render.get();
}


/******************************************************************//**
* @brief   destroy internal GPU objects
*
* @author  gernot
* @date    2018-02-06
* @version 1.0
**********************************************************************/
void CBasicDraw::Destroy( void )
{
  _initialized = false;
  _drawing     = false;

  _opaque_prog.reset( nullptr );
  _transp_prog.reset( nullptr );
  _mixcol_prog.reset( nullptr );

  for ( auto & buffer : _draw_buffers )
    buffer.reset( nullptr );

  for ( auto & key : _buffer_keys )
    key = nullptr;
  _nextBufferI = 0;

  if ( _color_texture != 0 )
  {
    glDeleteTextures( 1, &_color_texture );
    OPENGL_CHECK_GL_ERROR
    _color_texture = 0;
  }

  if ( _uniform_ssbo != 0 )
  {
    glDeleteBuffers( 1, &_uniform_ssbo );
    OPENGL_CHECK_GL_ERROR
    _uniform_ssbo = 0;
  }

  _fonts.clear();
}


/******************************************************************//**
* \brief   Returns a font by its id.
* 
* The font is loaded, if not loaded yet.
*
* Most Popular Fonts [https://www.fontsquirrel.com/fonts/list/popular]
* Fonts [https://www.fontsquirrel.com/]
* 
* \author  gernot
* \date    2018-03-18
* \version 1.0
**********************************************************************/
bool CBasicDraw::LoadFont( 
  TFontId         font_id, //!< in: id of the font
  Render::IFont *&font )   //!< in: the font
{
  auto it = _fonts.find( font_id );
  if ( it != _fonts.end() )
  {
    font = it->second.get();
    return true;
  }

  std::string font_finename;
  int min_char;
  switch( font_id )
  {
    default: break;

    case font_sans:        min_char = 32; font_finename = "../resource/font/FreeSans.ttf"; break;
    case font_symbol:      min_char = 32; font_finename = "../resource/font/37043_SYMBOL.ttf"; break;
    case font_pcifico:     min_char = 32; font_finename = "../resource/font/Pacifico.ttf"; break;
    case font_alura:       min_char = 32; font_finename = "../resource/font/Allura-Regular.otf"; break;
    case font_grandhotel:  min_char = 32; font_finename = "../resource/font/GrandHotel-Regular.otf"; break;
    case font_greatevibes: min_char = 32; font_finename = "../resource/font/GreatVibes-Regular.otf"; break;
    case font_pixslim_2:   min_char = 32; font_finename = "../resource/font/pixlim_2.ttf"; break;
  }

  if ( font_finename.empty() )
  {
    assert( false );
    return false;
  }

  Render::IFont *newFont = nullptr;
  try
  {
    newFont = new Render::CFreetypeTexturedFont( font_finename.c_str(), min_char );
    auto loader = NewTextureLoader();
    newFont->Load( *loader.get() );
    _fonts[font_id].reset( newFont );
  }
  catch (...)
  {
    return false;
  }

  font = newFont;
  return newFont != nullptr;
}


/******************************************************************//**
* \brief   General initializations.
*
* Specify the render buffers
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::Init( void )
{
  if ( _initialized )
    return true;

  // get names of OpenGL extensions
  GLint no_of_extensions = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &no_of_extensions);
  for ( int i = 0; i < no_of_extensions; ++i )
  {
    std::string ext_name = reinterpret_cast<const char*>( glGetStringi( GL_EXTENSIONS, i ) );
    _ogl_extensins.insert( ext_name );
  }

  // check for anisotropic texture filter extension
  // `EXT_texture_filter_anisotropic`
  // [https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_filter_anisotropic.txt]
  if ( _ogl_extensins.find( "GL_EXT_texture_filter_anisotropic" ) != _ogl_extensins.end() )
  {
    int max_anisotropic = 0;
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropic);
    _max_anistropic_texture_filter = max_anisotropic > 0 ? max_anisotropic : 0;
  }
  CTextureLoader::SetMaxAnisotropicSamplesDefault( (size_t)_max_anistropic_texture_filter );

  // specify render process
  SpecifyRenderProcess();

  // opaque shader
  try
  {
    _opaque_prog = std::make_unique<OpenGL::ShaderProgram>(
      std::vector< TShaderInfo >{
        { opaque_transp_sh_vert, GL_VERTEX_SHADER },
        { opaque_sh_frag, GL_FRAGMENT_SHADER }
      } );
  }
  catch (...)
  {}

  // opaque line shader
  try
  {
    _opaque_line_prog = std::make_unique<OpenGL::ShaderProgram>(
      std::vector< TShaderInfo >{
        { opaque_transp_sh_vert, GL_VERTEX_SHADER },
        { line_sh_geom, GL_GEOMETRY_SHADER },
        { opaque_sh_frag, GL_FRAGMENT_SHADER }
      } );
  }
  catch (...)
  {}

  // transparent shader
  try
  {
    _transp_prog = std::make_unique<OpenGL::ShaderProgram>(
      std::vector< TShaderInfo >{
        { opaque_transp_sh_vert, GL_VERTEX_SHADER },
        { transp_sh_frag, GL_FRAGMENT_SHADER }
      } );
  }
  catch (...)
  {}

  // transparent line shader
  try
  {
    _transp_line_prog = std::make_unique<OpenGL::ShaderProgram>(
      std::vector< TShaderInfo >{
        { opaque_transp_sh_vert, GL_VERTEX_SHADER },
        { line_sh_geom, GL_GEOMETRY_SHADER },
        { transp_sh_frag, GL_FRAGMENT_SHADER }
      } );
  }
  catch (...)
  {}

  // mix color shader
  try
  {
    _mixcol_prog.reset( new OpenGL::ShaderProgram(
      {
        { mixcol_sh_vert, GL_VERTEX_SHADER },
        { Multisample() ? mixcol_multisamples_sh_frag :  mixcol_sh_frag, GL_FRAGMENT_SHADER }
      } ) );
  }
  catch (...)
  {}

  // finish shader
  try
  {
    _finish_prog.reset( new OpenGL::ShaderProgram(
      {
        { finish_sh_vert, GL_VERTEX_SHADER },
        { _fxaa ? finish_sh_frag : finish_pass_sh_frag, GL_FRAGMENT_SHADER }
      } ) );
  }
  catch (...)
  {}

  // setup color texture
  glActiveTexture( GL_TEXTURE0 );
  glGenTextures( 1, &_color_texture );
  glBindTexture( GL_TEXTURE_2D, _color_texture );      
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  unsigned char white[]{ 255, 255, 255, 255 };
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white );

  glBindTexture( GL_TEXTURE_2D, 0 );  
  OPENGL_CHECK_GL_ERROR

  // setup shader storage buffer
  glGenBuffers( 1, &_uniform_ssbo );
  glBindBuffer( GL_SHADER_STORAGE_BUFFER, _uniform_ssbo ); // OpenGL 4.3
  glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(_uniforms), &_uniforms, GL_STATIC_DRAW );
  glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, _uniform_ssbo );
  glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

  // TODO $$$ uniform block model, view, projection
  
  _initialized = true;
  return true;
}


/******************************************************************//**
* \brief   Causes update of the render process
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
void CBasicDraw::InvalidateProcess( void )
{
  if ( _process != nullptr )
    _process->Invalidate();
}


/******************************************************************//**
* \brief   Causes update of the uniforms
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
void CBasicDraw::InvalidateUniforms( void )
{
  _unifroms_valid = false;
  // TODO $$$ invalidate uniform block
}


/******************************************************************//**
* \brief   Specifies the render process
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::SpecifyRenderProcess( void )
{
  if ( _vp_size[0] == 0 || _vp_size[1] == 0 )
    return false;
  if ( _process == nullptr )
    _process = NewRenderProcess();
  if ( _process->IsValid() && _process->IsComplete() && _process->CurrentSize() == _vp_size )
    return true;

  const size_t c_depth_ID        = 0;
  const size_t c_color_ID        = 1;
  const size_t c_transp_ID       = 2;
  const size_t c_transp_attr_ID  = 3;
  const size_t c_mixed_col_ID    = 4;

  bool linear = _fxaa || _fb_scale > 1.5f;
  Render::IRenderProcess::TBufferMap buffers;
  buffers.emplace( c_depth_ID,       Render::TBuffer( Render::TBufferType::DEPTH,  Render::TBufferDataType::DEFAULT, 0, _fb_scale, _samples, false ) );
  buffers.emplace( c_color_ID,       Render::TBuffer( Render::TBufferType::COLOR4, Render::TBufferDataType::DEFAULT, 0, _fb_scale, _samples, false ) );
  buffers.emplace( c_transp_ID,      Render::TBuffer( Render::TBufferType::COLOR4, Render::TBufferDataType::F16,     0, _fb_scale, _samples, false ) );
  buffers.emplace( c_transp_attr_ID, Render::TBuffer( Render::TBufferType::COLOR4, Render::TBufferDataType::F16,     0, _fb_scale, _samples, false ) );
  buffers.emplace( c_mixed_col_ID,   Render::TBuffer( Render::TBufferType::COLOR4, Render::TBufferDataType::DEFAULT, 0, _fb_scale, 0,        linear ) );

  Render::IRenderProcess::TPassMap passes;

  Render::TPass opaque_pass( Render::TPassDepthTest::LESS, Render::TPassBlending::OFF );
  opaque_pass._targets.emplace_back( c_depth_ID, Render::TPass::TTarget::depth ); // depth target
  opaque_pass._targets.emplace_back( c_color_ID, 0 );                             // color target
  passes.emplace( c_opaque_pass, opaque_pass );

  Render::TPass transp_pass( Render::TPassDepthTest::LESS_READONLY, Render::TPassBlending::ADD );
  transp_pass._targets.emplace_back( c_depth_ID, Render::TPass::TTarget::depth, false ); // depth target
  transp_pass._targets.emplace_back( c_transp_ID, 0 );                                   // transparency target
  transp_pass._targets.emplace_back( c_transp_attr_ID, 1 );                              // transparency attribute target (adaptive transparency)
  passes.emplace( c_tranp_pass, transp_pass );

  Render::TPass background_pass( Render::TPassDepthTest::OFF, Render::TPassBlending::OFF );
  background_pass._targets.emplace_back( c_mixed_col_ID, 0, true, _bg_color ); // color target
  passes.emplace( c_back_pass, background_pass );  // TODO $$$ set default clear

  Render::TPass mixcol_pass( Render::TPassDepthTest::OFF, Render::TPassBlending::MIX_PREMULTIPLIED_ALPHA );
  mixcol_pass._sources.emplace_back( c_color_ID,       1 );      // color buffer source
  mixcol_pass._sources.emplace_back( c_transp_ID,      2 );      // transparency buffer source
  mixcol_pass._sources.emplace_back( c_transp_attr_ID, 3 );      // transparency attribute buffer source
  mixcol_pass._targets.emplace_back( c_mixed_col_ID, 0, false ); // color target
  passes.emplace( c_mixcol_pass, mixcol_pass );

  Render::TPass finish_pass( Render::TPassDepthTest::OFF, Render::TPassBlending::OFF );
  finish_pass._sources.emplace_back( c_mixed_col_ID, 1 ); // color buffer source
  passes.emplace( c_finish_pass, finish_pass );  // TODO $$$ set default clear off

  _process->SpecifyBuffers( buffers );
  _process->SpecifyPasses( passes );

  return _process->Create( _vp_size );
}


/******************************************************************//**
* \brief   Set the uniforms and update the uniform blocks.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::UpdateGeneralUniforms( void )
{
  if ( _current_prog == nullptr )
    return false;
  if (_unifroms_valid )
    return true;

  // update shader storage buffer
  glNamedBufferSubData( _uniform_ssbo, 0, sizeof( _uniforms ), &_uniforms );
  OPENGL_CHECK_GL_ERROR

  _unifroms_valid = true;
  return true;
}


/******************************************************************//**
* \brief   Set the uniforms and update the uniform blocks.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::UpdateColorUniforms( 
  const Render::TColor &color ) //!< in: new color
{
  if ( _current_prog == nullptr )
    return false;
 
  _current_prog->SetUniformF4( "u_color", color );
  return true;
}


/******************************************************************//**
* \brief   Change the model matrix uniform. 
* 
* \author  gernot
* \date    2018-03-18
* \version 1.0
**********************************************************************/
bool CBasicDraw::SetModelUniform( 
  const float *model ) //!< in: model matrix
{
  if ( _current_prog == nullptr )
    return false;

  // update shader storage buffer
  glNamedBufferSubData( _uniform_ssbo, offsetof( TUniforms, _model ), sizeof( _uniforms._model ), &_uniforms._model );
  OPENGL_CHECK_GL_ERROR

  return true;
}
    

/******************************************************************//**
* \brief   Change the model matrix uniform.   

* Set up a scaled model matrix, where `p0` the origin, `px` is on
* the (-)x-axis and a vector in the XZ-plan.
* Multiplies the current model matrix by the new model matrix.
* 
* \author  gernot
* \date    2018-03-18
* \version 1.0
**********************************************************************/
bool CBasicDraw::SetModelUniform( 
  const TVec3 &scale,    //!< scale
  const TVec3 &p0,       //!< origin
  const TVec3 &px,       //!< point on the x-axis (negative)
  const TVec3 &xz_plane) //!< the z axis
{
  glm::vec3 x_axis( p0[0]- px[0], p0[1]- px[1], p0[2]- px[2] );
  x_axis = glm::normalize( x_axis );
  glm::vec3 z_axis( xz_plane[0], xz_plane[1], xz_plane[2] );
  glm::vec3 y_axis = glm::cross( z_axis, x_axis );
  y_axis = glm::normalize( y_axis );
  z_axis = glm::cross( x_axis, y_axis );

  glm::mat4 orientation(
    x_axis[0], x_axis[1], x_axis[2], 0.0f,
    y_axis[0], y_axis[1], y_axis[2], 0.0f,
    z_axis[0], z_axis[1], z_axis[2], 0.0f,
    p0[0],     p0[1],     p0[2],     1.0f );
      
  glm::mat4 arrow_model = ToGLM( _uniforms._model ) * orientation;
  arrow_model = glm::scale( arrow_model, glm::vec3(scale[0], scale[1], scale[2]) );

  std::memcpy( &_uniforms._model[0][0], glm::value_ptr(arrow_model), 16*sizeof(float) );
  SetModelUniform( glm::value_ptr(arrow_model) );
  return true;
}


/******************************************************************//**
* \brief   Draw full screen space.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
void CBasicDraw::DrawScereenspace( void )
{
  const std::vector<char>  bufferdescr = Render::IDrawBuffer::VADescription( Render::TVA::b0_xy );
  const std::vector<float> coords{ -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

  Render::IDrawBuffer &buffer = DrawBuffer();
  buffer.SpecifyVA( bufferdescr.size(), bufferdescr.data() );
  buffer.UpdateVB( 0, sizeof(float), coords.size(), coords.data() );

  buffer.DrawArray( Render::TPrimitive::trianglestrip, 0, 4, true );
  buffer.Release();
}


/******************************************************************//**
* \brief   Enables or disables multisampling.
* 
* \author  gernot
* \date    2018-03-20
* \version 1.0
**********************************************************************/
bool CBasicDraw::EnableMultisample( 
  bool enable ) //!< in: true: enable multisampling; false: disable multisampling
{
  if ( Multisample() == false )
    return false;

  if ( enable )
    glEnable( GL_MULTISAMPLE );
  else
    glDisable( GL_MULTISAMPLE );
  OPENGL_CHECK_GL_ERROR

  return true;
}

/******************************************************************//**
* \brief   Start the rendering.
*
* Specify the render buffers
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::Begin( void )
{
  if ( _drawing || Init() == false )
  {
    assert( false );
    return false;
  }

  // specify render process
  SpecifyRenderProcess();
  _process->PrepareClear( c_opaque_pass );
  _process->PrepareClear( c_tranp_pass );
  _process->PrepareClear( c_back_pass );
  _process->Prepare( c_finish_pass );

  // disable multisampling
  EnableMultisample( false );

  // draw settings
  glEnable( GL_LINE_SMOOTH );
  OPENGL_CHECK_GL_ERROR

  _current_pass = 0;
  _drawing      = true;
  return true;
}


/******************************************************************//**
* \brief   Prepares and activates render to background.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::ActivateBackground( void )
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  // activate pass
  _current_pass = c_back_pass;
  _process->PrepareNoClear( _current_pass );

  // activate shader
  _current_prog = _opaque_prog.get();
  _current_prog->Use();
  
  // set uniforms
  _unifroms_valid = false;
  UpdateGeneralUniforms();

  // disable multisampling
  EnableMultisample( false );
  
  return true;
}


/******************************************************************//**
* \brief   Prepares and activates render to the opaque buffer.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::ActivateOpaque( void )
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  // activate pass
  _current_pass = c_opaque_pass;
  _process->PrepareNoClear( _current_pass );
  //_process->Prepare( _current_pass );

  // activate shader
  _current_prog = _opaque_prog.get();
  _current_prog->Use();
  _unifroms_valid = false;

  // set uniforms
  _unifroms_valid = false;
  UpdateGeneralUniforms();

  // enable multisampling
  EnableMultisample( true );

  return true;
}


/******************************************************************//**
* \brief   Prepares and activates render  to the transparent buffer
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::ActivateTransparent( void )
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  // activate pass
  _current_pass = c_tranp_pass;
  _process->PrepareNoClear( _current_pass );

  // activate shader
  _current_prog = _transp_prog.get();
  _current_prog->Use();
  _unifroms_valid = false;

  // set uniforms
  _unifroms_valid = false;
  UpdateGeneralUniforms();

  // enable multisampling
  EnableMultisample( true );

  return true;
}


/******************************************************************//**
* \brief   Finish the rendering.
*
* Finally write to the default frame buffer.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::Finish( void )
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  // disable multisampling
  EnableMultisample( false );

  // mix opaque and transparent color
  _process->PrepareNoClear( c_mixcol_pass );
  _mixcol_prog->Use();
  if ( Multisample() )
    _mixcol_prog->SetUniformI1( "u_no_of_samples", _samples );
  DrawScereenspace();
  glUseProgram( 0 );
  OPENGL_CHECK_GL_ERROR

  // finish pass (FXAA)
  _process->PrepareNoClear( c_finish_pass );
  _finish_prog->Use();
  DrawScereenspace();
  glUseProgram( 0 );
  OPENGL_CHECK_GL_ERROR

  _current_pass = 0;
  _drawing      = false;
  return true;
}


/******************************************************************//**
* \brief   Interim clear of the depth buffer.
* 
* \author  gernot
* \date    2018-03-16
* \version 1.0
**********************************************************************/
bool CBasicDraw::ClearDepth( void )
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  if ( _current_pass != c_opaque_pass )
    _process->PrepareNoClear( c_opaque_pass );
  
  glClear( GL_DEPTH_BUFFER_BIT );
  OPENGL_CHECK_GL_ERROR
  
  if ( _current_pass == 0 )
    _process->Release();
  else if ( _current_pass != c_opaque_pass )
    _process->PrepareNoClear( _current_pass );

  return true;
}


/******************************************************************//**
* \brief   Project by projection, view and model.
* 
* \author  gernot
* \date    2018-03-15
* \version 1.0
**********************************************************************/
TVec3 CBasicDraw::Project( 
  const TVec3 &pt //!< in: the that will be projected 
  ) const
{
  glm::vec4 prj_pt = ToGLM(_uniforms._projection) * ToGLM(_uniforms._view) * ToGLM(_uniforms._model) * glm::vec4( pt[0], pt[1], pt[2], 1.0f );
  return { prj_pt[0]/prj_pt[3], prj_pt[1]/prj_pt[3], prj_pt[2]/prj_pt[3] };
}


/******************************************************************//**
* \brief   Prepare the polygon shader for drawing
* 
* \author  gernot
* \date    2018-03-15
* \version 1.0
**********************************************************************/
bool CBasicDraw::SetPolygonShader( 
  const Render::TColor &color ) //!< I - line color
{
  // set program
  bool transparent_pass = _current_pass == c_tranp_pass;
  _current_prog = transparent_pass ? _transp_prog.get() : _opaque_prog.get();
  _current_prog->Use(); 

  // set uniforms
  UpdateGeneralUniforms();
  UpdateColorUniforms( color );

  return true;
}


/******************************************************************//**
* \brief   Prepare the line shader for drawing
* 
* \author  gernot
* \date    2018-03-15
* \version 1.0
**********************************************************************/
bool CBasicDraw::SetLineShader( 
  const Render::TColor &color,      //!< I - line color
  Render::t_fp          thickness ) //!< I - thickness of the line
{
  bool transparent_pass = _current_pass == c_tranp_pass;
  _current_prog = transparent_pass ? _transp_line_prog.get() : _opaque_line_prog.get();
  _current_prog->Use();

  // set uniforms
  UpdateGeneralUniforms();
  UpdateColorUniforms( color );
  _current_prog->SetUniformI1( "u_perspective_thickness", _draw_properties[(int)TDrawProperty::perspective_line] ? 1 : 0 );
  _current_prog->SetUniformF1( "u_thickness", thickness * _fb_scale );

  // bind "white" color texture
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, _color_texture ); 
  OPENGL_CHECK_GL_ERROR

  return true;
}

/******************************************************************//**
* \brief   Draw an array of primitives with a single color
* 
* \author  gernot
* \date    2018-03-15
* \version 1.0
**********************************************************************/
bool CBasicDraw::Draw( 
  Render::TPrimitive    primitive_type, //!< in: type of the primitives
  size_t                size,           //!< in: size vertex coordinates
  size_t                coords_size,    //!< in: size of coordinate buffer
  const Render::t_fp   *coords,         //!< in: coordinate buffer
  const Render::TColor &color,          //!< in: color for drawing
  const TStyle         &style )         //!< in: additional style parameters 
{
  // $$$ TODO $$$ Render::IDrawLine

  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  if ( size != 2 && size != 3 && size != 4 )
  {
    assert( false );
    return false;
  }

  bool is_point   = primitive_type == Render::TPrimitive::points;

  bool is_line =
    primitive_type == Render::TPrimitive::lines ||
    primitive_type == Render::TPrimitive::linestrip ||
    primitive_type == Render::TPrimitive::lineloop;

  bool is_line_adjacency =
    primitive_type == Render::TPrimitive::lines_adjacency ||
    primitive_type == Render::TPrimitive::linestrip_adjacency; 

  bool draw_line = is_line || is_line_adjacency;

  bool is_polygon =
    primitive_type == Render::TPrimitive::triangles ||
    primitive_type == Render::TPrimitive::triangle_adjacency ||
    primitive_type == Render::TPrimitive::trianglestrip ||
    primitive_type == Render::TPrimitive::trianglestrip_adjacency ||
    primitive_type == Render::TPrimitive::trianglefan;

  if ( draw_line )
  {
    Render::Line::IRender &draw_line = LineRender();
    draw_line.SetColor( color ).SetStyle( { style._thickness, 1 } ).SetArrowStyle( { style._size, style._properites } );
    return draw_line.Draw( primitive_type, (unsigned int)size, coords_size, coords );
  }

 
  // create indices
  Render::TPrimitive final_primitive_type = primitive_type;
  size_t             no_of_vertices       = coords_size / size;
  std::vector<unsigned short int> indices;
  indices.reserve( no_of_vertices * 2 );
  
  // buffer specification
  Render::TVA va_id = size == 2 ? Render::TVA::b0_xy : (size == 3 ? Render::TVA::b0_xyz : Render::TVA::b0_xyzw);
  const std::vector<char> bufferdescr = Render::IDrawBuffer::VADescription( va_id );

  // set style and context
  if ( is_point )
  {
    glEnable( GL_POLYGON_OFFSET_POINT );
    glPolygonOffset( 2.0, 1.0 );
    OPENGL_CHECK_GL_ERROR
    glPointSize( style._thickness );
    OPENGL_CHECK_GL_ERROR
  }

  // create buffer
  Render::IDrawBuffer &buffer = DrawBuffer();
  buffer.SpecifyVA( bufferdescr.size(), bufferdescr.data() );
  buffer.UpdateVB( 0, sizeof(float), coords_size, coords );
  
  // set program
  bool transparent_pass = _current_pass == c_tranp_pass;
  _current_prog = transparent_pass ? _transp_prog.get() : _opaque_prog.get();
  _current_prog->Use();

  // set uniforms
  UpdateGeneralUniforms();
  UpdateColorUniforms( color );

  // bind "white" color texture
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, _color_texture ); 
  OPENGL_CHECK_GL_ERROR

  // draw_buffer
  buffer.DrawArray( final_primitive_type, 0, no_of_vertices, true );
  buffer.Release();

  // reset style and context
  if ( is_point )
  {
    glPointSize( 1.0 );
    OPENGL_CHECK_GL_ERROR
    glDisable( GL_POLYGON_OFFSET_LINE ); 
    OPENGL_CHECK_GL_ERROR
  }

  return true;
}


/******************************************************************//**
* \brief   Render a text
* 
* \author  gernot
* \date    2018-03-18
* \version 1.0
**********************************************************************/
bool CBasicDraw::DrawText2D( 
  TFontId                font_id,     //!< in: id of the font
  const char            *text,        //!< in: the text
  float                  height,      //!< in: the height of the text
  float                  width_scale, //!< in: scale of the text in the y direction
  const Render::TPoint3 &pos,         //!< in: reference position
  const Render::TColor  &color )      //!< in: color of the text
{
  if ( _drawing == false )
  {
    assert( false );
    return false;
  }

  // get the font (load the font if not loaded yet)
  Render::IFont *font = nullptr;
  if ( LoadFont( font_id, font ) == false )
    return false;

  // set uniforms
  UpdateGeneralUniforms();
  UpdateColorUniforms( color );

  // bind "white" color texture
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, _color_texture );
  OPENGL_CHECK_GL_ERROR

  // set blending
  bool set_depth_and_belnding = _current_pass == c_opaque_pass || _current_pass == c_back_pass || _current_pass == 0;
  if ( set_depth_and_belnding )
  {
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glDepthMask( GL_TRUE );

    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA ); // premultiplied alpha
    //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); // D * (1-alpha) + S * alpha
    OPENGL_CHECK_GL_ERROR
  }

  // draw the text
  bool ret = font->DrawText( *this, 0, text, height, width_scale, pos );

  // reset blending
  if ( set_depth_and_belnding )
  {
    if ( _process != nullptr && _current_pass != 0 )
    {
      _process->PrepareMode( _current_pass );
    }
    else
    {
      glEnable( GL_DEPTH_TEST );
      glDepthFunc( GL_LESS );
      glDepthMask( GL_TRUE );
      glDisable( GL_BLEND );
      OPENGL_CHECK_GL_ERROR
    }
  }

  return ret;
}


} // OpenGL