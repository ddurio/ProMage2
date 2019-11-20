#if defined(_EDITOR)
#include "Editor/EditorWindow.hpp"

#include "Editor/Editor.hpp"


EditorWindow::EditorWindow( const Vec2& normDimensions, const Vec2& alignment ) {
    SetLocation( normDimensions, alignment );
}


EditorWindow::~EditorWindow() {

}


bool EditorWindow::Update( float deltaSeconds ) {
    Vec2 windowSize = g_theGui->CreateStaticWindow( m_windowDimensions, m_windowAlignment, m_windowName.c_str(), m_extraFlags, &m_windowIsOpen );
    bool regenTriggered = UpdateChild( deltaSeconds );
    ImGui::End();

    return regenTriggered;
}


void EditorWindow::Render() const {

}


void EditorWindow::SetLocation( const Vec2& normDimensions, const Vec2& alignment ) {
    m_windowDimensions = normDimensions;
    m_windowAlignment = alignment;
}

#endif
