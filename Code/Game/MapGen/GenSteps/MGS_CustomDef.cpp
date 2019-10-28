#include "Game/MapGen/GenSteps/MGS_CustomDef.hpp"

#include "Game/MapGen/GenSteps/MapGenStep.hpp"
#include "Game/MapGen/GenSteps/MGS_Custom.hpp"


void MGS_CustomDef::DefineObject( MGS_Custom& theObject ) const {
    const Strings& objectMotifs = theObject.m_motifHierarchy;
    GUARANTEE_OR_DIE( objectMotifs.size() == 3, "(MGS_CustomDef) ERROR -- Expected motif array of size three" );

    int numSteps = (int)m_genSteps.size();

    for( int stepIndex = 0; stepIndex < numSteps; stepIndex++ ) {
        MapGenStep* stepToCopy = m_genSteps[stepIndex];
        MapGenStep* newGenStep = MapGenStep::CreateMapGenStep( stepToCopy );

        const Strings& stepMotifs = newGenStep->GetMotifs();
        GUARANTEE_OR_DIE( stepMotifs.size() == 2, "(MGS_CustomDef) ERROR -- Expected motif array of size two" );

        // Interweaving motifs...  Child Step,    Custom Step,     Custom Step,     MapDef           CustomDef,     
        //                         motif="..."    motifVar="..."   motif="..."      motif="..."      <Motif>...     
        Strings motifHierarchy = { stepMotifs[0], objectMotifs[0], objectMotifs[1], objectMotifs[2], stepMotifs[1] };

        newGenStep->SetMotifs( motifHierarchy );

        EventArgs args;
        args.SetValue( "attrName", "All" );
        newGenStep->RecalculateMotifVars( args );

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
