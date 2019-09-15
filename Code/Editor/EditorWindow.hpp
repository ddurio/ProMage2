#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Math/Vec2.hpp"


class EditorWindow {
    public:
    EditorWindow( const Vec2& normDimensions, const Vec2& alignment );
    virtual ~EditorWindow();

    void Update( float deltaSeconds );
    virtual void Render() const;

    virtual void UpdateChild( float deltaSeconds ) = 0;

    void SetLocation( const Vec2& normDimensions, const Vec2& alignment );


    protected:
    const char* m_windowName    = "EditorWindow";


    private:
    Vec2 m_windowDimensions     = Vec2::ZERO;
    Vec2 m_windowAlignment      = Vec2::ZERO;
};
