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

FILE: AsteroidsApp.cpp
Sample demonstrating parallel rendering of the distinct asteroids massive

******************************************************************************/

#include "AsteroidsApp.h"
#include "AsteroidsAppController.h"

#include <Methane/Graphics/AppCameraController.h>
#include <Methane/Data/TimeAnimation.h>
#include <Methane/Data/AppIconsProvider.h>
#include <Methane/Instrumentation.h>

#include <memory>
#include <thread>
#include <array>
#include <map>
#include <magic_enum.hpp>

namespace Methane::Samples
{

struct MutableParameters
{
    uint32_t instances_count;
    uint32_t unique_mesh_count;
    uint32_t textures_count;
    float    scale_ratio;
};

constexpr uint32_t g_max_complexity = 9;
static const std::array<MutableParameters, g_max_complexity+1> g_mutable_parameters{ {
    { 1000U,  35U,   10U, 0.6F  }, // 0
    { 2000U,  50U,   10U, 0.5F  }, // 1
    { 3000U,  75U,   20U, 0.45F }, // 2
    { 4000U,  100U,  20U, 0.4F  }, // 3
    { 5000U,  200U,  30U, 0.33F }, // 4
    { 10000U, 300U,  30U, 0.3F  }, // 5
    { 15000U, 400U,  40U, 0.27F }, // 6
    { 20000U, 500U,  40U, 0.23F }, // 7
    { 35000U, 750U,  50U, 0.2F  }, // 8
    { 50000U, 1000U, 50U, 0.17F }, // 9
} };

[[nodiscard]]
inline uint32_t GetDefaultComplexity()
{
#ifdef _DEBUG
    return 1U;
#else
    return std::thread::hardware_concurrency() / 2;
#endif
}

[[nodiscard]]
inline const MutableParameters& GetMutableParameters(uint32_t complexity)
{
    return g_mutable_parameters[std::min(complexity, g_max_complexity)];
}

[[nodiscard]]
inline const MutableParameters& GetMutableParameters()
{
    return GetMutableParameters(GetDefaultComplexity());
}

static const std::map<pin::Keyboard::State, AsteroidsAppAction> g_asteroids_action_by_keyboard_state{
    { { pin::Keyboard::Key::P            }, AsteroidsAppAction::SwitchParallelRendering     },
    { { pin::Keyboard::Key::L            }, AsteroidsAppAction::SwitchMeshLodsColoring      },
    { { pin::Keyboard::Key::Apostrophe   }, AsteroidsAppAction::IncreaseMeshLodComplexity   },
    { { pin::Keyboard::Key::Semicolon    }, AsteroidsAppAction::DecreaseMeshLodComplexity   },
    { { pin::Keyboard::Key::RightBracket }, AsteroidsAppAction::IncreaseComplexity          },
    { { pin::Keyboard::Key::LeftBracket  }, AsteroidsAppAction::DecreaseComplexity          },
    { { pin::Keyboard::Key::Num0         }, AsteroidsAppAction::SetComplexity0              },
    { { pin::Keyboard::Key::Num1         }, AsteroidsAppAction::SetComplexity1              },
    { { pin::Keyboard::Key::Num2         }, AsteroidsAppAction::SetComplexity2              },
    { { pin::Keyboard::Key::Num3         }, AsteroidsAppAction::SetComplexity3              },
    { { pin::Keyboard::Key::Num4         }, AsteroidsAppAction::SetComplexity4              },
    { { pin::Keyboard::Key::Num5         }, AsteroidsAppAction::SetComplexity5              },
    { { pin::Keyboard::Key::Num6         }, AsteroidsAppAction::SetComplexity6              },
    { { pin::Keyboard::Key::Num7         }, AsteroidsAppAction::SetComplexity7              },
    { { pin::Keyboard::Key::Num8         }, AsteroidsAppAction::SetComplexity8              },
    { { pin::Keyboard::Key::Num9         }, AsteroidsAppAction::SetComplexity9              },
};

void AsteroidsFrame::ReleaseScreenPassAttachmentTextures()
{
    META_FUNCTION_TASK();
    asteroids_pass.ReleaseAttachmentTextures();
    AppFrame::ReleaseScreenPassAttachmentTextures();
}

AsteroidsApp::AsteroidsApp()
    : UserInterfaceApp(
        Graphics::CombinedAppSettings
        {                                              // =========================
            Platform::IApp::Settings {                 // platform_app:
                "Methane Asteroids",                   //   - name
                { 0.8, 0.8 },                          //   - size
                { 640U, 480U },                        //   - min_size
                false,                                 //   - is_full_screen
                &Data::IconProvider::Get(),            //   - icon_resources_ptr
            },                                         // =========================
            Graphics::IApp::Settings {                 // graphics_app:
                rhi::RenderPassAccessMask{             //   - screen_pass_access
                    rhi::RenderPassAccess::ShaderResources,
                    rhi::RenderPassAccess::Samplers
                },
                true,                                  //   - animations_enabled
                true,                                  //   - show_hud_in_window_title
                0                                      //   - default_device_index
            },                                         // =========================
            rhi::RenderContext::Settings {             // render_context:
                gfx::FrameSize(),                      //   - frame_size
                gfx::PixelFormat::BGRA8Unorm,          //   - color_format
                gfx::PixelFormat::Depth32Float,        //   - depth_stencil_format
                std::nullopt,                          //   - clear_color
                gfx::DepthStencilValues(0.F, {}),      //   - clear_depth_stencil
                3U,                                    //   - frame_buffers_count
                false,                                 //   - vsync_enabled
                false,                                 //   - is_full_screen
                {},                                    //   - options_mask
                1000U,                                 //   - unsync_max_fps (MacOS only)
            }                                          // =========================
        },
        UserInterface::IApp::Settings
        {
            UserInterface::HeadsUpDisplayMode::UserInterface
        },
        "Methane Asteroids sample is demonstrating parallel rendering\nof massive asteroids field dynamic simulation.")
    , m_view_camera(GetAnimations(), gfx::ActionCamera::Pivot::Aim)
    , m_light_camera(m_view_camera, GetAnimations(), gfx::ActionCamera::Pivot::Aim)
    , m_asteroids_array_settings(                       // Asteroids array settings:
        {                                               // ================
            m_view_camera,                              // - view_camera
            m_scene_scale,                              // - scale
            GetMutableParameters().instances_count,     // - instance_count
            GetMutableParameters().unique_mesh_count,   // - unique_mesh_count
            4U,                                         // - subdivisions_count
            GetMutableParameters().textures_count,      // - textures_count
            { 256U, 256U },                             // - texture_dimensions
            1123U,                                      // - random_seed
            13.F,                                       // - orbit_radius_ratio
            4.F,                                        // - disc_radius_ratio
            0.06F,                                      // - mesh_lod_min_screen_size
            GetMutableParameters().scale_ratio / 10.F,  // - min_asteroid_scale_ratio
            GetMutableParameters().scale_ratio,         // - max_asteroid_scale_ratio
            true,                                       // - textures_array_enabled
            true                                        // - depth_reversed
        })
    , m_asteroids_complexity(GetDefaultComplexity())
{
    META_FUNCTION_TASK();

    // NOTE: Near and Far values are swapped in camera parameters (1st value is near = max depth, 2nd value is far = min depth)
    // for Reversed-Z buffer values range [ near: 1, far 0], instead of [ near 0, far 1]
    // which is used for "from near to far" drawing order for reducing pixels overdraw
    m_view_camera.ResetOrientation({ { -110.F, 75.F, 210.F }, { 0.F, -60.F, 25.F }, { 0.F, 1.F, 0.F } });
    m_view_camera.SetParameters({ 600.F /* near = max depth */, 0.01F /*far = min depth*/, 90.F /* FOV */ });
    m_view_camera.SetZoomDistanceRange({ 60.F , 400.F });

    m_light_camera.ResetOrientation({ { -100.F, 120.F, 0.F }, { 0.F, 0.F, 0.F }, { 0.F, 1.F, 0.F } });
    m_light_camera.SetProjection(gfx::Camera::Projection::Orthogonal);
    m_light_camera.SetParameters({ -300.F, 300.F, 90.F });
    m_light_camera.Resize(Data::FloatSize(120.F, 120.F));

    AddInputControllers({
        std::make_shared<AsteroidsAppController>(*this, g_asteroids_action_by_keyboard_state),
        std::make_shared<gfx::AppCameraController>(m_view_camera,  "VIEW CAMERA"),
        std::make_shared<gfx::AppCameraController>(m_light_camera, "LIGHT SOURCE",
            gfx::AppCameraController::ActionByMouseButton   { { pin::Mouse::Button::Right, gfx::ActionCamera::MouseAction::Rotate   } },
            gfx::AppCameraController::ActionByKeyboardState { { { pin::Keyboard::Key::LeftControl, pin::Keyboard::Key::L }, gfx::ActionCamera::KeyboardAction::Reset } },
            gfx::AppCameraController::ActionByKeyboardKey   { }),
    });

    const std::string options_group = "Asteroids Options";
    add_option_group(options_group);
    add_option("-c,--complexity",
               [this](const CLI::results_t& res)
               {
                   if (uint32_t complexity = 0;
                       CLI::detail::lexical_cast(res[0], complexity))
                   {
                       SetAsteroidsComplexity(complexity);
                       return true;
                   }
                   return false;
               }, "simulation complexity")
        ->default_val(m_asteroids_complexity)
        ->expected(0, static_cast<int>(g_max_complexity))
        ->group(options_group);
    add_option("-s,--subdiv-count", m_asteroids_array_settings.subdivisions_count, "mesh subdivisions count")->group(options_group);
    add_option("-t,--texture-array", m_asteroids_array_settings.textures_array_enabled, "texture array enabled")->group(options_group);
    add_option("-r,--parallel-render", m_is_parallel_rendering_enabled, "parallel rendering enabled")->group(options_group);

    // Setup animations
    GetAnimations().push_back(std::make_shared<Data::TimeAnimation>(std::bind(&AsteroidsApp::Animate, this, std::placeholders::_1, std::placeholders::_2)));

    // Enable dry updates on pause to keep asteroids in sync with projection matrix dependent on window size which may change
    GetAnimations().SetDryUpdateOnPauseEnabled(true);

    ShowParameters();
}

AsteroidsApp::~AsteroidsApp()
{
    META_FUNCTION_TASK();
    // Wait for GPU rendering is completed to release resources
    WaitForRenderComplete();
}

void AsteroidsApp::Init()
{
    META_FUNCTION_TASK();
    META_SCOPE_TIMER("AsteroidsApp::Init");

    // Create initial render-pass pattern for asteroids rendering
    rhi::RenderPattern::Settings asteroids_render_pattern_settings     = GetScreenRenderPatternSettings();
    asteroids_render_pattern_settings.color_attachments[0].load_action = rhi::RenderPass::Attachment::LoadAction::DontCare;
    asteroids_render_pattern_settings.depth_attachment->load_action    = rhi::RenderPass::Attachment::LoadAction::Clear;
    asteroids_render_pattern_settings.depth_attachment->store_action   = rhi::RenderPass::Attachment::StoreAction::Store;
    asteroids_render_pattern_settings.is_final_pass = false;
    m_asteroids_render_pattern = rhi::RenderPattern(GetRenderContext(), asteroids_render_pattern_settings);

    // Modify settings of the final screen render-pass pattern so that color and depth attachments are reused from initial asteroids render pass
    rhi::RenderPattern::Settings& screen_render_pattern_settings    = GetScreenRenderPatternSettings();
    screen_render_pattern_settings.color_attachments[0].load_action = rhi::RenderPass::Attachment::LoadAction::Load;
    screen_render_pattern_settings.depth_attachment->load_action    = rhi::RenderPass::Attachment::LoadAction::Load;

    // Screen render pattern and screen passes for all frames are initialized here based on modified settings
    UserInterfaceApp::Init();

    const rhi::RenderContext& context = GetRenderContext();
    rhi::CommandQueue render_cmd_queue = context.GetRenderCommandKit().GetQueue();
    const rhi::RenderContext::Settings& context_settings = context.GetSettings();
    m_view_camera.Resize(context_settings.frame_size);

    // Load cube-map texture images for Sky-box
    const rhi::Texture sky_box_texture = GetImageLoader().LoadImagesToTextureCube(render_cmd_queue,
        gfx::ImageLoader::CubeFaceResources
        {
            "Galaxy/PositiveX.jpg",
            "Galaxy/NegativeX.jpg",
            "Galaxy/PositiveY.jpg",
            "Galaxy/NegativeY.jpg",
            "Galaxy/PositiveZ.jpg",
            "Galaxy/NegativeZ.jpg"
        },
        { gfx::ImageOption::Mipmapped },
        "Sky-Box Texture"
    );

    // Create sky-box
    m_sky_box = gfx::SkyBox(render_cmd_queue, m_asteroids_render_pattern, sky_box_texture,
        gfx::SkyBox::Settings
        {
            m_view_camera,
            m_scene_scale * 100.F,
            { gfx::SkyBox::Option::DepthEnabled, gfx::SkyBox::Option::DepthReversed }
        });

    // Create planet
    m_planet_ptr = std::make_shared<Planet>(render_cmd_queue, m_asteroids_render_pattern, GetImageLoader(),
        Planet::Settings
        {
            m_view_camera,
            m_light_camera,
            "Planet/Mars.jpg",                      // texture_path
            hlslpp::float3(0.F, 0.F, 0.F),          // position
            m_scene_scale * 3.F,                    // scale
            0.1F,                                   // spin_velocity_rps
            true,                                   // depth_reversed
            { gfx::ImageOption::Mipmapped,          // image_options
              gfx::ImageOption::SrgbColorSpace },   //
            -1.F,                                   // lod_bias
        }
    );

    // Create asteroids array
    m_asteroids_array_ptr = m_asteroids_array_state_ptr
                          ? std::make_unique<AsteroidsArray>(render_cmd_queue, m_asteroids_render_pattern, m_asteroids_array_settings, *m_asteroids_array_state_ptr)
                          : std::make_unique<AsteroidsArray>(render_cmd_queue, m_asteroids_render_pattern, m_asteroids_array_settings);

    const auto       constants_data_size         = static_cast<Data::Size>(sizeof(hlslpp::SceneConstants));
    const auto       scene_uniforms_data_size    = static_cast<Data::Size>(sizeof(hlslpp::SceneUniforms));
    const Data::Size asteroid_uniforms_data_size = m_asteroids_array_ptr->GetUniformsBufferSize();

    // Create constants buffer for frame rendering
    m_const_buffer = context.CreateBuffer(rhi::BufferSettings::ForConstantBuffer(constants_data_size));
    m_const_buffer.SetName("Constants Buffer");
    m_const_buffer.SetData(render_cmd_queue, {
        reinterpret_cast<Data::ConstRawPtr>(&m_scene_constants),
        sizeof(m_scene_constants)
    });

    // ========= Per-Frame Data =========
    for(AsteroidsFrame& frame : GetFrames())
    {
        // Create asteroids render pass
        frame.asteroids_pass = rhi::RenderPass(m_asteroids_render_pattern, frame.screen_pass.GetSettings());

        // Create parallel command list for asteroids rendering
        frame.parallel_cmd_list = rhi::ParallelRenderCommandList(context.GetRenderCommandKit().GetQueue(), frame.asteroids_pass);
        frame.parallel_cmd_list.SetParallelCommandListsCount(std::thread::hardware_concurrency());
        frame.parallel_cmd_list.SetName(IndexedName("Parallel Rendering", frame.index));
        frame.parallel_cmd_list.SetValidationEnabled(false);

        // Create serial command list for asteroids rendering
        frame.serial_cmd_list = rhi::RenderCommandList(context.GetRenderCommandKit().GetQueue(), frame.asteroids_pass);
        frame.serial_cmd_list.SetName(IndexedName("Serial Rendering", frame.index));
        frame.serial_cmd_list.SetValidationEnabled(false);

        // Create final command list for sky-box and planet rendering
        frame.final_cmd_list = rhi::RenderCommandList(context.GetRenderCommandKit().GetQueue(), frame.screen_pass);
        frame.final_cmd_list.SetName(IndexedName("Final Rendering", frame.index));
        frame.final_cmd_list.SetValidationEnabled(false);

        // Rendering command lists sequence
        frame.execute_cmd_list_set = CreateExecuteCommandListSet(frame);

        // Create uniforms buffer with volatile parameters for the whole scene rendering
        frame.scene_uniforms_buffer = context.CreateBuffer(rhi::BufferSettings::ForConstantBuffer(scene_uniforms_data_size, false, true));
        frame.scene_uniforms_buffer.SetName(IndexedName("Scene Uniforms Buffer", frame.index));

        // Create uniforms buffer for Sky-Box rendering
        frame.sky_box.uniforms_buffer = context.CreateBuffer(rhi::BufferSettings::ForConstantBuffer(gfx::SkyBox::GetUniformsSize(), false, true));
        frame.sky_box.uniforms_buffer.SetName(IndexedName("Sky-box Uniforms Buffer", frame.index));

        // Create uniforms buffer for Planet rendering
        frame.planet.uniforms_buffer = context.CreateBuffer(rhi::BufferSettings::ForConstantBuffer(sizeof(hlslpp::PlanetUniforms), false, true));
        frame.planet.uniforms_buffer.SetName(IndexedName("Planet Uniforms Buffer", frame.index));

        // Create uniforms buffer for Asteroids array rendering
        frame.asteroids.uniforms_buffer = context.CreateBuffer(rhi::BufferSettings::ForConstantBuffer(asteroid_uniforms_data_size, true, true));
        frame.asteroids.uniforms_buffer.SetName(IndexedName("Asteroids Array Uniforms Buffer", frame.index));

        // Resource bindings for Sky-Box rendering
        frame.sky_box.program_bindings = m_sky_box.CreateProgramBindings(frame.sky_box.uniforms_buffer, frame.index);
        frame.sky_box.program_bindings.SetName(IndexedName("Space Sky-Box Bindings {}", frame.index));

        // Resource bindings for Planet rendering
        frame.planet.program_bindings = m_planet_ptr->CreateProgramBindings(m_const_buffer, frame.planet.uniforms_buffer, frame.index);
        frame.planet.program_bindings.SetName(IndexedName("Planet Bindings {}", frame.index));

        // Resource bindings for Asteroids rendering
        frame.asteroids.program_bindings_per_instance = m_asteroids_array_ptr->CreateProgramBindings(m_const_buffer,
                                                                                                     frame.scene_uniforms_buffer,
                                                                                                     frame.asteroids.uniforms_buffer,
                                                                                                     frame.index);
    }

    // Update initial resource states before asteroids drawing without applying barriers on GPU (automatic state propagation from Common state works),
    // which is required for correct automatic resource barriers to be set after asteroids drawing, on planet drawing
    m_asteroids_array_ptr->CreateBeginningResourceBarriers(&m_const_buffer).ApplyTransitions();

    CompleteInitialization();
    META_LOG(GetParametersString());
}

bool AsteroidsApp::Resize(const gfx::FrameSize& frame_size, bool is_minimized)
{
    META_FUNCTION_TASK();

    // Resize screen color and depth textures
    if (!UserInterfaceApp::Resize(frame_size, is_minimized))
        return false;
    
    // Update frame buffer and depth textures in initial & final render passes
    for (const AsteroidsFrame& frame : GetFrames())
    {
        rhi::RenderPassSettings asteroids_pass_settings{
            {
                rhi::TextureView(frame.screen_texture.GetInterface()),
                rhi::TextureView(GetDepthTexture().GetInterface())
            },
            frame_size
        };
        frame.asteroids_pass.Update(asteroids_pass_settings);
    }

    m_view_camera.Resize(frame_size);

    return true;
}

bool AsteroidsApp::Update()
{
    META_FUNCTION_TASK();
    META_SCOPE_TIMER("AsteroidsApp::Update");

    if (!UserInterfaceApp::Update())
        return false;

    // Update scene uniforms
    m_scene_uniforms.view_proj_matrix = hlslpp::transpose(m_view_camera.GetViewProjMatrix());
    m_scene_uniforms.eye_position     = m_view_camera.GetOrientation().eye;
    m_scene_uniforms.light_position   = m_light_camera.GetOrientation().eye;

    m_sky_box.Update();
    return true;
}

bool AsteroidsApp::Animate(double elapsed_seconds, double delta_seconds) const
{
    META_FUNCTION_TASK();
    bool update_result = m_planet_ptr->Update(elapsed_seconds, delta_seconds);
    update_result     |= m_asteroids_array_ptr->Update(elapsed_seconds, delta_seconds);
    return update_result;
}

bool AsteroidsApp::Render()
{
    META_FUNCTION_TASK();
    META_SCOPE_TIMER("AsteroidsApp::Render");
    if (!UserInterfaceApp::Render())
        return false;

    // Upload uniform buffers to GPU
    const AsteroidsFrame& frame = GetCurrentFrame();
    rhi::CommandQueue render_cmd_queue = GetRenderContext().GetRenderCommandKit().GetQueue();
    frame.scene_uniforms_buffer.SetData(render_cmd_queue, m_scene_uniforms_subresource);

    // Asteroids rendering in parallel or in main thread
    if (m_is_parallel_rendering_enabled)
    {
        GetAsteroidsArray().DrawParallel(frame.parallel_cmd_list, frame.asteroids, GetViewState());
        frame.parallel_cmd_list.Commit();
    }
    else
    {
        GetAsteroidsArray().Draw(frame.serial_cmd_list, frame.asteroids, GetViewState());
        frame.serial_cmd_list.Commit();
    }
    
    // Draw planet and sky-box after asteroids to minimize pixel overdraw
    m_planet_ptr->Draw(frame.final_cmd_list, frame.planet, GetViewState());
    m_sky_box.Draw(frame.final_cmd_list, frame.sky_box, GetViewState());

    RenderOverlay(frame.final_cmd_list);
    frame.final_cmd_list.Commit();

    // Execute rendering commands and present frame to screen
    render_cmd_queue.Execute(frame.execute_cmd_list_set);
    GetRenderContext().Present();

    return true;
}

void AsteroidsApp::OnContextReleased(rhi::IContext& context)
{
    META_FUNCTION_TASK();
    META_SCOPE_TIMERS_FLUSH();

    if (m_asteroids_array_ptr)
    {
        m_asteroids_array_state_ptr = m_asteroids_array_ptr->GetState();
    }

    m_planet_ptr.reset();
    m_asteroids_array_ptr.reset();
    m_sky_box = {};
    m_const_buffer = {};
    m_asteroids_render_pattern = {};

    UserInterfaceApp::OnContextReleased(context);
}

void AsteroidsApp::SetAsteroidsComplexity(uint32_t asteroids_complexity)
{
    META_FUNCTION_TASK();

    asteroids_complexity = std::min(g_max_complexity, asteroids_complexity);
    if (m_asteroids_complexity == asteroids_complexity)
        return;

    if (GetRenderContext().IsInitialized())
    {
        WaitForRenderComplete();
    }

    m_asteroids_complexity = asteroids_complexity;

    const MutableParameters& mutable_parameters         = GetMutableParameters(m_asteroids_complexity);
    m_asteroids_array_settings.instance_count           = mutable_parameters.instances_count;
    m_asteroids_array_settings.unique_mesh_count        = mutable_parameters.unique_mesh_count;
    m_asteroids_array_settings.textures_count           = mutable_parameters.textures_count;
    m_asteroids_array_settings.min_asteroid_scale_ratio = mutable_parameters.scale_ratio / 10.F;
    m_asteroids_array_settings.max_asteroid_scale_ratio = mutable_parameters.scale_ratio;

    m_asteroids_array_ptr.reset();
    m_asteroids_array_state_ptr.reset();

    if (GetRenderContext().IsInitialized())
    {
        GetRenderContext().Reset();
    }

    UpdateParametersText();
}

void AsteroidsApp::SetParallelRenderingEnabled(bool is_parallel_rendering_enabled)
{
    META_FUNCTION_TASK();
    if (m_is_parallel_rendering_enabled == is_parallel_rendering_enabled)
        return;

    META_SCOPE_TIMERS_FLUSH();
    m_is_parallel_rendering_enabled = is_parallel_rendering_enabled;
    for(AsteroidsFrame& frame : GetFrames())
    {
        frame.execute_cmd_list_set = CreateExecuteCommandListSet(frame);
    }

    UpdateParametersText();
    META_LOG(GetParametersString());
}

AsteroidsArray& AsteroidsApp::GetAsteroidsArray() const
{
    META_FUNCTION_TASK();
    META_CHECK_ARG_NOT_NULL(m_asteroids_array_ptr);
    return *m_asteroids_array_ptr;
}

std::string AsteroidsApp::GetParametersString()
{
    META_FUNCTION_TASK();

    std::stringstream ss;
    ss << "Asteroids simulation parameters:"
       << std::endl << "  - simulation complexity [0.."  << g_max_complexity << "]: " << m_asteroids_complexity
       << std::endl << "  - asteroid instances count:     " << m_asteroids_array_settings.instance_count
       << std::endl << "  - unique meshes count:          " << m_asteroids_array_settings.unique_mesh_count
       << std::endl << "  - mesh subdivisions count:      " << m_asteroids_array_settings.subdivisions_count
       << std::endl << "  - unique textures count:        " << m_asteroids_array_settings.textures_count << " "
       << std::endl << "  - asteroid textures size:       " << static_cast<std::string>(m_asteroids_array_settings.texture_dimensions)
       << std::endl << "  - textures array binding:       " << (m_asteroids_array_settings.textures_array_enabled ? "ON" : "OFF")
       << std::endl << "  - parallel rendering:           " << (m_is_parallel_rendering_enabled ? "ON" : "OFF")
       << std::endl << "  - asteroid animations:          " << (!GetAnimations().IsPaused() ? "ON" : "OFF")
       << std::endl << "  - CPU h/w thread count:         " << std::thread::hardware_concurrency();

    return ss.str();
}

rhi::CommandListSet AsteroidsApp::CreateExecuteCommandListSet(const AsteroidsFrame& frame) const
{
    return rhi::CommandListSet({
        m_is_parallel_rendering_enabled
            ? static_cast<rhi::ICommandList&>(frame.parallel_cmd_list.GetInterface())
            : static_cast<rhi::ICommandList&>(frame.serial_cmd_list.GetInterface()),
        frame.final_cmd_list.GetInterface()
    }, frame.index);
}

} // namespace Methane::Samples

int main(int argc, const char* argv[])
{
    return Methane::Samples::AsteroidsApp().Run({ argc, argv });
}
