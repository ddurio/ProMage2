#include "Game/MapGen/GenSteps/MGS_VirtualDef.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"
#include "Game/MapGen/GenSteps/MGS_Virtual.hpp"


void MGS_VirtualDef::DefineObject( MGS_Virtual& theObject ) const {
    const Strings& objectMotifs = theObject.m_motifHierarchy;
    GUARANTEE_OR_DIE( objectMotifs.size() == 2, "(MGS_VirtualDef) ERROR -- Expected motif array of size two" );

    int numSteps = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* stepToCopy = m_genSteps[stepIndex];
        MapGenStep* newGenStep = MapGenStep::CreateMapGenStep( stepToCopy );

        const Strings& stepMotifs = newGenStep->GetMotifs();
        GUARANTEE_OR_DIE( stepMotifs.size() == 2, "(MGS_VirtualDef) ERROR -- Expected motif array of size two" );

        // Interweaving motifs...  Child XML,     Virtual XML,     VirtualDef,    Map
        Strings motifHierarchy = { stepMotifs[0], objectMotifs[0], stepMotifs[1], objectMotifs[1] };

        newGenStep->AddParentMotifs( motifHierarchy );

        theObject.m_genSteps.push_back( newGenStep );
    }
}


std::string MGS_VirtualDef::GetMotif() const {
    return m_motif;
}


// PRIVATE --------------------------------------------------------
MGS_VirtualDef::MGS_VirtualDef( const XMLElement& element ) {
    // Name
    s_defClass     = "MGS_VirtualDef";
    m_defType      = ParseXMLAttribute( element, "name", m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(MGS_VirtualDef) Missing required attribute 'name'" );

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


MGS_VirtualDef::~MGS_VirtualDef() {
    EngineCommon::ClearVector( m_genSteps ); // This may cause issues (inaccessible destructor)
}
