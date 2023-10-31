#define DIRECTIONAL_LIGHT 0

struct PerFrameData
{
  view: mat4x4<f32>,
  proj: mat4x4<f32>,
  camera_pos: vec3<f32>,
  num_lights: u32
};

struct Light
{
  position: vec4<f32>,
  color: vec4<f32>,
  direction: vec4<f32>,
  info: vec2<f32>,
  padding: vec2<f32>
};

struct TerrainMaterial
{
    base_color_factor: vec4<f32>,
    texture_scale: vec4<f32>
};

@group(0)
@binding(0)
var<uniform> per_frame_data: PerFrameData;

@group(0)
@binding(1)
var<storage, read> lights: array<Light>;

@group(1)
@binding(0)
var<uniform> material: TerrainMaterial;

#ifdef HAS_BLEND_MAP_TEXTURE
@group(1)
@binding(1)
var blend_map_sampler: sampler;

@group(1)
@binding(2)
var blend_map_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER1_TEXTURE
@group(1)
@binding(3)
var layer1_sampler: sampler;

@group(1)
@binding(4)
var layer1_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER2_TEXTURE
@group(1)
@binding(5)
var layer2_sampler: sampler;

@group(1)
@binding(6)
var layer2_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER3_TEXTURE
@group(1)
@binding(7)
var layer3_sampler: sampler;

@group(1)
@binding(8)
var layer3_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER4_TEXTURE
@group(1)
@binding(9)
var layer4_sampler: sampler;

@group(1)
@binding(10)
var layer4_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER5_TEXTURE
@group(1)
@binding(11)
var layer5_sampler: sampler;

@group(1)
@binding(12)
var layer5_texture: texture_2d<f32>;
#endif

struct VertexInput
{
  @location(0) position: vec3<f32>,
  @location(1) normal: vec3<f32>,
  @location(2) uv: vec2<f32>
};

struct FragmentInput
{
  @builtin(position) clip_position: vec4<f32>,
  @location(0) position: vec3<f32>,
  @location(1) uv: vec2<f32>,
  @location(2) normal: vec3<f32>
};

struct FragmentOutput
{
    @location(0) frag_color : vec4<f32>
};

fn normal(in_pos: vec3<f32>, in_normal: vec3<f32>, in_uv: vec2<f32>) -> vec3<f32>
{
  var pos_dx = dpdx(in_pos);
  var pos_dy = dpdy(in_pos);
  var st1 = dpdx(vec3<f32>(in_uv, 0.0));
  var st2 = dpdy(vec3<f32>(in_uv, 0.0));
  var t = (st2.y * pos_dx - st1.y * pos_dy) / (st1.x * st2.y - st2.x * st1.y);
  
  var n = normalize(in_normal);
  t = normalize(t - n * dot(n, t));

  var b = normalize(cross(n, t));
  var tbn = mat3x3<f32>(t, b, n);

  return normalize(tbn[2].xyz);
}

fn get_light_direction(light: Light, in_pos: vec3<f32>) -> vec3<f32>
{
  if (light.position.w == DIRECTIONAL_LIGHT)
  {
    return -light.direction.xyz;
  }

  return -light.direction.xyz;
}

fn apply_directional_light(light: Light, normal: vec3<f32>) -> vec3<f32>
{
  var world_to_light = normalize(-light.direction.xyz);
  var ndotl = clamp(dot(normal, world_to_light), 0.0, 1.0);

  return ndotl * light.color.w * light.color.rgb;
}

@vertex
fn vs_main(in: VertexInput) -> FragmentInput
{
  var out: FragmentInput;
  
  out.position = in.position;
  out.normal = in.normal;
  out.uv = in.uv;  
  out.clip_position = per_frame_data.proj * per_frame_data.view * vec4<f32>(in.position, 1.0);

  return out;
}

@fragment
fn fs_main(in: FragmentInput) -> FragmentOutput
{
    var out: FragmentOutput;
    var base_color = vec4<f32>(0.0, 0.0, 0.0, 1.0);
    var light_contribution = vec3<f32>(0.0);

#ifdef HAS_BLEND_MAP_TEXTURE
    var t = textureSample(blend_map_texture, blend_map_sampler, in.uv);
    base_color = material.base_color_factor;

#ifdef HAS_LAYER1_TEXTURE
    base_color = textureSample(layer1_texture, layer1_sampler, in.uv * 10.0f);
#endif

#ifdef HAS_LAYER2_TEXTURE
    var c2 = textureSample(layer2_texture, layer2_sampler, in.uv * 10.0f);
    base_color = mix(base_color, c2, t.r);
#endif

#ifdef HAS_LAYER3_TEXTURE
    var c3 = textureSample(layer3_texture, layer3_sampler, in.uv * 10.0f);
    base_color = mix(base_color, c3, t.g);
#endif

#ifdef HAS_LAYER4_TEXTURE
    var c4 = textureSample(layer4_texture, layer4_sampler, in.uv * 10.0f);
    base_color = mix(base_color, c4, t.b);
#endif

#ifdef HAS_LAYER5_TEXTURE
    var c5 = textureSample(layer5_texture, layer5_sampler, in.uv * 10.0f);
    base_color = mix(base_color, c5, t.a);
#endif
#else
    base_color = material.base_color_factor;
#endif

    var n = normal(in.position, in.normal, in.uv);

    for (var i: u32 = 0; i < per_frame_data.num_lights; i++)
    {
        var light = lights[i];
        var l = get_light_direction(light, in.position);

        if (light.position.w == DIRECTIONAL_LIGHT)
        {
            light_contribution += apply_directional_light(light, n);
        }
    }

    out.frag_color = vec4<f32>(base_color.rgb + light_contribution, base_color.a);
    return out;
}