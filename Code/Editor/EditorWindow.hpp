#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Math/Vec2.hpp"


class EditorWindow {
    public:
    EditorWindow( const Vec2& normDimensions, const Vec2& alignment );
    virtual ~EditorWindow();

    bool Update( float deltaSeconds );
    virtual void Render() const;

    virtual bool UpdateChild( float deltaSeconds ) = 0;

    void SetLocation( const Vec2& normDimensions, const Vec2& alignment );


    protected:
    std::string m_windowName    = "EditorWindow";
    Vec2 m_windowDimensions     = Vec2::ZERO;
    Vec2 m_windowAlignment      = Vec2::ZERO;

    ImGuiWindowFlags m_extraFlags = ImGuiWindowFlags_HorizontalScrollbar;
};
