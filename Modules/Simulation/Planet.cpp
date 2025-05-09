/******************************************************************************

Copyright 2019-2020 Evgeny Gorodetskiy

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

FILE: Planet.cpp
Planet rendering primitive

******************************************************************************/

#include "Planet.h"

#include <Methane/Graphics/RHI/CommandListDebugGroup.h>
#include <Methane/Graphics/SphereMesh.hpp>
#include <Methane/Data/AppResourceProviders.h>
#include <Methane/Instrumentation.h>

namespace Methane::Samples
{

Planet::Planet(const rhi::CommandQueue& render_cmd_queue,
               const rhi::RenderPattern& render_pattern,
               const gfx::ImageLoader& image_loader,
               const Settings& settings)
    : Planet(render_cmd_queue, render_pattern, image_loader, settings, gfx::SphereMesh<Vertex>(Vertex::layout, 1.F, 32, 32))
{
    META_FUNCTION_TASK();
}

Planet::Planet(const rhi::CommandQueue& render_cmd_queue,
               const rhi::RenderPattern& render_pattern,
               const gfx::ImageLoader& image_loader,
               const Settings& settings,
               const gfx::BaseMesh<Vertex>& mesh)
    : m_settings(settings)
    , m_context(render_pattern.GetRenderContext())
    , m_render_cmd_queue(render_cmd_queue)
    , m_mesh_buffers(render_cmd_queue, mesh, "Planet")

{
    META_FUNCTION_TASK();
    rhi::Program render_program = m_context.CreateProgram(
        rhi::Program::Settings
        {
            .shader_set = rhi::Program::ShaderSet
            {
                { rhi::ShaderType::Vertex, { Data::ShaderProvider::Get(), { "Planet", "PlanetVS" }, { } } },
                { rhi::ShaderType::Pixel,  { Data::ShaderProvider::Get(), { "Planet", "PlanetPS" }, { } } }
            },
           .input_buffer_layouts = rhi::Program::InputBufferLayouts
            {
                rhi::Program::InputBufferLayout { mesh.GetVertexLayout().GetSemantics() }
            },
            .argument_accessors = rhi::Program::ArgumentAccessors
            {
                META_PROGRAM_ARG_ROOT_BUFFER_FRAME_CONSTANT(rhi::ShaderType::All, "g_uniforms")
            },
            .attachment_formats = render_pattern.GetAttachmentFormats()
        });
    render_program.SetName("Planet Shaders");

    m_render_state = m_context.CreateRenderState(
        rhi::RenderState::Settings
        {
            .program        = render_program,
            .render_pattern = render_pattern,
            .depth          = {
                .enabled = true,
                .compare  = m_settings.depth_reversed ? gfx::Compare::GreaterEqual : gfx::Compare::Less
            }
        });
    m_render_state.SetName("Planet Render State");
    
    m_mesh_buffers.SetTexture(image_loader.LoadImageToTexture2D(render_cmd_queue, m_settings.texture_path, m_settings.image_options, "Planet Texture"));

    m_texture_sampler = m_context.CreateSampler(
        rhi::SamplerSettings
        {
            .filter  = rhi::Sampler::Filter(rhi::Sampler::Filter::MinMag::Linear),
            .address = rhi::Sampler::Address(rhi::Sampler::Address::Mode::ClampToEdge),
            .lod     = rhi::Sampler::LevelOfDetail(m_settings.lod_bias)
        });
    m_texture_sampler.SetName("Planet Texture Sampler");
}

Planet::ProgramBindingsAndUniformArgumentBinding Planet::CreateProgramBindings(const rhi::Buffer& constants_buffer,
                                                                               Data::Index frame_index) const
{
    META_FUNCTION_TASK();
    rhi::ProgramBindings program_bindings = m_render_state.GetProgram().CreateBindings({
        { { rhi::ShaderType::Pixel, "g_constants" }, constants_buffer.GetResourceView()  },
        { { rhi::ShaderType::Pixel, "g_texture"   }, m_mesh_buffers.GetTexture().GetResourceView() },
        { { rhi::ShaderType::Pixel, "g_sampler"   }, m_texture_sampler.GetResourceView() },
    }, frame_index);
    rhi::IProgramArgumentBinding& uniforms_arg_binding = program_bindings.Get({ rhi::ShaderType::All, "g_uniforms" });
    return ProgramBindingsAndUniformArgumentBinding(std::move(program_bindings), &uniforms_arg_binding);
}

bool Planet::Update(double elapsed_seconds, double, rhi::IProgramArgumentBinding& uniforms_argument_binding)
{
    META_FUNCTION_TASK();
    const hlslpp::float4x4 model_scale_matrix     = hlslpp::float4x4::scale(m_settings.scale);
    const hlslpp::float4x4 model_translate_matrix = hlslpp::float4x4::translation(m_settings.position);
    const hlslpp::float4x4 model_rotation_matrix  = hlslpp::float4x4::rotation_y(static_cast<float>(-m_settings.spin_velocity_rps * elapsed_seconds));
    const hlslpp::float4x4 model_matrix           = hlslpp::mul(hlslpp::mul(model_scale_matrix, model_rotation_matrix), model_translate_matrix);

    hlslpp::PlanetUniforms uniforms{};
    uniforms.eye_position   = hlslpp::float4(m_settings.view_camera.GetOrientation().eye, 1.F);
    uniforms.light_position = m_settings.light_camera.GetOrientation().eye;
    uniforms.model_matrix   = hlslpp::transpose(model_matrix);
    uniforms.mvp_matrix     = hlslpp::transpose(hlslpp::mul(model_matrix, m_settings.view_camera.GetViewProjMatrix()));

    uniforms_argument_binding.SetRootConstant(rhi::RootConstant(uniforms));
    return true;
}

void Planet::Draw(const rhi::RenderCommandList& cmd_list,
                  const rhi::ProgramBindings& program_bindings,
                  const rhi::ViewState& view_state)
{
    META_FUNCTION_TASK();

    META_DEBUG_GROUP_VAR(s_debug_group, "Planet Rendering");
    cmd_list.ResetWithState(m_render_state, &s_debug_group);
    cmd_list.SetViewState(view_state);
    m_mesh_buffers.Draw(cmd_list, program_bindings);
}

} // namespace Methane::Graphics
