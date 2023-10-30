struct PerFrameData
{
  view: mat4x4<f32>,
  proj: mat4x4<f32>,
  transform: mat4x4<f32>
};

struct SkyboxMaterial
{
    base_color_factor: vec4<f32>
};

@group(0)
@binding(0)
var<uniform> per_frame_data: PerFrameData;

@group(1)
@binding(0)
var<uniform> material: SkyboxMaterial;

#ifdef HAS_ENV_MAP_TEXTURE
@group(1)
@binding(1)
var env_map_sampler: sampler;

@group(1)
@binding(2)
var env_map_texture: texture_cube<f32>;
#endif

struct VertexInput
{
    @location(0) position : vec3<f32>
};

struct FragmentInput
{
    @builtin(position) clip_position : vec4<f32>,
    @location(0) position : vec3<f32>,
};

struct FragmentOutput
{
    @location(0) frag_color : vec4<f32>
};

@vertex
fn vs_main(in : VertexInput) -> FragmentInput 
{
    var out: FragmentInput;
    out.position = in.position;
    out.clip_position = per_frame_data.proj * per_frame_data.view * per_frame_data.transform *
        vec4<f32>(in.position, 1.0);

    return out;
}

@fragment
fn fs_main(in: FragmentInput) -> FragmentOutput {
    var color = material.base_color_factor.rgb;
    
#ifdef HAS_ENV_MAP_TEXTURE    
    color = textureSample(env_map_texture, env_map_sampler, normalize(in.position)).rgb;
#endif

    var out: FragmentOutput;    
    out.frag_color = vec4<f32>(color, 1.0);

    return out;
}
