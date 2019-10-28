#include "Game/MapGen/GenSteps/MGS_Custom.hpp"

#include "Game/MapGen/GenSteps/MGS_CustomDef.hpp"


MGS_Custom::MGS_Custom( const XMLElement& element, const Strings& motifHierarchy ) :
    MapGenStep( element, motifHierarchy ) {
    // Name
    m_defType = element.Name();
    const MGS_CustomDef* stepDef = MGS_CustomDef::GetDefinition( m_defType );

    SetupMotifFromXML( element, stepDef );
    stepDef->DefineObject( *this );
}


void MGS_Custom::SaveToXml( XmlDocument& document, XMLElement& element ) const {

}


bool MGS_Custom::RecalculateMotifVars( EventArgs& args ) {
    int numSteps = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* genStep = m_genSteps[stepIndex];
        genStep->RecalculateMotifVars( args );
    }

    return false;
}


// PRIVATE -----------------------------------------
int MGS_Custom::s_numCustomSteps = 0;


void MGS_Custom::RunOnce( Map& theMap ) const {
    int numSteps = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* genStep = m_genSteps[stepIndex];
        genStep->Run( theMap );
    }
}


void MGS_Custom::SetupMotifFromXML( const XMLElement& element, const MGS_CustomDef* virtualDef ) {
    // Make new motif based on CustomDef motif
    std::string motifToCopy = virtualDef->GetMotif();
    std::string nameToAppend = Stringf( "CustomXML.%d", s_numCustomSteps );
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
        } else {
            motifVars.ClearValue( varName );
        }
    }

    // Add motif to my hierarchy
    AddChildMotifs( { newMotif->GetDefintionType() } );
}
