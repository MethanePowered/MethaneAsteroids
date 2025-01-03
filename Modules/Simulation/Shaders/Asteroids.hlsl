/******************************************************************************

Copyright 2019-2021 Evgeny Gorodetskiy

Licensed under the Apache License, Version 2.0 (the "License"),
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*******************************************************************************

FILE: MethaneKit/Apps/Samples/Asteroids/Shaders/Asteroids.hlsl
Shaders for asteroids rendering with tri-planar texturing and Phong lighting.
Asteroid textures can be bound indirectly with array of textures and selected
using uniform texture index or bound directly with descriptor table.

Optional macro definition: TEXTURES_COUNT=10

******************************************************************************/

#include "SceneConstants.h"
#include "AsteroidUniforms.h"
#include "Primitives.hlsl"

#ifndef TEXTURES_COUNT
#define TEXTURES_COUNT 1
#endif

struct VSInput
{
    float3 position          : POSITION;
    float3 normal            : NORMAL;
};

struct PSInput
{
    float4 position          : SV_POSITION;
    float4 world_position    : POSITION0;
    float3 world_normal      : NORMAL;
    float3 albedo            : ALBEDO;
    float3 uvw               : UVFACE;
    float3 face_blend_weights: BLENDWEIGHT;
};

ConstantBuffer<AsteroidUniforms> g_mesh_uniforms                 : register(b0, META_ARG_MUTABLE);
ConstantBuffer<SceneUniforms>    g_scene_uniforms                : register(b1, META_ARG_FRAME_CONSTANT);
ConstantBuffer<SceneConstants>   g_constants                     : register(b2, META_ARG_CONSTANT);
SamplerState                     g_texture_sampler               : register(s0, META_ARG_CONSTANT);
Texture2DArray<float4>           g_face_textures[TEXTURES_COUNT] : register(t0,
#if TEXTURES_COUNT > 1
    META_ARG_CONSTANT
#else
    META_ARG_MUTABLE
#endif
);

PSInput AsteroidVS(VSInput input)
{
    const float4 position = float4(input.position, 1.0F);
    const float  depth    = linstep(g_mesh_uniforms.depth_min, g_mesh_uniforms.depth_max, length(input.position.xyz));

    PSInput output;
    output.world_position    = mul(position, g_mesh_uniforms.model_matrix);
    output.position          = mul(output.world_position, g_scene_uniforms.view_proj_matrix);

    output.world_normal      = normalize(mul(float4(input.normal, 0.0), g_mesh_uniforms.model_matrix).xyz);
    output.albedo            = lerp(g_mesh_uniforms.deep_color, g_mesh_uniforms.shallow_color, depth);

    // Prepare coordinates and blending weights for tri-planar projection texturing
    output.uvw               = input.position / g_mesh_uniforms.depth_max * 0.5F + 0.5F;
    output.face_blend_weights = abs(normalize(input.position));
    output.face_blend_weights = saturate((output.face_blend_weights - 0.2F) * 7.0F);
    output.face_blend_weights /= (output.face_blend_weights.x + output.face_blend_weights.y + output.face_blend_weights.z).xxx;

    return output;
}

float4 AsteroidPS(PSInput input) : SV_TARGET
{
    const float3 fragment_to_light  = normalize(g_scene_uniforms.light_position - input.world_position.xyz);
    const float3 fragment_to_eye    = normalize(g_scene_uniforms.eye_position - input.world_position.xyz);
    const float3 light_reflected_from_fragment = reflect(-fragment_to_light, input.world_normal);

    // Tri-planar projection sampling
    float3 texel_rgb = 0.0;
    const uint tex_index = g_mesh_uniforms.texture_index;
    texel_rgb += input.face_blend_weights.x * g_face_textures[tex_index].Sample(g_texture_sampler, float3(input.uvw.yz, 0)).xyz;
    texel_rgb += input.face_blend_weights.y * g_face_textures[tex_index].Sample(g_texture_sampler, float3(input.uvw.zx, 1)).xyz;
    texel_rgb += input.face_blend_weights.z * g_face_textures[tex_index].Sample(g_texture_sampler, float3(input.uvw.xy, 2)).xyz;

    const float4 texel_color    = float4(texel_rgb * input.albedo, 1.0);
    const float4 ambient_color  = texel_color * g_constants.light_ambient_factor;
    const float4 base_color     = texel_color * g_constants.light_color * g_constants.light_power;

    const float  diffuse_part   = clamp(dot(fragment_to_light, input.world_normal), 0.0, 1.0);
    const float4 diffuse_color  = base_color * diffuse_part;

    const float  specular_part  = pow(clamp(dot(fragment_to_eye, light_reflected_from_fragment), 0.0, 1.0), g_constants.light_specular_factor);
    const float4 specular_color = base_color * specular_part;
    const float  fading_ratio   = saturate(input.position.z * 8000.0F);

    return ColorLinearToSrgb((ambient_color + diffuse_color + specular_color) * fading_ratio);
}
