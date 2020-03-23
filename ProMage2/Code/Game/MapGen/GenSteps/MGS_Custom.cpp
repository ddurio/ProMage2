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
    AddChildMotifs( { m_customXmlMotif->GetDefintionType() } );

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
    UNUSED( document );

    // Get all values from customXmlMotif
    const NamedProperties& motifVars = m_customXmlMotif->GetVariableValues();
    Strings varKeys = motifVars.GetNames();
    int numVars = (int)varKeys.size();

    for( int varIndex = 0; varIndex < numVars; varIndex++ ) {
        const std::string& varKey = varKeys[varIndex];
        const std::string& varValue = motifVars.GetValue( varKey, "" );

        element.SetAttribute( varKey.c_str(), varValue.c_str() );
    }

    // Get step motif value (should be motifHierarchy[1])
    const std::string& stepMotif = m_motifHierarchy[1];

    if( stepMotif != "" ) {
        element.SetAttribute( "Motif", stepMotif.c_str() );
    }
}


void MGS_Custom::UpdateParentMotifs( const Strings& parentMotifHierarchy ) {
    GUARANTEE_OR_DIE( parentMotifHierarchy.size() == 1, "(MGS_Custom) ERROR -- Parent motif hierarchy must only be mapDef motif (size 1)" );
    GUARANTEE_OR_DIE( m_motifHierarchy.size() == 3, "(MGS_Custom) ERROR -- Motif hierarchy must be of size 3" );

    m_motifHierarchy[2] = parentMotifHierarchy[0];
    int numSteps = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* genStep = m_genSteps[stepIndex];
        MGS_CustomDef::UpdateMotifHierarchy( genStep, m_motifHierarchy );
    }
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

    m_customXmlMotif = new MotifDef( motifToCopy, nameToAppend );
    s_numCustomSteps++;
}


void MGS_Custom::SetupMotifFromXML( const XMLElement& element ) {
    // Make new motif based on CustomDef motif
    SetupEmptyMotif();

    // Modify new motif based on XML overrides
    NamedProperties& motifVars = m_customXmlMotif->GetVariableValues();
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
    AddChildMotifs( { m_customXmlMotif->GetDefintionType() } );
}
