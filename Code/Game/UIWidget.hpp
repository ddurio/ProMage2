#pragma once
#include "Game/GameCommon.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"


class GPUMesh;
//class UIRadioGroup;

struct AABB2;

enum MouseEvent : int;


class UIWidget {
    public:
    UIWidget( const Vec2& alignment, const Vec2& offset, const Vec2& relativeSize, const Vec2& absoluteSize );
    virtual ~UIWidget();

    virtual void UpdateBounds( const AABB2& containerBounds );
    virtual void Render() const;

    virtual bool HandleMouseButton( MouseEvent event, float scrollAmount = 0.f );

    void AddChild( UIWidget* child );
    void RemoveChild( UIWidget* child );

    //void SetRadioGroup( UIRadioGroup* group ); // TODO: Implement RadioGroup

    Vec2 GetWorldPosition() const;
    AABB2 GetWorldBounds() const;
    Rgba GetColor() const;
    std::string GetTexture() const;

    void SetColor( const Rgba& color );
    void SetTexture( const std::string& texturePath );

    protected:
    void UpdateChildrenBounds();
    bool HandleMouseButtonChildren( MouseEvent event, float scrollAmount = 0.f );
    void RenderChildren() const;
    void DestroyChildren();

    private:
    UIWidget* m_parent = nullptr;
    std::vector<UIWidget*> m_children;

    Vec2 m_alignment = ALIGN_CENTER;
    Vec2 m_offset = Vec2::ZERO;
    Vec2 m_relativeSize = Vec2::ONE;
    Vec2 m_absoluteSize = Vec2::ZERO;

    Vec2 m_position = Vec2::ZERO;
    AABB2 m_worldBounds = AABB2();

    Rgba m_color = Rgba::WHITE;
    std::string m_texture = "";
    GPUMesh* m_mesh = nullptr;

    //UIRadioGroup* m_radioGroup = nullptr;
};
