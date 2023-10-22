
struct VertexInput
{
    @location(0) position : vec2<f32>,
    @location(1) uv : vec2<f32>,
    @location(2) color : vec4<f32>
};

struct VertexOutput
{
    @builtin(position) clip_position : vec4<f32>,
    @location(0) uv : vec2<f32>,
    @location(1) color : vec4<f32>
};

struct FragmentOutput
{
    @location(0) frag_color : vec4<f32>
};

struct PerFrameData
{
    viewProj : mat4x4<f32>
};

@group(0)
@binding(0)
var<uniform> per_frame_data : PerFrameData;

@group(1)
@binding(0)
var diffuse_sampler : sampler;

@group(1)
@binding(1)
var diffuse_texture : texture_2d<f32>;

@vertex
fn vs_main(in : VertexInput) -> VertexOutput 
{
    var out: VertexOutput;
    out.uv = in.uv;
    out.color = in.color;
    out.clip_position = per_frame_data.viewProj * vec4<f32>(in.position, 0.0, 1.0);

    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> FragmentOutput {
    var color = in.color * textureSample(diffuse_texture, diffuse_sampler, in.uv);

    var out: FragmentOutput;    
    out.frag_color = color;

    return out;
}