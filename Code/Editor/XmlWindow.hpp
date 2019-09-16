#pragma once
#include "Editor/EditorCommon.hpp"

#include "Editor/EditorWindow.hpp"

#include "Engine/Math/Vec2.hpp"


class XmlWindow : public EditorWindow {
    public:
    XmlWindow( const Vec2& normDimensions = Vec2( 0.2f, 1.f ), const Vec2& alignment = Vec2::ONE );
    ~XmlWindow();


    private:
    void UpdateChild( float deltaSeconds ) override;
};
