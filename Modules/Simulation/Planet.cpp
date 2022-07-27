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

#include <Methane/Graphics/ImageLoader.h>
#include <Methane/Graphics/SphereMesh.hpp>
#include <Methane/Graphics/Buffer.h>
#include <Methane/Graphics/RenderPass.h>
#include <Methane/Data/AppResourceProviders.h>
#include <Methane/Instrumentation.h>

namespace Methane::Samples
{

Planet::Planet(gfx::CommandQueue& render_cmd_queue, gfx::RenderPattern& render_pattern, const gfx::ImageLoader& image_loader, const Settings& settings)
    : Planet(render_cmd_queue, render_pattern, image_loader, settings, gfx::SphereMesh<Vertex>(Vertex::layout, 1.F, 32, 32))
{
    META_FUNCTION_TASK();
}

Planet::Planet(gfx::CommandQueue& render_cmd_queue, gfx::RenderPattern& render_pattern, const gfx::ImageLoader& image_loader, const Settings& settings, const gfx::BaseMesh<Vertex>& mesh)
    : m_settings(settings)
    , m_context(render_pattern.GetRenderContext())
    , m_render_cmd_queue_ptr(std::dynamic_pointer_cast<gfx::CommandQueue>(render_cmd_queue.GetPtr()))
    , m_mesh_buffers(render_cmd_queue, mesh, "Planet")

{
    META_FUNCTION_TASK();

    gfx::RenderState::Settings state_settings;
    state_settings.program_ptr = gfx::Program::Create(m_context,
        gfx::Program::Settings
        {
            gfx::Program::Shaders
            {
                gfx::Shader::CreateVertex(m_context, { Data::ShaderProvider::Get(), { "Planet", "PlanetVS" }, { } }),
                gfx::Shader::CreatePixel( m_context, { Data::ShaderProvider::Get(), { "Planet", "PlanetPS" }, { } }),
            },
            gfx::Program::InputBufferLayouts
            {
                gfx::Program::InputBufferLayout { mesh.GetVertexLayout().GetSemantics() }
            },
            gfx::Program::ArgumentAccessors
            {
                { { gfx::Shader::Type::All,    "g_uniforms"  }, gfx::Program::ArgumentAccessor::Type::FrameConstant  },
                { { gfx::Shader::Type::Pixel,  "g_constants" }, gfx::Program::ArgumentAccessor::Type::Constant       },
                { { gfx::Shader::Type::Pixel,  "g_texture"   }, gfx::Program::ArgumentAccessor::Type::Constant       },
                { { gfx::Shader::Type::Pixel,  "g_sampler"   }, gfx::Program::ArgumentAccessor::Type::Constant       },
            },
            render_pattern.GetAttachmentFormats()
        }
    );
    state_settings.program_ptr->SetName("Planet Shaders");
    state_settings.render_pattern_ptr = std::dynamic_pointer_cast<gfx::RenderPattern>(render_pattern.GetPtr());
    state_settings.depth.enabled = true;
    state_settings.depth.compare = m_settings.depth_reversed ? gfx::Compare::GreaterEqual : gfx::Compare::Less;

    m_render_state_ptr = gfx::RenderState::Create(m_context, state_settings);
    m_render_state_ptr->SetName("Planet Render State");
    
    m_mesh_buffers.SetTexture(image_loader.LoadImageToTexture2D(render_cmd_queue, m_settings.texture_path, m_settings.image_options, "Planet Texture"));

    m_texture_sampler_ptr = gfx::Sampler::Create(m_context, {
        gfx::Sampler::Filter(gfx::Sampler::Filter::MinMag::Linear),
        gfx::Sampler::Address(gfx::Sampler::Address::Mode::ClampToEdge),
        gfx::Sampler::LevelOfDetail(m_settings.lod_bias)
    });
    m_texture_sampler_ptr->SetName("Planet Texture Sampler");

    // Initialize default uniforms to be ready to render right away
    Update(0.0, 0.0);
}

Ptr<gfx::ProgramBindings> Planet::CreateProgramBindings(const Ptr<gfx::Buffer>& constants_buffer_ptr, const Ptr<gfx::Buffer>& uniforms_buffer_ptr, Data::Index frame_index) const
{
    META_FUNCTION_TASK();
    META_CHECK_ARG_NOT_NULL(m_render_state_ptr);
    META_CHECK_ARG_NOT_NULL(m_render_state_ptr->GetSettings().program_ptr);

    return gfx::ProgramBindings::Create(m_render_state_ptr->GetSettings().program_ptr, {
        { { gfx::Shader::Type::All,   "g_uniforms"  }, { { *uniforms_buffer_ptr            } } },
        { { gfx::Shader::Type::Pixel, "g_constants" }, { { *constants_buffer_ptr           } } },
        { { gfx::Shader::Type::Pixel, "g_texture"   }, { { m_mesh_buffers.GetTexture() } } },
        { { gfx::Shader::Type::Pixel, "g_sampler"   }, { { *m_texture_sampler_ptr          } } },
    }, frame_index);
}

bool Planet::Update(double elapsed_seconds, double)
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

    m_mesh_buffers.SetFinalPassUniforms(std::move(uniforms));
    return true;
}

void Planet::Draw(gfx::RenderCommandList& cmd_list, const gfx::MeshBufferBindings& buffer_bindings, gfx::ViewState& view_state)
{
    META_FUNCTION_TASK();
    META_CHECK_ARG_NOT_NULL(buffer_bindings.uniforms_buffer_ptr);
    META_CHECK_ARG_GREATER_OR_EQUAL(buffer_bindings.uniforms_buffer_ptr->GetDataSize(), sizeof(hlslpp::PlanetUniforms));
    META_CHECK_ARG_NOT_NULL(buffer_bindings.program_bindings_ptr);

    buffer_bindings.uniforms_buffer_ptr->SetData(m_mesh_buffers.GetFinalPassUniformsSubresources(), *m_render_cmd_queue_ptr);

    META_DEBUG_GROUP_CREATE_VAR(s_debug_group, "Planet Rendering");
    cmd_list.ResetWithState(*m_render_state_ptr, s_debug_group.get());
    cmd_list.SetViewState(view_state);
    m_mesh_buffers.Draw(cmd_list, *buffer_bindings.program_bindings_ptr);
}

} // namespace Methane::Graphics
