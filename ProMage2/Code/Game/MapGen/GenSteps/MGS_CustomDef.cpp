#include "Game/MapGen/GenSteps/MGS_CustomDef.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"
#include "Game/MapGen/GenSteps/MGS_Custom.hpp"


// STATIC ----------------------------------------------------------
Strings MGS_CustomDef::GetAllDefinitionTypes() {
    Strings allTypes;
    std::map< std::string, MGS_CustomDef* >::const_iterator defIter = s_definitions.begin();

    for( defIter; defIter != s_definitions.end(); defIter++ ) {
        allTypes.push_back( defIter->first );
    }

    return allTypes;
}


void MGS_CustomDef::UpdateMotifHierarchy( MapGenStep* genStep, const Strings& parentMotifHierarchy ) {
    GUARANTEE_OR_DIE( parentMotifHierarchy.size() == 3, "(MGS_CustomDef) ERROR -- Expected parent motif array of size three" );

    const Strings& childStepMotifs = genStep->GetMotifs();
    int numChildMotifs = (int)childStepMotifs.size();
    Strings motifHierarchy;

    if( numChildMotifs == 2 ) { // New step.. first time setup
        // Interweaving parent and child motifs
        motifHierarchy = {
            childStepMotifs[0],         //  Child Step: motif="..."
            parentMotifHierarchy[0],    // Custom Step: motifVar="..."
            parentMotifHierarchy[1],    // Custom Step: motif="..."
            parentMotifHierarchy[2],    //    Map  Def: motif="..."
            childStepMotifs[1]          // Custom  Def: <Motif>...
        };
    } else if( numChildMotifs == 5 ) { // Previously setup
        // Just update the three parent motifs
        motifHierarchy = childStepMotifs;
        motifHierarchy[1] = parentMotifHierarchy[0];
        motifHierarchy[2] = parentMotifHierarchy[1];
        motifHierarchy[3] = parentMotifHierarchy[2];
    } else {
        ERROR_AND_DIE( "(MGS_CustomDef) ERROR -- Expected motif array of size two (new step) or five (updating step)" );
    }

    genStep->SetMotifs( motifHierarchy );

    EventArgs args;
    args.SetValue( MAPGEN_ARG_ATTR_NAME, MAPGEN_ARG_RECALC_ALL );
    genStep->RecalculateMotifVars( args );
}


// PUBLIC ----------------------------------------------------------
void MGS_CustomDef::DefineObject( MGS_Custom& theObject ) const {
    const Strings& customStepMotifs = theObject.m_motifHierarchy;
    int numSteps = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* stepToCopy = m_genSteps[stepIndex];
        MapGenStep* newGenStep = MapGenStep::CreateMapGenStep( stepToCopy );

        UpdateMotifHierarchy( newGenStep, customStepMotifs );

        theObject.m_genSteps.push_back( newGenStep );
    }
}


std::string MGS_CustomDef::GetMotif() const {
    return m_motif;
}


// PRIVATE --------------------------------------------------------
MGS_CustomDef::MGS_CustomDef( const XMLElement& element ) {
    // Name
    s_defClass     = "MGS_CustomDef";
    m_defType      = ParseXMLAttribute( element, "name", m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(MGS_CustomDef) Missing required attribute 'name'" );

    // Motif
    const XMLElement* motifEle = element.FirstChildElement( "Motif" );

    if( motifEle != nullptr ) {
        MotifDef* autoMotif = new MotifDef( *motifEle, true );
        m_motif = autoMotif->GetDefintionType();
    }

    // GenSteps
    const XMLElement* stepEle = element.FirstChildElement();

    for( stepEle; stepEle != nullptr; stepEle = stepEle->NextSiblingElement() ) {
        if( !StringICmp( stepEle->Name(), "Motif" ) ) {
            MapGenStep* genStep = MapGenStep::CreateMapGenStep( *stepEle, { m_motif } );
            m_genSteps.push_back( genStep );
        }
    }
}


MGS_CustomDef::~MGS_CustomDef() {
    EngineCommon::ClearVector( m_genSteps ); // This may cause issues (inaccessible destructor)
}
