#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1

struct Transform
{
  model: mat4x4<f32>,
  rotation: mat4x4<f32>
};

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

struct PBRMaterial
{
  emissive: vec4<f32>,
  base_color_factor: vec4<f32>,
  metallic_factor: f32,
  roughness_factor: f32,
  padding: vec2<f32>
};

@group(0)
@binding(0)
var<uniform> per_frame_data: PerFrameData;

@group(0)
@binding(1)
var<storage, read> lights: array<Light>;

@group(1)
@binding(0)
var<uniform> pbr_material: PBRMaterial;

#ifdef HAS_BASE_COLOR_TEXTURE
@group(1)
@binding(1)
var base_color_sampler: sampler;

@group(1)
@binding(2)
var base_color_texture: texture_2d<f32>;
#endif

#ifdef HAS_NORMAL_TEXTURE
@group(1)
@binding(3)
var normal_sampler: sampler;

@group(1)
@binding(4)
var normal_texture: texture_2d<f32>;
#endif

#ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
@group(1)
@binding(5)
var metallic_roughness_sampler: sampler;

@group(1)
@binding(6)
var metallic_roughness_texture: texture_2d<f32>;
#endif

@group(2)
@binding(0)
var<uniform> transform: Transform;

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

const PI = 3.14159265359;

fn f_schlick(f0: vec3<f32>, f90: f32, u: f32) -> vec3<f32>
{
  return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

fn f_schlick_roughness(f0: vec3<f32>, cos_theta: f32, roughness: f32) -> vec3<f32>
{
  return f0 + (max(vec3<f32>(1.0 - roughness), f0) - f0) * pow(1.0 - cos_theta, 5.0);
}

fn fr_disney_diffuse(ndotv: f32, ndotl: f32, ldoth: f32, roughness: f32) -> f32
{
  var e_bias = 0.0 * (1.0 - roughness) + 0.5 * roughness;
  var e_factor = 1.0 * (1.0 - roughness) + (1.0 / 1.51) * roughness;
  var fd90 = e_bias + 2.0 * ldoth * ldoth * roughness;
  var f0 = vec3<f32>(1.0);
  var light_scatter = f_schlick(f0, fd90, ndotl).r;
  var view_scatter = f_schlick(f0, fd90, ndotv).r;

  return light_scatter * view_scatter * e_factor;
}

fn v_smith_ggx_correlated(ndotv: f32, ndotl: f32, roughness: f32) -> f32
{
  var alpha_roughness_sq = roughness * roughness;
  var ggxv = ndotl * sqrt(ndotv * ndotv * (1.0 - alpha_roughness_sq) + alpha_roughness_sq);
  var ggxl = ndotv * sqrt(ndotl * ndotl * (1.0 - alpha_roughness_sq) + alpha_roughness_sq);

  var ggx = ggxv + ggxl;
  if (ggx > 0.0)
  {
    return 0.5 / ggx;
  }

  return 0.0;
}

fn d_ggx(ndoth: f32, roughness: f32) -> f32
{
  var alpha_roughness_sq = roughness * roughness;
  var f = (ndoth * alpha_roughness_sq - ndoth) * ndoth + 1.0;

  return alpha_roughness_sq / (PI * f * f);
}

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

#ifdef HAS_NORMAL_TEXTURE
  var tn = textureSample(normal_texture, normal_sampler, in_uv).rgb;
  return normalize(tbn * (2.0 * tn - 1.0));
#else
  return normalize(tbn[2].xyz);
#endif
}

fn diffuse(albedo: vec3<f32>, metallic: f32) -> vec3<f32>
{
  return albedo * (1.0 - metallic) + ((1.0 - metallic) * albedo) * metallic;
}

fn f_saturate(t: f32) -> f32
{
  return clamp(t, 0.0, 1.0);
}

fn v_saturate(t: vec3<f32>) -> vec3<f32>
{
  return clamp(t, vec3<f32>(0.0), vec3<f32>(1.0));
}

fn apply_directional_light(light: Light, normal: vec3<f32>) -> vec3<f32>
{
  var world_to_light = normalize(-light.direction.xyz);
  var ndotl = clamp(dot(normal, world_to_light), 0.0, 1.0);

  return ndotl * light.color.w * light.color.rgb;
}

fn apply_point_light(light: Light, in_pos: vec3<f32>, normal: vec3<f32>) -> vec3<f32>
{
  var world_to_light = light.position.xyz - in_pos.xyz;
  var dist = length(world_to_light);
  var atten = 1.0 / (dist * dist);

  world_to_light = normalize(world_to_light);
  var ndotl = clamp(dot(normal, world_to_light), 0.0, 1.0);

  return ndotl * light.color.w * atten * light.color.rgb;
}

fn get_light_direction(light: Light, in_pos: vec3<f32>) -> vec3<f32>
{
  if (light.position.w == DIRECTIONAL_LIGHT)
  {
    return -light.direction.xyz;
  }

  if (light.position.w == POINT_LIGHT)
  {
    return light.position.xyz - in_pos.xyz;
  }

  return -light.direction.xyz;
}

@vertex
fn vs_main(in: VertexInput) -> FragmentInput
{
  var out: FragmentInput;
  var local_pos = transform.model * vec4<f32>(in.position, 1.0);
    
  out.uv = in.uv;  
  out.normal = (transform.rotation * vec4<f32>(in.normal, 0.0)).xyz;
  out.clip_position = per_frame_data.proj * per_frame_data.view * local_pos;
  out.position = local_pos.xyz;

  return out;
}

@fragment
fn fs_main(in: FragmentInput) -> FragmentOutput
{
  var out: FragmentOutput;
  var f0 = vec3<f32>(0.04);
  var f90 = f_saturate(50.0 * f0.r);
  var base_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);

#ifdef HAS_BASE_COLOR_TEXTURE
  base_color = textureSample(base_color_texture, base_color_sampler, in.uv);
#else
  base_color = pbr_material.base_color_factor;
#endif

#ifdef HAS_METALLIC_ROUGHNESS_TEXTURE
  var roughness = f_saturate(textureSample(metallic_roughness_texture, metallic_roughness_sampler, in.uv).g);
  var metallic = f_saturate(textureSample(metallic_roughness_texture, metallic_roughness_sampler, in.uv).b);
#else
  var roughness = pbr_material.roughness_factor;
  var metallic = pbr_material.metallic_factor;
#endif

  var n = normal(in.position, in.normal, in.uv);
  var v = normalize(per_frame_data.camera_pos - in.position);
  var ndotv = f_saturate(dot(n, v));

  var light_contribution = vec3<f32>(0.0);
  var diffuse_color = base_color.rgb * (1.0 - metallic);

  for (var i: u32 = 0; i < per_frame_data.num_lights; i++)
  {
    var light = lights[i];
    var l = get_light_direction(light, in.position);
    var h = normalize(v + l);

    var ldoth = f_saturate(dot(l, h));
    var ndoth = f_saturate(dot(n, h));
    var ndotl = f_saturate(dot(n, l));

    var f = f_schlick(f0, f90, ldoth);
    var d = d_ggx(ndoth, roughness);
    var vis = v_smith_ggx_correlated(ndotv, ndotl, roughness);
    var fr = f * d * vis;
    var fd = fr_disney_diffuse(ndotv, ndotl, ldoth, roughness);

    if ( light.position.w == DIRECTIONAL_LIGHT)
    {
      light_contribution += apply_directional_light(light, n) * (diffuse_color * (vec3<f32>(1.0) - f) * fd + fr);
    }

    if ( light.position.w == POINT_LIGHT)
    {
      light_contribution += apply_point_light(light, in.position, n) * (diffuse_color * (vec3<f32>(1.0) - f) * fd + fr);
    }
  }

  var irradiance = vec3<f32>(0.5);
  var f = f_schlick_roughness(f0, max(dot(n, v), 0.0), roughness * roughness * roughness * roughness);
  var ibl_diffuse = irradiance * base_color.rgb;
  out.frag_color = vec4<f32>(0.3 * ibl_diffuse + light_contribution, base_color.a);

  return out;
}