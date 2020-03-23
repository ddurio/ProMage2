#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "ThirdParty/DearImGui/imgui.h"
#include "ThirdParty/DearImGui/imgui_impl_dx11.h"
#include "ThirdParty/DearImGui/imgui_impl_win32.h"
#include "ThirdParty/DearImGui/imgui_stdlib.h"

#include "Engine/Renderer/RenderTypes.hpp"


class RenderContext;
class WindowContext;


class ImGuiSystem {
    friend class RenderContext;

    public:
    ImGuiSystem( RenderContext* renderer = nullptr, WindowContext* window = nullptr );

    void Startup( RenderContext* renderer = nullptr, WindowContext* window = nullptr );
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Render() const;

    Vec2 CreateGuiWindow( const Vec2& normDimensions, const Vec2& alignment, const char* name, ImGuiWindowFlags windowFlags, bool* windowIsOpen = nullptr ) const;
    Vec2 CreateStaticWindow( const Vec2& normDimensions, const Vec2& alignment, const char* name = "", ImGuiWindowFlags extraFlags = ImGuiWindowFlags_None, bool* windowIsOpen = nullptr ) const;

    bool IsSetup() const;


    private:
    bool m_isSetup = false;

    RenderContext* m_renderer = nullptr;
    ID3D11Device* m_d3dDevice = nullptr;
    ID3D11DeviceContext* m_d3dContext = nullptr;
    WindowContext* m_window = nullptr;


    ~ImGuiSystem();

    void Setup( RenderContext* renderer, WindowContext* window );
    void CheckSetup() const;
};
