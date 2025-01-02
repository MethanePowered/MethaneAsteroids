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

FILE: Planet.h
Planet rendering primitive

******************************************************************************/

#pragma once

#include <Methane/Graphics/RHI/CommandQueue.h>
#include <Methane/Graphics/RHI/RenderContext.h>
#include <Methane/Graphics/RHI/RenderCommandList.h>
#include <Methane/Graphics/RHI/RenderState.h>
#include <Methane/Graphics/RHI/ViewState.h>
#include <Methane/Graphics/RHI/Buffer.h>
#include <Methane/Graphics/RHI/Program.h>
#include <Methane/Graphics/RHI/Sampler.h>
#include <Methane/Graphics/MeshBuffers.hpp>
#include <Methane/Graphics/Mesh.h>
#include <Methane/Graphics/Camera.h>
#include <Methane/Graphics/Types.h>
#include <Methane/Graphics/ImageLoader.h>

namespace hlslpp // NOSONAR
{
#pragma pack(push, 16)
#include "Shaders/PlanetUniforms.h" // NOSONAR
#pragma pack(pop)
}

#include <memory>

namespace Methane::Samples
{

namespace gfx = Graphics;
namespace rhi = Graphics::Rhi;

struct RenderPattern;

class Planet
{
public:
    struct Settings
    {
        const gfx::Camera&           view_camera;
        const gfx::Camera&           light_camera;
        std::string                  texture_path;
        hlslpp::float3               position;
        float                        scale;
        float                        spin_velocity_rps   = 0.3F; // (rps = radians per second)
        bool                         depth_reversed      = false;
        gfx::ImageLoader::OptionMask image_options       = {};
        float                        lod_bias            = 0.F;
    };

    using ProgramBindingsAndUniformArgumentBinding = std::pair<rhi::ProgramBindings, rhi::IProgramArgumentBinding*>;

    Planet(const rhi::CommandQueue& render_cmd_queue,
           const rhi::RenderPattern& render_pattern,
           const gfx::ImageLoader& image_loader,
           const Settings& settings);

    ProgramBindingsAndUniformArgumentBinding CreateProgramBindings(const rhi::Buffer& constants_buffer_ptr,
                                                                   Data::Index frame_index) const;
    bool Update(double elapsed_seconds, double delta_seconds,
                rhi::IProgramArgumentBinding& uniforms_argument_binding);
    void Draw(const rhi::RenderCommandList& cmd_list,
              const rhi::ProgramBindings& program_bindings,
              const rhi::ViewState& view_state);

private:
    using TexturedMeshBuffers = gfx::TexturedMeshBuffers<hlslpp::PlanetUniforms>;

    struct Vertex
    {
        gfx::Mesh::Position position;
        gfx::Mesh::Normal   normal;
        gfx::Mesh::TexCoord texcoord;

        inline static const gfx::Mesh::VertexLayout layout{
            gfx::Mesh::VertexField::Position,
            gfx::Mesh::VertexField::Normal,
            gfx::Mesh::VertexField::TexCoord,
        };
    };

    Planet(const rhi::CommandQueue& render_cmd_queue,
           const rhi::RenderPattern& render_pattern,
           const gfx::ImageLoader& image_loader,
           const Settings& settings,
           const gfx::BaseMesh<Vertex>& mesh);

    Settings            m_settings;
    rhi::RenderContext  m_context;
    rhi::CommandQueue   m_render_cmd_queue;
    TexturedMeshBuffers m_mesh_buffers;
    rhi::Sampler        m_texture_sampler;
    rhi::RenderState    m_render_state;
};

} // namespace Methane::Graphics
