#if defined(_EDITOR)
#include "Editor/XmlEditor.hpp"


XmlEditor::XmlEditor( const Vec2& normDimensions /*= Vec2( 0.2f, 1.f )*/, const Vec2& alignment /*= Vec( 0.8f, 0.f ) */ ) :
    EditorWindow( normDimensions, alignment ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "XmlEditor";
}


XmlEditor::~XmlEditor() {

}


void XmlEditor::UpdateChild( float deltaSeconds ) {

}

#endif
