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

/*
vec3 Parallax( in float frontFace, in vec3 texDir3D, in vec3 texCoord )
{   
    vec2  quality_range = u_parallax_quality;
   
    float quality         = mix( quality_range.x, quality_range.y, 1.0 - pow(abs(normalize(texDir3D).z),2.0) );
    float numSteps        = clamp( quality * 50.0, 1.0, 50.0 );
    int   numBinarySteps  = int( clamp( quality * 10.0, 1.0, 7.0 ) );
    
    vec2  texDir          = texDir3D.xy / abs(texDir3D.z); // (z is negative) the direction vector points downwards int tangent-space
    
    float mapHeight       = 1.0;
    float bestBumpHeight  = mapHeight;
    vec2  texC            = texCoord.st; 
    float base_height     = texCoord.p;
    float bumpHeightStep  = 1.0 / numSteps;
    
    float silhouette_dir  = base_height == 0.0 ? 1.0 : sign(-texDir3D.z);
    vec2  texStep         = silhouette_dir * texDir;
    if ( silhouette_dir > 0.0 )
    {
        float surf_sign = base_height > 0.01 ? 1.0 : frontFace;
        float back_face = step(0.0, -surf_sign); 
    
        texC += texStep.xy * base_height + back_face * texStep.xy;
        for ( int i = 0; i < int( numSteps ); ++ i )
        {
            mapHeight = back_face + surf_sign * CalculateHeight( texC.xy - bestBumpHeight * texStep.xy );
            if ( mapHeight >= bestBumpHeight )
                break;
            bestBumpHeight -= bumpHeightStep;   
        }
        bestBumpHeight += bumpHeightStep;
        for ( int i = 0; i < numBinarySteps; ++ i )
        {
            bumpHeightStep *= 0.5;
            bestBumpHeight -= bumpHeightStep;
            mapHeight       = back_face + surf_sign * CalculateHeight( texC.xy - bestBumpHeight * texStep.xy );
            bestBumpHeight += ( bestBumpHeight < mapHeight ) ? bumpHeightStep : 0.0;
        }
        bestBumpHeight -= bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / bumpHeightStep, 0.0, 1.0 );
        mapHeight       = bestBumpHeight;
        texC           -= mapHeight * texStep;
    }
    else
    {
        texC          += texStep.xy * base_height;
        bestBumpHeight = base_height;
        for ( int i = 0; i < int( numSteps ); ++ i )
        {
            mapHeight = CalculateHeight( texC.xy - bestBumpHeight * texStep.xy );
            if ( mapHeight >= bestBumpHeight || bestBumpHeight >= 1.0 )
                break;
            bestBumpHeight += bumpHeightStep;   
        }
        bestBumpHeight -= bumpHeightStep;
        for ( int i = 0; i < numBinarySteps; ++ i )
        {
            bumpHeightStep *= 0.5;
            bestBumpHeight += bumpHeightStep;
            mapHeight       = CalculateHeight( texC.xy - bestBumpHeight * texStep.xy );
            bestBumpHeight -= ( bestBumpHeight < mapHeight ) ? bumpHeightStep : 0.0;
        }
        bestBumpHeight += bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / bumpHeightStep, 0.0, 1.0 );
        mapHeight       = bestBumpHeight;
        texC           -= mapHeight * texStep;
    }
    
    return vec3( texC.xy, mapHeight );
}
*/


vec3 Parallax( in float frontFace, in vec3 texDir3D, in vec3 texCoord )
{   
    // sample steps and quality
    vec2  quality_range  = u_parallax_quality;
    float quality        = mix( quality_range.x, quality_range.y, 1.0 - pow(abs(normalize(texDir3D).z),2.0) );
    float numSteps       = clamp( quality * 50.0, 1.0, 50.0 );
    int   numBinarySteps = int( clamp( quality * 10.0, 1.0, 7.0 ) );
    
    // intersection direction and start height
    vec2  texStep        = texDir3D.xy / abs(texDir3D.z); // (z is negative) the direction vector points downwards int tangent-space
    float base_height    = texCoord.p;

    // intersection direction: -1 for downwards or 1 for upwards
    // downwards for base triangles (back faces are inverted)
    // upwards for upwards intersection of silhouettes
    float isect_dir      = base_height == 0.0 ? -1.0 : sign(texDir3D.z);

    // inverse height map: -1 for inverse height map or 1 if not inverse
    // height maps of back faces base triangles are inverted
    float inverse_dir    = base_height > 0.01 ? 1.0 : frontFace;
    float back_face      = step(0.0, -inverse_dir); 

    // start texture coordinates
    float start_height   = -isect_dir * base_height + back_face; // back_face is either 1.0 or 0.0  
    vec2  texC           = texCoord.st + start_height * texStep.xy;

    // change of the height per step
    float bumpHeightStep = isect_dir / numSteps;

    // sample steps, starting before the target point (dependent on the maximum height)
    float mapHeight      = 1.0;
    float bestBumpHeight = isect_dir > 0.0 ? base_height : 1.0;
    for ( int i = 0; i < int( numSteps ); ++ i )
    {
        mapHeight = back_face + inverse_dir * CalculateHeight( texC.xy + isect_dir * bestBumpHeight * texStep.xy );
        if ( mapHeight >= bestBumpHeight || bestBumpHeight > 1.0 )
            break;
        bestBumpHeight += bumpHeightStep;   
    } 

    // binary steps, starting at the previous sample point 
    bestBumpHeight -= bumpHeightStep;
    for ( int i = 0; i < numBinarySteps; ++ i )
    {
        bumpHeightStep *= 0.5;
        bestBumpHeight += bumpHeightStep;
        mapHeight       = back_face + inverse_dir * CalculateHeight( texC.xy + isect_dir * bestBumpHeight * texStep.xy );
        bestBumpHeight -= ( bestBumpHeight < mapHeight ) ? bumpHeightStep : 0.0;
    }

    // final linear interpolation between the last to heights 
    bestBumpHeight += bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / abs(bumpHeightStep), 0.0, 1.0 );

    // set displaced texture coordiante and intersection height
    texC      += isect_dir * bestBumpHeight * texStep.xy;
    mapHeight  = bestBumpHeight;
   
    return vec3(texC.xy, mapHeight);
}


void main()
{
    float clip_dist      = in_data.clip;
    if ( clip_dist < 0.0 )
        discard;

    vec3 objPosEs    = in_data.pos;
    vec3 objNormalEs = in_data.nv;
    vec3 texCoords   = in_data.uvh.stp;
    vec3 normalEs    = ( gl_FrontFacing ? 1.0 : -1.0 ) * normalize( objNormalEs );
    
    //vec3  tangentEs    = normalize( tangentVec - normalEs * dot(tangentVec, normalEs ) );
    //mat3  tbnMat       = mat3( tangentEs, binormalSign * cross( normalEs, tangentEs ), normalEs );

    // tangent space
    // Followup: Normal Mapping Without Precomputed Tangents [http://www.thetenthplanet.de/archives/1180]
    vec3  N           = objNormalEs;
    vec3  T           = in_data.tv;
    vec3  B           = in_data.bv;
    float invmax      = inversesqrt(max(dot(T, T), dot(B, B)));
    mat3  tbnMat      = mat3(T * invmax, B * invmax, N * invmax);
   
    vec3  texDir3D     = normalize( inverse( tbnMat ) * objPosEs );
    float frontFace    = gl_FrontFacing ? 1.0 : -1.0; // TODO $$$ sign(dot(N,objPosEs));
    vec3  newTexCoords = abs(u_displacement_scale) < 0.001 ? vec3(texCoords.st, 0.0) : Parallax( frontFace, texDir3D, texCoords.stp );

    // TODO $$$ calcualte depth by adding length( texDir3D.xy / texDir3D.z ) * newTexCoords.z

    vec2  range_vec  = step(vec2(0.0), newTexCoords.st) * step(newTexCoords.st, vec2(1.0));
    float range_test = range_vec.x * range_vec.y;
    if ( texCoords.p > 0.0 && (range_test == 0.0 || newTexCoords.z > 1.0))
    //if ( texCoords.p > 0.0 && range_test == 0.0)
      discard;
    //if ( cosDir > 0.0 )
    //  discard;

    // discard by test against 3 clip planes (riangle prism), similar clip distance 

    texCoords.st       = newTexCoords.xy;
    
    vec4  normalVec    = CalculateNormal( texCoords.st );
    tbnMat[2].xyz      = (gl_FrontFacing ? 1.0 : -1.0) * N / u_displacement_scale;  
    vec3  nvMappedEs   = normalize( tbnMat * normalVec.xyz );

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

    // debug
    //float gray = dot(lightCol.rgb, vec3(0.2126, 0.7152, 0.0722));
    //fragColor = vec4( vec3( step(0.0, -frontFace), step(0.0, texDir3D.z), step(0.0, -texDir3D.z) ) * gray, 1.0 );

    //vec3 newObjPosEs = objPosEs + normalize(objPosEs) * length(tbnMat * texDir3D * newTexCoords.w) * sign(newTexCoords.w);
    //vec3 newObjPosEs = objPosEs;
    //vec4 objPosClip  = u_projectionMat44 * vec4(newObjPosEs.xyz, 1.0); 
    //gl_FragDepth     = 0.5 + 0.5 * objPosClip.z/objPosClip.w;
}