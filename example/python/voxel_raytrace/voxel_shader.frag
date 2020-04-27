#version 460 core
//#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in TVertexData
{
    vec3 pos;
    vec3 nv;
    vec3 col;
} inData;

layout (location = 0) out vec4 fragColor;

struct TMVP
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(std430, binding = 1) buffer MVP
{
    TMVP v[];
} mvp;

layout(std430, binding = 2) buffer TLight
{
    vec4  u_lightDir;
    float u_ambient;
    float u_diffuse;
    float u_specular;
    float u_shininess;
} light_data;

layout(std430, binding = 3) buffer Cube
{
    mat4 model;
    float color_mix;
} cube;

layout(binding = 1) uniform usampler3D voxelSampler;

ivec3 VoxelSize();
uint VoxelLookup(in ivec3 map);
uint VoxelIntersect(in vec3 origin, in vec3 dir, out int side, out ivec3 map);
float VoxelSSAO(in ivec3 map, in ivec3 iside, in vec3 hitPt);
float VoxelShadow(in vec3 hitPt, in vec3 normal, in vec3 dir, in vec3 lightvec);

void main()
{
    //mat4 mv_mat     = mvp.v[0].view * mvp.v[0].model;
    //mat4 mv_mat_inv = inverse(mv_mat);

    // fragment attributes
    vec3 normalV = normalize( inData.nv );
    vec3 eyeV    = normalize( -inData.pos );
    vec3 color   = inData.col;

    vec3 lightV  = normalize(-light_data.u_lightDir.xyz);
    //vec3 lightV  = normalize(mat3(mvp.v[0].view) * -light_data.u_lightDir.xyz);

    mat4 cube_mv = mvp.v[0].view * mvp.v[0].model * cube.model;
    mat4 cube_space = inverse(cube_mv);

    vec3 cube_eye    = cube_space[3].xyz;
    vec3 cube_dir    = mat3(cube_space) * -eyeV;
    vec3 cube_pos    = (cube_space * vec4(inData.pos, 1.0)).xyz;
    vec3 cube_lightV = mat3(cube_space) * lightV;
    
    ivec3 size = VoxelSize();
    vec3 abs_d = abs(cube_dir);
    float max_d = max(abs_d.x, max(abs_d.y, abs_d.z));
    vec3 offset = cube_dir / max_d / vec3(size);
    
    int side = 0;
    ivec3 map;
    vec3 cube_pos_size = cube_pos * vec3(size); 
    uint voxel = VoxelIntersect(cube_pos_size - offset * 0.001, offset, side, map);
    if (voxel == 0)
        discard;
    bvec3 bside = bvec3(side==0, side==1, side==2);
    ivec3 iside = ivec3(bside) * ivec3(sign(-offset));

    vec3  cube_nv = vec3(iside);
    float delta   = dot(vec3(map + max(iside, 0)) - cube_pos_size, vec3(bside));
    vec3  hitPt   = cube_pos_size + delta * offset / dot(offset, vec3(bside));
    
    vec3 color_uv   = fract(hitPt) * (1.0 - vec3(bside));
    vec3 color_side = vec3(bside);

    float ssao   = VoxelSSAO(map, iside, hitPt);
    float shadow = VoxelShadow(hitPt, cube_nv, offset, cube_lightV);
    
    normalV = mat3(cube_mv) * cube_nv;
    color   = mix(color_uv, vec3(1.0), cube.color_mix) * mix(ssao, 1.0, 0.3);
    if (voxel == 2)
        color *= vec3(1.0, 0.2, 0.2);
    else if (voxel == 3)
        color *= vec3(0.2, 1.0, 0.2);
    else if (voxel == 4)
        color *= vec3(0.2, 0.2, 1.0);

    // ambient part
    vec3 lightCol = light_data.u_ambient * color;

    // diffuse part
    float NdotL   = max(0.0, dot(normalV, lightV));
    lightCol     += NdotL * light_data.u_diffuse * color * mix(shadow, 1.0, 0.5);

    // specular part
    vec3  halfV     = normalize( eyeV + lightV );
    float NdotH     = max( 0.0, dot( normalV, halfV ) );
    float kSpecular = ( light_data.u_shininess + 2.0 ) * pow( NdotH, light_data.u_shininess ) / ( 2.0 * 3.14159265 );
    lightCol       += kSpecular * light_data.u_specular * color * shadow;

    fragColor = vec4( lightCol.rgb, 1.0);
}

ivec3 VoxelSize()
{
    return textureSize(voxelSampler, 0);
}

uint VoxelLookup(in ivec3 map)
{
    ivec3 size = VoxelSize();
    if (map.x < 0 || map.x >= size.x || map.y < 0 || map.y >= size.y || map.z < 0 || map.z >= size.z)
        return 0;
    uint voxel = texelFetch(voxelSampler, map, 0).r;
    return voxel;
}

// https://www.dulhanjayalath.com/raytracer
uint VoxelIntersect(in vec3 origin, in vec3 dir, out int side, out ivec3 map)
{
    //ivec3 map = ivec3(origin);
    map = ivec3(origin + 1.0) - 1;
    ivec3 stepAmount;
    vec3 tDelta = abs(1.0 / dir);
    vec3 tMax;
    uint voxel = 0;
    //int side;
    ivec3 size = VoxelSize();

    if (dir.x < 0)
    {
        stepAmount.x = -1;
        tMax.x = (origin.x - map.x) * tDelta.x;
    }
    else if (dir.x > 0)
    {
        stepAmount.x = 1;
        tMax.x = (map.x + 1.0 - origin.x) * tDelta.x;
    }
    else
    {
        stepAmount.x = 0;
        tMax.x = 0;
    }

    if (dir.y < 0)
    {
        stepAmount.y = -1;
        tMax.y = (origin.y - map.y) * tDelta.y;
    }
    else if (dir.y > 0)
    {
        stepAmount.y = 1;
        tMax.y = (map.y + 1.0 - origin.y) * tDelta.y;
    }
    else
    {
        stepAmount.y = 0;
        tMax.y = 0;
    }

    if (dir.z < 0)
    {
        stepAmount.z = -1;
        tMax.z = (origin.z - map.z) * tDelta.z;
    }
    else if (dir.z > 0)
    {
        stepAmount.z = 1;
        tMax.z = (map.z + 1.0 - origin.z) * tDelta.z;
    }
    else
    {
        stepAmount.z = 0;
        tMax.z = 0;
    }

    do
    {
        if (tMax.x < tMax.y)
        {
            if (tMax.x < tMax.z)
            {
                map.x += stepAmount.x;
                //if (map.x >= MAP_WIDTH || map.x < 0)
                    //return vec3(0, 0, 0);
                if ((stepAmount.x > 0 && map.x >= size.x) || (stepAmount.x < 0 && map.x < 0))
                    return 0;
                tMax.x += tDelta.x;
                side = 0;
            }
            else
            {
                map.z += stepAmount.z;
                //if (map.z >= MAP_HEIGHT || map.z < 0)
                    //return vec3(0, 0, 0);
                if ((stepAmount.z > 0 && map.z >= size.z) || (stepAmount.z < 0 && map.z < 0))
                    return 0;
                tMax.z += tDelta.z;
                side = 2;
            }
        }
        else
        {
            if (tMax.y < tMax.z)
            {
                map.y += stepAmount.y;
                //if (map.y >= MAP_DEPTH || map.y < 0)
                    //return vec3(0, 0, 0);
                if ((stepAmount.y > 0 && map.y >= size.y) || (stepAmount.y < 0 && map.y < 0))
                    return 0;
                tMax.y += tDelta.y;
                side = 1;
            }
            else
            {
                map.z += stepAmount.z;
                //if (map.z >= MAP_HEIGHT || map.z < 0)
                    //return vec3(0, 0, 0);
                if ((stepAmount.z > 0 && map.z >= size.z) || (stepAmount.z < 0 && map.z < 0))
                    return 0;
                tMax.z += tDelta.z;
                side = 2;
            }
        }
        //voxel = world_map[map.y * MAP_WIDTH * MAP_HEIGHT + map.z * MAP_WIDTH + map.x];
        voxel = VoxelLookup(map);
    } while (voxel == 0);

    return voxel;
}

float VoxelSSAO(in ivec3 map, in ivec3 iside, in vec3 hitPt)
{
    float ssao = 1.0;
    vec3 dir = fract(hitPt) * 2.0 - 1.0; 
    ivec3 idir = ivec3(sign(dir));

    if (iside.x == 0)
    {
        if (VoxelLookup(map + iside + ivec3(idir.x, 0, 0)) != 0)
            ssao *= 1.0 - abs(dir.x);
        //if (VoxelLookup(map + ivec3(idir.x, 0, 0)) == 0)
        //    ssao *= 1.0 + abs(dir.x);
    }
    if (iside.y == 0)
    {
        if (VoxelLookup(map + iside + ivec3(0, idir.y, 0)) != 0)
            ssao *= 1.0 - abs(dir.y);
        //if (VoxelLookup(map + ivec3(0, idir.y, 0)) == 0)
        //    ssao *= 1.0 + abs(dir.y);
    }
    if (iside.z == 0)
    {
        if (VoxelLookup(map + iside + ivec3(0, 0, idir.z)) != 0)
            ssao *= 1.0 - abs(dir.z);
        //if (VoxelLookup(map + ivec3(0, 0, idir.z)) == 0)
         //   ssao *= 1.0 + abs(dir.z);
    }

    //return ssao;
    return 1.0 - ((1.0 - ssao) * (1.0 - ssao));
}

float VoxelShadow(in vec3 hitPt, in vec3 normal, in vec3 dir, in vec3 lightvec)
{
    if (dot(normal, lightvec) <= 0.0)
        return 0.0;
    
    int side;
    ivec3 map;
    vec3 origin = hitPt + lightvec * 0.001;
    if (VoxelIntersect(origin, lightvec, side, map) != 0)
        return 0.0;

    return 1.0;
}