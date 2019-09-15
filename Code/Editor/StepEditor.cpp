#if defined(_EDITOR)
#include "Editor/StepEditor.hpp"


StepEditor::StepEditor( const Vec2& normDimensions /*= Vec2( 0.8f, 0.1f )*/, const Vec2& alignment /*= Vec2::ZERO */ ) :
    EditorWindow( normDimensions, alignment ) {
    // ThesisFIXME: Choose better name for this window
    m_windowName = "StepEditor";
}


StepEditor::~StepEditor() {

}


void StepEditor::UpdateChild( float deltaSeconds ) {

}

#endif
