#if defined(_EDITOR)
#include "Editor/EditorWindow.hpp"

#include "Editor/Editor.hpp"


EditorWindow::EditorWindow( const Vec2& normDimensions /*= Vec2( 0.8f, 0.9f )*/, const Vec2& alignment /*= Vec2( 0.f, 1.f ) */ ) {
    SetLocation( normDimensions, alignment );
}


EditorWindow::~EditorWindow() {

}


void EditorWindow::Update( float deltaSeconds ) {
    Vec2 windowSize = g_theEditor->CreateWindow( m_windowDimensions, m_windowAlignment, m_windowName );
    UpdateChild( deltaSeconds );
    ImGui::End();
}


void EditorWindow::Render() const {

}


void EditorWindow::SetLocation( const Vec2& normDimensions, const Vec2& alignment ) {
    m_windowDimensions = normDimensions;
    m_windowAlignment = alignment;
}

#endif
