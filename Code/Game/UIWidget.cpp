#include "Game/UIWidget.hpp"

#include "Engine/Core/DebugDraw.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Renderer/ColorTargetView.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Game.hpp"


UIWidget::UIWidget( const Vec2& alignment, const Vec2& offset, const Vec2& relativeSize, const Vec2& absoluteSize ) :
    m_alignment( alignment ),
    m_offset( offset ),
    m_relativeSize( relativeSize ),
    m_absoluteSize( absoluteSize ) {
}


UIWidget::~UIWidget() {
    CLEAR_POINTER( m_mesh );
    DestroyChildren();
}


void UIWidget::UpdateBounds( const AABB2& containerBounds ) {
    if( m_parent != nullptr ) {
        AABB2 parentBounds = m_parent->GetWorldBounds();

        if( parentBounds == containerBounds ) {
            return;
        }
    }

    Vec2 parentDimensions = containerBounds.GetDimensions();
    Vec2 myDimensions = parentDimensions * m_relativeSize + m_absoluteSize;

    AABB2 myBounds = AABB2( Vec2::ZERO, myDimensions );
    myBounds.AlignWithinBox( containerBounds, m_alignment );

    m_worldBounds = myBounds.GetTranslated( m_offset );
    m_position = m_worldBounds.GetCenter();

    CPUMesh builder;
    builder.SetColor( m_color );
    builder.AddQuad( OBB2(m_worldBounds) );

    CLEAR_POINTER( m_mesh );
    m_mesh = new GPUMesh( g_theRenderer );
    m_mesh->CopyVertsFromCPUMesh( &builder, "BuiltIn/Unlit" );

    UpdateChildrenBounds();
}


void UIWidget::Render() const {
    //g_theDebugger->DrawDebugQuad( m_position, m_worldBounds.GetDimensions(), 0.f, ALIGN_CENTER, m_color, m_color, m_texture );
    g_theRenderer->BindShader( "BuiltIn/Unlit" );
    g_theRenderer->BindTexture( m_texture );
    g_theRenderer->DrawMesh( m_mesh );

    RenderChildren();
}


bool UIWidget::HandleMouseButton( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    return HandleMouseButtonChildren( event, scrollAmount );
}


void UIWidget::AddChild( UIWidget* child ) {
    child->m_parent = this;
    m_children.push_back( child );
}


void UIWidget::RemoveChild( UIWidget* childToRemove ) {
    int numChildren = (int)m_children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        UIWidget* child = m_children[childIndex];

        if( child == childToRemove ) {
            CLEAR_POINTER( m_children[childIndex] );
            return;
        }
    }
}


Vec2 UIWidget::GetWorldPosition() const {
    return m_position;
}


AABB2 UIWidget::GetWorldBounds() const {
    return m_worldBounds;
}


Rgba UIWidget::GetColor() const {
    return m_color;
}


std::string UIWidget::GetTexture() const {
    return m_texture;
}


void UIWidget::SetColor( const Rgba& color ) {
    m_color = color;
}


void UIWidget::SetTexture( const std::string& texturePath ) {
    m_texture = texturePath;
}


void UIWidget::UpdateChildrenBounds() {
    int numChildren = (int)m_children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        UIWidget* child = m_children[childIndex];

        if( child != nullptr ) {
            child->UpdateBounds( m_worldBounds );
        }
    }
}


bool UIWidget::HandleMouseButtonChildren( MouseEvent event, float scrollAmount /*= 0.f */ ) {
    int numChildren = (int)m_children.size();

    for( int childIndex = numChildren - 1; childIndex >= 0; childIndex-- ) {
        UIWidget* child = m_children[childIndex];

        if( child != nullptr ) {
            bool handled = child->HandleMouseButton( event, scrollAmount );

            if( handled ) {
                return true;
            }
        }
    }

    return false;
}


void UIWidget::RenderChildren() const {
    int numChildren = (int)m_children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        UIWidget* child = m_children[childIndex];

        if( child != nullptr ) {
            child->Render();
        }
    }
}


void UIWidget::DestroyChildren() {
    int numChildren = (int)m_children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        CLEAR_POINTER( m_children[childIndex] ); // Handles nullptr
    }
}
