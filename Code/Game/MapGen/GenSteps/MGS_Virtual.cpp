#include "Game/MapGen/GenSteps/MGS_Virtual.hpp"

#include "Game/MapGen/GenSteps/MGS_VirtualDef.hpp"


MGS_Virtual::MGS_Virtual( const XMLElement& element, const Strings& motifHierarchy ) :
    MapGenStep( element, motifHierarchy ) {
    // Name
    m_defType      = ParseXMLAttribute( element, "name", m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(MGS_VirtualDef) Missing required attribute 'name'" );
    const MGS_VirtualDef* stepDef = MGS_VirtualDef::GetDefinition( m_defType );

    SetupMotifFromXML( element, stepDef );
    stepDef->DefineObject( *this );
}


void MGS_Virtual::SaveToXml( XmlDocument& document, XMLElement& element ) const {

}


bool MGS_Virtual::RecalculateMotifVars( EventArgs& args ) {
    int numSteps = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* genStep = m_genSteps[stepIndex];
        genStep->RecalculateMotifVars( args );
    }

    return false;
}


// PRIVATE -----------------------------------------
int MGS_Virtual::s_numVirtualSteps = 0;


void MGS_Virtual::RunOnce( Map& theMap ) const {
    int numSteps = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* genStep = m_genSteps[stepIndex];
        genStep->Run( theMap );
    }
}


void MGS_Virtual::SetupMotifFromXML( const XMLElement& element, const MGS_VirtualDef* virtualDef ) {
    // Make new motif based on VirtualDef motif
    std::string motifToCopy = virtualDef->GetMotif();
    std::string nameToAppend = Stringf( "VirtualXML.%d", s_numVirtualSteps );
    MotifDef* newMotif = new MotifDef( motifToCopy, nameToAppend );

    // Modify new motif based on XML overrides
    NamedProperties& motifVars = newMotif->GetVariables();
    Strings varNames = motifVars.GetNames();

    std::string invalidValue = "__HOPEFULLY_NOT_A_VALUE__";
    int numVars = (int)varNames.size();

    for( int varIndex = 0; varIndex < numVars; varIndex++ ) {
        const std::string& varName = varNames[varIndex];
        std::string varValue = ParseXMLAttribute( element, varName.c_str(), invalidValue );

        if( varValue != invalidValue ) {
            motifVars.SetValue( varName, varValue );
        }
    }

    // Add motif to my hierarchy
    AddChildMotifs( { newMotif->GetDefintionType() } );
}
