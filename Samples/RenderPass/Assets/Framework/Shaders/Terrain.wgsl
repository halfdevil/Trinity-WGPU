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

struct TerrainData
{
  scale: vec4<f32>,
  offset: vec4<f32>,
  quad_world_max: vec2<f32>,
  world_to_texture_scale: vec2<f32>,
  grid_dimension: vec4<f32>,
  height_map_texture_info: vec4<f32>
};

struct QuadData
{
  morph_consts: vec4<f32>,
  offset: vec4<f32>,
  scale: vec4<f32>
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

@group(0)
@binding(2)
var<uniform> terrain_data: TerrainData;

@group(0)
@binding(3)
var<uniform> quad_data: QuadData;

@group(1)
@binding(0)
var<uniform> material: TerrainMaterial;

@group(1)
@binding(1)
var height_map_sampler: sampler;

@group(1)
@binding(2)
var height_map_texture: texture_2d<f32>;

@group(1)
@binding(3)
var normal_map_sampler: sampler;

@group(1)
@binding(4)
var normal_map_texture: texture_2d<f32>;

#ifdef HAS_BLEND_MAP_TEXTURE
@group(1)
@binding(5)
var blend_map_sampler: sampler;

@group(1)
@binding(6)
var blend_map_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER1_TEXTURE
@group(1)
@binding(7)
var layer1_sampler: sampler;

@group(1)
@binding(8)
var layer1_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER2_TEXTURE
@group(1)
@binding(9)
var layer2_sampler: sampler;

@group(1)
@binding(10)
var layer2_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER3_TEXTURE
@group(1)
@binding(11)
var layer3_sampler: sampler;

@group(1)
@binding(12)
var layer3_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER4_TEXTURE
@group(1)
@binding(13)
var layer4_sampler: sampler;

@group(1)
@binding(14)
var layer4_texture: texture_2d<f32>;
#endif

#ifdef HAS_LAYER5_TEXTURE
@group(1)
@binding(15)
var layer5_sampler: sampler;

@group(1)
@binding(16)
var layer5_texture: texture_2d<f32>;
#endif

struct VertexInput
{
  @location(0) position: vec3<f32>
};

struct FragmentInput
{
  @builtin(position) clip_position: vec4<f32>,
  @location(0) position: vec3<f32>,
  @location(1) uv: vec2<f32>
};

struct FragmentOutput
{
    @location(0) frag_color : vec4<f32>
};

fn get_base_vertex_pos(in_pos: vec4<f32>) -> vec4<f32>
{
  var ret = in_pos * quad_data.scale + quad_data.offset;
  ret.x = min(ret.x, terrain_data.quad_world_max.x);
  ret.z = min(ret.z, terrain_data.quad_world_max.y);

  return ret;
}

fn morph_vertex(in_pos: vec4<f32>, vertex: vec2<f32>, morph_lerp_k: f32) -> vec2<f32>
{
  var frac_part = (fract(in_pos.xz * 
    vec2<f32>(terrain_data.grid_dimension.z, terrain_data.grid_dimension.z)) * 
    vec2<f32>(terrain_data.grid_dimension.y, terrain_data.grid_dimension.y)) * quad_data.scale.xz;
  
  return vertex.xy - frac_part * morph_lerp_k;
}

fn calculate_uv(vertex: vec2<f32>) -> vec2<f32>
{
  var uv = (vertex.xy - terrain_data.offset.xz) / terrain_data.scale.xz;
  uv *= terrain_data.world_to_texture_scale;
  uv += terrain_data.height_map_texture_info.zw * 0.5;

  return uv;
}

fn sample_heightmap(uv : vec2<f32>) -> f32
{
  let texture_size = terrain_data.height_map_texture_info.xy;
  let texel_size = terrain_data.height_map_texture_info.zw;

  var n_uv = uv * texture_size - vec2<f32>(0.5, 0.5);
  var uvf = floor(n_uv);
  var f = n_uv - uvf;
  n_uv = (uvf + vec2<f32>(0.5, 0.5)) * texel_size;

  var c1 = u32(n_uv.x * texture_size.x);
  var c2 = u32(n_uv.y * texture_size.y);
  var c3 = u32((n_uv.x + texel_size.x) * texture_size.x);
  var c4 = u32((n_uv.y + texel_size.y) * texture_size.y);

  var t00 = textureLoad(height_map_texture, vec2<u32>(c1, c2), 0).x;
  var t10 = textureLoad(height_map_texture, vec2<u32>(c3, c2), 0).x;
  var ta = mix(t00, t10, f.x);

  var t01 = textureLoad(height_map_texture, vec2<u32>(c1, c4), 0).x;
  var t11 = textureLoad(height_map_texture, vec2<u32>(c3, c4), 0).x;
  var tb = mix(t01, t11, f.x);

  return mix(ta, tb, f.y);
}

fn normal(in_pos: vec3<f32>, in_uv: vec2<f32>) -> vec3<f32>
{
  var pos_dx = dpdx(in_pos);
  var pos_dy = dpdy(in_pos);
  var st1 = dpdx(vec3<f32>(in_uv, 0.0));
  var st2 = dpdy(vec3<f32>(in_uv, 0.0));
  var t = (st2.y * pos_dx - st1.y * pos_dy) / (st1.x * st2.y - st2.x * st1.y);
  
  var n = textureSample(normal_map_texture, normal_map_sampler, in_uv).rgb;
  n = n * vec3<f32>(2.0, 2.0, 0.0) - vec3<f32>(1.0, 1.0, 0.0);
  n.z = sqrt(1.0 - n.x * n.x - n.y * n.y);

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

  var pre_vertex = get_base_vertex_pos(vec4<f32>(in.position, 1.0));
  var pre_uv = calculate_uv(pre_vertex.xy);
  pre_vertex.y = sample_heightmap(pre_uv);

  var eye_dist = distance(pre_vertex, vec4<f32>(per_frame_data.camera_pos, 1.0));
  var morph_lerp_k = 1.0f - clamp(quad_data.morph_consts.z - eye_dist * quad_data.morph_consts.w, 0.0, 1.0);
  var morph_vertex = morph_vertex(vec4<f32>(in.position, 1.0), pre_vertex.xz, morph_lerp_k);
  
  var uv = calculate_uv(morph_vertex);
  var vertex = vec4<f32>(morph_vertex.x, 0.0, morph_vertex.y, 1.0);
  vertex.y = sample_heightmap(uv);
  
  out.position = vertex.xyz;
  out.uv = uv;  
  out.clip_position = per_frame_data.proj * per_frame_data.view * vertex;

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

    var n = normal(in.position, in.uv);
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