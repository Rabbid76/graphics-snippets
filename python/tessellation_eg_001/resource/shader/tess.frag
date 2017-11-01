#version 400

in TTessData
{
    vec3  pos;
    vec3  nv;
    float height;
} inData;

out vec4 fragColor;

uniform sampler2D u_texture;

uniform UB_material
{
    float u_roughness;
    float u_fresnel0;
    vec4  u_color;
    vec4  u_specularTint;
};

struct TLightSource
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 dir;
};

uniform UB_lightSource
{
    TLightSource u_lightSource;
};

float Fresnel_Schlick( in float theta );
vec3 LightModel( in vec3 esPt, in vec3 esPtNV, in vec3 col, in vec4 specularTint, in float roughness, in float fresnel0 );

void main()
{
    vec3 col = mix( u_color.rgb, vec3( 1.0, 1.0, 1.0 ), inData.height );
    vec3 lightCol = LightModel( inData.pos, inData.nv, col, u_specularTint, u_roughness, u_fresnel0 );
    fragColor = vec4( clamp( lightCol, 0.0, 1.0 ), 1.0 );
}

float Fresnel_Schlick( in float theta )
{
    float m = clamp( 1.0 - theta, 0.0, 1.0 );
    float m2 = m * m;
    return m2 * m2 * m; // pow( m, 5.0 )
}

vec3 LightModel( in vec3 esPt, in vec3 esPtNV, in vec3 col, in vec4 specularTint, in float roughness, in float fresnel0 )
{
  vec3  esVLight      = normalize( -u_lightSource.dir.xyz );
  vec3  esVEye        = normalize( -esPt );
  vec3  halfVector    = normalize( esVEye + esVLight );
  float HdotL         = dot( halfVector, esVLight );
  float NdotL         = dot( esPtNV, esVLight );
  float NdotV         = dot( esPtNV, esVEye );
  float NdotH         = dot( esPtNV, halfVector );
  float NdotH2        = NdotH * NdotH;
  float NdotL_clamped = max( NdotL, 0.0 );
  float NdotV_clamped = max( NdotV, 0.0 );
  float m2            = roughness * roughness;
  
  // Lambertian diffuse
  float k_diffuse = NdotL_clamped;
  // Schlick approximation
  float fresnel = fresnel0 + ( 1.0 - fresnel0 ) * Fresnel_Schlick( HdotL );
  // Beckmann distribution
  float distribution = max( 0.0, exp( ( NdotH2 - 1.0 ) / ( m2 * NdotH2 ) ) / ( 3.14159265 * m2 * NdotH2 * NdotH2 ) );
  // Torrance-Sparrow geometric term
  float geometric_att = min( 1.0, min( 2.0 * NdotH * NdotV_clamped / HdotL, 2.0 * NdotH * NdotL_clamped / HdotL ) );
  // Microfacet bidirectional reflectance distribution function 
  float k_specular = fresnel * distribution * geometric_att / ( 4.0 * NdotL_clamped * NdotV_clamped );
  
  vec3 lightColor = col.rgb * u_lightSource.ambient.rgb +
                    max( 0.0, k_diffuse ) * col.rgb * u_lightSource.diffuse.rgb +
                    max( 0.0, k_specular ) * mix( col.rgb, specularTint.rgb, specularTint.a ) * u_lightSource.specular.rgb;
  return lightColor;
}