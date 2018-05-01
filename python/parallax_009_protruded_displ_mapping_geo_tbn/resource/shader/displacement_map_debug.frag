#version 400

//#define NORMAL_MAP_TEXTURE
#define NORMAL_MAP_QUALITY 1

#define CONE_STEP_MAPPING

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
    float height = CalculateHeight( texCoords );
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


vec3 Parallax( in float frontFace, in vec3 tbnDir, in vec3 texCoord )
{   
    // geometry situation
    float base_height  = texCoord.p;                       // intersection level (height) on the silhouette (side of prism geometry)
    bool  is_sihouette = texCoord.p > 0.00001;             // fragment is on a potential silhouette (side of prism geometry)
    bool  is_up_isect  = is_sihouette && tbnDir.z > 0.0; // upwards intersection on potential silhouette (side of prism geometry)

    // sample start and end height (level)
    float maxBumpHeight   = 1.0;
    float delta_height0   = is_up_isect ? 1.05*(1.0-base_height) : base_height; // TODO $$$ 1.05 ??? 
    float delta_height1   = is_up_isect ? 0.0 : (base_height - maxBumpHeight);

    // sample distance
    //vec3 texDist = tbnDir / abs(tbnDir.z); // (z is negative) the direction vector points downwards int tangent-space
    vec3 texDist = is_sihouette == false ? tbnDir / abs(tbnDir.z) : tbnDir / max(abs(tbnDir.z), 0.5*length(tbnDir.xy));
    vec3 texStep = vec3(texDist.xy, sign(tbnDir.z));

    // inverse height map: -1 for inverse height map or 1 if not inverse
    // height maps of back faces base triangles are inverted
    float inverse_dir    = is_sihouette ? 1.0 : frontFace;
    float back_face      = step(0.0, -inverse_dir); 

    // start and end of samples
    vec3 texC0 = texCoord.xyz + (back_face + delta_height0) * texStep; // sample end - bottom of prism 
    vec3 texC1 = texCoord.xyz + (back_face + delta_height1) * texStep; // sample start - top of prism  

    // sample steps and quality
    vec2  quality_range  = u_parallax_quality;
    float quality        = mix( quality_range.x, quality_range.y, 1.0 - abs(normalize(tbnDir).z) );
    float numSteps       = clamp( quality * 50.0, 1.0, 50.0 );
    int   numBinarySteps = int( clamp( quality * 10.0, 1.0, 10.0 ) );

#if defined(CONE_STEP_MAPPING)

    // [Determinante](https://de.wikipedia.org/wiki/Determinante)
    // A x B = A.x * B.y - A.y * B.x = dot(A, vec2(B.y,-B.x)) = det(mat2(A,B))

    float isect_dir       = base_height < 0.0001 ? -1.0 : sign(tbnDir.z);
    float startBumpHeight = isect_dir > 0.0 ? base_height : maxBumpHeight;

    // [How do you detect where two line segments intersect?](https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect)
    vec2 R = normalize(vec2(length(tbnDir.xy), tbnDir.z)); 
    vec2 P = vec2(length(texStep.xy), 1.0) * texC1.z; 
    
    vec2  tex_size     = textureSize(u_displacement_map, 0).xy;
    vec2  min_tex_step = normalize(tbnDir.xy) / tex_size;
    float min_step     = length(min_tex_step) * 1.0/R.x;

    float mapHeight = texC1.z;
    float t = 0.0;
    numSteps = 30.0;
    for ( int i = 0; i < int( numSteps ); ++ i )
    {
        vec3 sample_pt = mix(texC0, texC1, 1.0-t);

        vec2 h_and_c = GetHeightAndCone( sample_pt.xy );
        float h = h_and_c.x * maxBumpHeight;
        float c = h_and_c.y * h_and_c.y / maxBumpHeight;

        mapHeight = h;
        vec2 C = P + R * t;
        if ( C.y <= h )
            break;
        if ( C.y > maxBumpHeight )
            discard;
        
        vec2 Q = vec2(C.x, h);
        vec2 S = normalize(vec2(c, 1.0));
        float new_t = dot(Q-P, vec2(S.y, -S.x)) / dot(R, vec2(S.y, -S.x));
        t = max(t+min_step, new_t); 
    } 

    // set displaced texture coordiante and intersection height
    vec2 texC = mix(texC0.xy, texC1.xy, 1.0-t);
    
#else

    // change of the height per step
    float bumpHeightStep = inverse_dir * (texC0.z-texC1.z) / numSteps;

    float bestBumpHeight = texC1.z;
    float mapHeight      = 1.0;
    for ( int i = 0; i < int( numSteps ); ++ i )
    {
        mapHeight = back_face + inverse_dir * CalculateHeight( mix(texC0.xy, texC1.xy, (bestBumpHeight-texC0.z)/(texC1.z-texC0.z)) );
        if ( mapHeight >= bestBumpHeight || bestBumpHeight > 1.0 )
            break;
        bestBumpHeight += bumpHeightStep;   
    } 

    if ( is_sihouette == false || bestBumpHeight < 1.0 )
    {
        // binary steps, starting at the previous sample point 
        bestBumpHeight -= bumpHeightStep;
        for ( int i = 0; i < numBinarySteps; ++ i )
        {
            bumpHeightStep *= 0.5;
            bestBumpHeight += bumpHeightStep;
            mapHeight       = back_face + inverse_dir * CalculateHeight( mix(texC0.xy, texC1.xy, (bestBumpHeight-texC0.z)/(texC1.z-texC0.z))  );
            bestBumpHeight -= ( bestBumpHeight < mapHeight ) ? bumpHeightStep : 0.0;
        }

        // final linear interpolation between the last to heights 
        bestBumpHeight += bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / abs(bumpHeightStep), 0.0, 1.0 );
    }

    // set displaced texture coordiante and intersection height
    vec2 texC  = mix(texC0.xy, texC1.xy, (bestBumpHeight-texC0.z)/(texC1.z-texC0.z));
    mapHeight  = bestBumpHeight;

#endif
    
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
   
    vec3  tbnDir     = normalize( inv_tbnMat * objPosEs );
    vec3  newTexCoords = abs(u_displacement_scale) < 0.001 ? vec3(texCoords.st, 0.0) : Parallax( frontFace, tbnDir, texCoords.stp );
    vec3  displ_vec    = tbnMat * (newTexCoords.stp-texCoords.stp)/invmax;
    vec3  view_pos_displ = objPosEs + displ_vec;

    vec2  range_vec  = step(vec2(0.0), newTexCoords.st) * step(newTexCoords.st, vec2(1.0));
    float range_test = range_vec.x * range_vec.y;
    if ( texCoords.p > 0.0 && (range_test == 0.0 || newTexCoords.z > 1.000001))
      discard;

    texCoords.st       = newTexCoords.xy;

#define DEBUG_CLIP
#define DEBUG_CLIP_DISPLACED

#if defined (DEBUG_CLIP)
    vec4  modelPos       = inverse(u_viewMat44) * vec4(view_pos_displ, 1.0);
    vec4  clipPlane      = vec4(normalize(u_clipPlane.xyz), u_clipPlane.w);
#if defined (DEBUG_CLIP_DISPLACED)
    float clip_dist      = dot(modelPos, clipPlane);
#else
    float clip_dist      = in_data.clip;
#endif
    if ( clip_dist < 0.0 )
        discard;
#endif
    
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

    //if (texCoords.p > 0.0)
    //    depth -= 0.0001;

    gl_FragDepth = depth;

//#define DEBUG_GEOMETRY
//#define DEBUG_DEPTH

#if defined(DEBUG_GEOMETRY)
    float gray = dot(lightCol.rgb, vec3(0.2126, 0.7152, 0.0722));
    //fragColor = vec4( vec3( step(0.0, -frontFace), step(0.0, tbnDir.z), step(0.0, -tbnDir.z) ) * gray, 1.0 );
    fragColor = vec4( vec3( step(0.0001, texCoords.p) * step(0.0, tbnDir.z), step(0.0001, texCoords.p) * step(0.0, -tbnDir.z), step(texCoords.p, 0.0001) ) * gray, 1.0 );
    //fragColor = vec4( vec3( step(0.0001, texCoords.p) * step(0.0, tbnDir.z), step(0.0001, texCoords.p) * step(0.0, -tbnDir.z), step(texCoords.p, 0.0001) ) * gray, 1.0 ) * step(0.0, frontFace);
#endif

#if defined(DEBUG_DEPTH)
    fragColor = vec4( vec3(1.0-depth), 1.0 );
#endif
}