#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/VertexUtils.hpp"

#include "Game/UIWidget.hpp"


class UILabel : public UIWidget {
    public:
    UILabel( const Vec2& alignment, const Vec2& offset, const Vec2& relativeSize, const Vec2& absoluteSize, const std::string& text );
    ~UILabel();

    void UpdateBounds( const AABB2& containerBounds ) override;
    void Render() const override;

    void SetText( const std::string& text );

    private:
    std::string m_text = "";
    VertexList m_textVerts;
};
