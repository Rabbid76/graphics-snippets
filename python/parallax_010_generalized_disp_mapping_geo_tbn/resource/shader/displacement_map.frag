#version 400

//#define NORMAL_MAP_TEXTURE
#define NORMAL_MAP_QUALITY 1

in TGeometryData
{
    vec3  pos;
    vec3  nv;
    vec3  tv;
    vec3  bv;
    vec3  col;
    vec3  uvh;
    vec4  d;
    float clip;
} in_data;

out vec4 fragColor;

uniform vec3  u_lightDir;
uniform float u_ambient;
uniform float u_diffuse;
uniform float u_specular;
uniform float u_shininess;

uniform sampler2D u_texture;
uniform sampler2D u_displacement_map;
uniform float     u_displacement_scale;
uniform vec2      u_parallax_quality;

uniform vec4 u_clipPlane;
uniform mat4 u_viewMat44;
uniform mat4 u_projectionMat44;

#if defined(NORMAL_MAP_TEXTURE)
uniform sampler2D u_normal_map;
#endif

float CalculateHeight( in vec2 texCoords )
{
    float height = texture( u_displacement_map, texCoords ).x;
    return clamp( height, 0.0, 1.0 );
}

vec2 GetHeightAndCone( in vec2 texCoords )
{
    vec2 h_and_c = texture( u_displacement_map, texCoords ).rg;
    return clamp( h_and_c, 0.0, 1.0 );
}

vec4 CalculateNormal( in vec2 texCoords )
{
#if defined(NORMAL_MAP_TEXTURE)
    float height = GetHeight( texCoords );
    vec3  tempNV = texture( u_normal_map, texCoords ).xyz * 2.0 / 1.0;
    return vec4( normalize( tempNV ), height );
#else
    vec2 texOffs = 1.0 / textureSize( u_displacement_map, 0 ).xy;
    vec2 scale   = 1.0 / texOffs;
#if NORMAL_MAP_QUALITY > 1
    float hx[9];
    hx[0] = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0, -1.0) ).r;
    hx[1] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0, -1.0) ).r;
    hx[2] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0, -1.0) ).r;
    hx[3] = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0,  0.0) ).r;
    hx[4] = texture( u_displacement_map, texCoords.st ).r;
    hx[5] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0, 0.0) ).r;
    hx[6] = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0, 1.0) ).r;
    hx[7] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0, 1.0) ).r;
    hx[8] = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0, 1.0) ).r;
    vec2  deltaH = vec2(hx[0]-hx[2] + 2.0*(hx[3]-hx[5]) + hx[6]-hx[8], hx[0]-hx[6] + 2.0*(hx[1]-hx[7]) + hx[2]-hx[8]); 
    float h_mid  = hx[4];
#elif NORMAL_MAP_QUALITY > 0
    float h_mid  = texture( u_displacement_map, texCoords.st ).r;
    float h_xa   = texture( u_displacement_map, texCoords.st + texOffs * vec2(-1.0,  0.0) ).r;
    float h_xb   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 1.0,  0.0) ).r;
    float h_ya   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0, -1.0) ).r;
    float h_yb   = texture( u_displacement_map, texCoords.st + texOffs * vec2( 0.0,  1.0) ).r;
    vec2  deltaH = vec2(h_xa-h_xb, h_ya-h_yb); 
#else
    vec4  heights = textureGather( u_displacement_map, texCoords, 0 );
    vec2  deltaH  = vec2(dot(heights, vec4(1.0, -1.0, -1.0, 1.0)), dot(heights, vec4(-1.0, -1.0, 1.0, 1.0)));
    float h_mid   = heights.w; 
#endif
    return vec4( normalize( vec3( deltaH * scale, 1.0 ) ), h_mid );
#endif 
}

vec3 Parallax( in float frontFace, in vec3 texCoord, in vec3 tbnP0, in vec3 tbnP1, in vec3 tbnDir )
{   
   /*
    vec3 maxC0 = vec3( tbnDir.xy/tbnDir.z * (1.0-texCoord.p), 1.0);
    vec3 texC0 = tbnP0.z > 1.0 && tbnP1.z < 1.0 ? maxC0 : tbnP0;
    vec3 texC1 = tbnP1.z < 0.0 && tbnP0.z > 0.0 ? vec3(tbnP1.xy * (1.0-tbnP1.z/(tbnP1.z-texCoord.z)), 0.0) : tbnP1;
    texC1 = texC1.z > 1.0 && texC0.z < 1.0 ? maxC0 : texC1;
    texC0 = texC0.z < 0.0 && texC1.z > 0.0 ? vec3(tbnP0.xy * (1.0-tbnP0.z/(tbnP0.z-texCoord.z)), 0.0) : texC0;
    if ( texCoord.p < 0.0001 )
    {
        //texC1 = vec3(0.0);
        //texC0 = tbnDir/tbnDir.z;
    }
    */
    
    
    vec3 texC0 = tbnP0;
    vec3 texC1 = tbnP1;  
    texC0 += texCoord.xyz;
    texC1 += texCoord.xyz;

    bool sihouette = texCoord.p > 0.00001;

    // sample steps and quality
    vec2  quality_range  = u_parallax_quality;
    float quality        = mix( quality_range.x, quality_range.y, 1.0 - abs(normalize(tbnDir).z) );
    float numSteps       = clamp( quality * 50.0, 2.0, 50.0 );
    int   numBinarySteps = int( clamp( quality * 10.0, 1.0, 10.0 ) );
    
    numSteps = 30.0;
    float bestBumpHeight = texC0.z;
    float mapHeight = bestBumpHeight;
    float bumpHeightStep = (texC1.z-texC0.z)/numSteps;
    for ( int i = 0; i < int( numSteps ); ++ i )
    {
        mapHeight = CalculateHeight( mix(texC1.xy, texC0.xy, (bestBumpHeight-texC1.z)/(texC0.z-texC1.z)) );
        if ( mapHeight >= bestBumpHeight || bestBumpHeight > 1.0 )
        {
            if (sihouette && i==0)
                discard;
            break;
        }
        bestBumpHeight += bumpHeightStep;
    } 

    // binary steps, starting at the previous sample point 
    bestBumpHeight -= bumpHeightStep;
    for ( int i = 0; i < numBinarySteps; ++ i )
    {
        bumpHeightStep *= 0.5;
        bestBumpHeight += bumpHeightStep;
        mapHeight       = CalculateHeight( mix(texC1.xy, texC0.xy, (bestBumpHeight-texC1.z)/(texC0.z-texC1.z)) );
        bestBumpHeight -= ( bestBumpHeight < mapHeight ) ? bumpHeightStep : 0.0;
    }

    // final linear interpolation between the last to heights 
    bestBumpHeight += bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / abs(bumpHeightStep), 0.0, 1.0 );

    // set displaced texture coordiante and intersection height
    vec2 texC  = mix(texC1.xy, texC0.xy, (bestBumpHeight-texC1.z)/(texC0.z-texC1.z));
    mapHeight  = bestBumpHeight;
    
    return vec3(texC.xy, mapHeight);
}

void main()
{
    vec3  objPosEs    = in_data.pos;
    vec3  objNormalEs = in_data.nv;
    vec3  texCoords   = in_data.uvh.stp;
    float frontFace   = gl_FrontFacing ? 1.0 : -1.0; // TODO $$$ sign(dot(N,objPosEs));
    
    //vec3  tangentEs    = normalize( tangentVec - normalEs * dot(tangentVec, normalEs ) );
    //mat3  tbnMat       = mat3( tangentEs, binormalSign * cross( normalEs, tangentEs ), normalEs );

    // tangent space
    // Followup: Normal Mapping Without Precomputed Tangents [http://www.thetenthplanet.de/archives/1180]
    vec3  N           = objNormalEs;
    vec3  T           = in_data.tv;
    vec3  B           = in_data.bv;
    float invmax      = inversesqrt(max(dot(T, T), dot(B, B)));
    mat3  tbnMat      = mat3(T * invmax, B * invmax, N * invmax);
    mat3  inv_tbnMat  = inverse( tbnMat );

    // distances to the sides of the prism
    bool  sihouette        = texCoords.p > 0.0001;
    bool  silhouette_front = in_data.d.z > 0.0;
    float df = length( objPosEs );
    float d0;
    float d1;
    if ( sihouette == false )
    {
        if ( frontFace > 0.0 )
        {
            d1 = 0.0;
            d0 = min(min(in_data.d.x, in_data.d.y), in_data.d.z) - df; // TODO $$$ * 0.9
        }
        else
        {
            d0 = 0.0;
            d1 = max(max(in_data.d.x, in_data.d.y), in_data.d.z) - df;
        }
    }
    else
    {
        d0 = min(in_data.d.x, in_data.d.y) - df;
        d1 = max(in_data.d.x, in_data.d.y) - df;
    }

    // intersection points
    vec3  V  = objPosEs / df;
    vec3  P0 = V * d0;
    vec3  P1 = V * d1;
   
    //vec3  texDir3D     = normalize( inv_tbnMat * objPosEs );
    vec3  tbnP0        = inv_tbnMat * P0;
    vec3  tbnP1        = inv_tbnMat * P1;
    vec3  tbnDir       = normalize(inv_tbnMat * objPosEs);
    vec3  tbnTopMax    = tbnDir / tbnDir.z;

    if ( sihouette == false )
    {
        if ( frontFace > 0.0 )
        {
            tbnP1 = vec3(0.0);
            if ( length(tbnTopMax) < length(tbnP0) )
                tbnP0 = tbnTopMax;
        } 
        else
            discard; // TODO $$$
    }
    else 
    {
        if ( silhouette_front )
        {
            //tbnP0 = vec3(0.0);
            tbnP0 = (1.0-texCoords.p) * tbnTopMax;
            if ( d1 > -0.00001 || length(tbnTopMax)*texCoords.p < length(tbnP1) )
                tbnP1 = -texCoords.p * tbnTopMax; 
        }
        else
        {
            tbnP1 = vec3(0.0);
            if ( d0 < 0.00001 || length(tbnTopMax)*(1.0-texCoords.p) < length(tbnP0) )
                tbnP0 = (1.0-texCoords.p) * tbnTopMax;
        }
    }

    vec3  newTexCoords = abs(u_displacement_scale) < 0.001 ? vec3(texCoords.st, 0.0) : Parallax( frontFace, texCoords.stp, tbnP0, tbnP1, tbnDir );
    if ( sihouette )
    {
        
        if ( silhouette_front )
        {
            if ( newTexCoords.z > 1.000001 )
                discard;
            vec2 tex_range = tbnP1.xy; 
            float range_dist = dot(tex_range, newTexCoords.xy - texCoords.xy );
            if ( range_dist < 0.0 || range_dist > 1.0 )
                discard;
        }
        else
        {
            vec2 tex_range = tbnP0.xy; 
            float range_dist = dot(tex_range, newTexCoords.xy - texCoords.xy );
            if ( range_dist < 0.0 || range_dist > 1.0 )
                discard;
        }
    }

    vec3  displ_vec    = tbnMat * (newTexCoords.stp-texCoords.stp)/invmax;
    
    vec3  view_pos_displ = objPosEs + displ_vec;
    vec4  modelPos       = inverse(u_viewMat44) * vec4(view_pos_displ, 1.0);
    vec4  clipPlane      = vec4(normalize(u_clipPlane.xyz), u_clipPlane.w);
    float clip_dist      = dot(modelPos, clipPlane);
    //float clip_dist      = in_data.clip;
    //if ( clip_dist < 0.0 )
    //    discard;

    vec2  range_vec  = step(vec2(0.0), newTexCoords.st) * step(newTexCoords.st, vec2(1.0));
    float range_test = range_vec.x * range_vec.y;
    //if ( texCoords.p > 0.0 && (range_test == 0.0 || newTexCoords.z > 1.000001))
    //if ( texCoords.p > 0.0 && range_test == 0.0)
    //  discard;

    // discard by test against 3 clip planes (riangle prism), similar clip distance 

    texCoords.st       = newTexCoords.xy;
    
    vec4  normalVec    = CalculateNormal( texCoords.st );
    //vec3  nvMappedEs   = normalize( tbnMat * normalVec.xyz );
    vec3  nvMappedEs   = (texCoords.p > 0.0 ? 1.0 : frontFace) * normalize( transpose(inv_tbnMat) * normalVec.xyz ); // TODO $$$ evaluate `invmax`?

    //vec3 color = in_data.col;
    vec3 color = texture( u_texture, texCoords.st ).rgb;

    // ambient part
    vec3 lightCol = u_ambient * color;

    // diffuse part
    vec3  normalV = normalize( nvMappedEs );
    vec3  lightV  = normalize( -u_lightDir );
    float NdotL   = max( 0.0, dot( normalV, lightV ) );
    lightCol     += NdotL * u_diffuse * color;
    
    // specular part
    vec3  eyeV      = normalize( -objPosEs );
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( u_shininess + 2.0 ) * pow( NdotH, u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * u_specular * color;

    fragColor = vec4( lightCol.rgb, 1.0 );

    vec4 proj_pos_displ = u_projectionMat44 * vec4(view_pos_displ.xyz, 1.0);
    float depth = 0.5 + 0.5 * proj_pos_displ.z / proj_pos_displ.w;

    gl_FragDepth = depth;

//#define DEBUG_FRONT_SILHOUETTES
//#define DEBUG_BACK_SILHOUETTES
//#define DEBUG_DEPTH

#if defined(DEBUG_FRONT_SILHOUETTES)
    if ( texCoords.p < 0.0001 )
        discard;
    if ( in_data.d.z < 0.0 )
        discard;
    fragColor = vec4(vec2(in_data.d.xy-df), in_data.d.z, 1.0);
    //fragColor = vec4(vec2(d1), in_data.d.z, 1.0);
#endif

#if defined(DEBUG_BACK_SILHOUETTES)
    if ( texCoords.p < 0.0001 )
        discard;
    if ( in_data.d.z > 0.0 )
        discard;
    fragColor = vec4(vec2(df-in_data.d.xy), -in_data.d.z, 1.0);
    //fragColor = vec4(vec2(-d0), -in_data.d.z, 1.0);
#endif

#if defined(DEBUG_DEPTH)
    fragColor = vec4( vec3(1.0-depth), 1.0 );
#endif
}