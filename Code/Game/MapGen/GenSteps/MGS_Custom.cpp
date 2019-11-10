#include "Game/MapGen/GenSteps/MGS_Custom.hpp"

#include "Game/MapGen/GenSteps/MGS_CustomDef.hpp"


MGS_Custom::MGS_Custom( const XMLElement& element, const Strings& motifHierarchy ) :
    MapGenStep( element, motifHierarchy ) {
    // Name
    m_defType = element.Name();
    const MGS_CustomDef* stepDef = MGS_CustomDef::GetDefinition( m_defType );

    SetupMotifFromXML( element );
    stepDef->DefineObject( *this );
}


MGS_Custom::MGS_Custom( const std::string& stepType, const Strings& motifHierarchy ) :
    MapGenStep( motifHierarchy ) {
    m_defType = stepType;

    SetupEmptyMotif();
    AddChildMotifs( { m_motifDef->GetDefintionType() } );

    const MGS_CustomDef* stepDef = MGS_CustomDef::GetDefinition( m_defType );
    stepDef->DefineObject( *this );
}


std::vector< MapGenStep::CustomEvent > MGS_Custom::GetCustomResults() const {
    std::vector< MapGenStep::CustomEvent > stepResults;
    int numChildren = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numChildren; stepIndex++ ) {
        const MapGenStep* genStep = m_genSteps[stepIndex];
        std::vector< MapGenStep::CustomEvent > childResults = genStep->GetCustomResults();
        stepResults.insert( stepResults.end(), childResults.begin(), childResults.end() );
    }

    return stepResults;
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


void MGS_Custom::SetupEmptyMotif() {
    const MGS_CustomDef* customDef = MGS_CustomDef::GetDefinition( m_defType );
    std::string motifToCopy = customDef->GetMotif();
    std::string nameToAppend = Stringf( "CustomXML.%d", s_numCustomSteps );

    m_motifDef = new MotifDef( motifToCopy, nameToAppend );
    s_numCustomSteps++;
}


void MGS_Custom::SetupMotifFromXML( const XMLElement& element ) {
    // Make new motif based on CustomDef motif
    SetupEmptyMotif();

    // Modify new motif based on XML overrides
    NamedProperties& motifVars = m_motifDef->GetVariableValues();
    Strings varNames = motifVars.GetNames();

    const std::string invalidValue = "__HOPEFULLY_NOT_A_VALUE__";
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
    AddChildMotifs( { m_motifDef->GetDefintionType() } );
}
