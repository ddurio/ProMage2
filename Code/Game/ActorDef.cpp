#include "Game/ActorDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"
#include "Game/MapGen/Map/Map.hpp"
#include "Game/StatsManager.hpp"


void ActorDef::DefineObject( Actor& theObject ) const {
    Map* theMap = theObject.m_map;
    RNG* mapRNG = theMap->GetMapRNG();

    int numParents = (int)m_parentActors.size();

    // Select parent actor type
    if( numParents > 0 ) {
        int parentIndex = mapRNG->GetRandomIntLessThan( numParents );
        const ActorDef* parentDef = GetDefinition( m_parentActors[parentIndex] );
        parentDef->DefineObject( theObject );
    } else { // No parents
        theObject.m_inventory = new Inventory( &theObject, theObject.m_map );

        GUARANTEE_OR_DIE( m_statsManager != nullptr, "(ActorDef) StatsManager prototype was nullptr!" );
        theObject.m_statsManager = new StatsManager( *m_statsManager );
        theObject.m_statsManager->m_myActor = &theObject;
    }

    int parentMoney = theObject.m_inventory->GetMoney();
    int transferMoney = m_money - parentMoney;
    theObject.m_inventory->TransferMoney( transferMoney );

    // Body appearance
    int numBody = (int)m_bodySprites.size();
    int numEars = (int)m_earSprites.size();
    int numHair = (int)m_hairSprites.size();
    std::string bodyItemSet = "";

    if( numBody > 0 ) {
        int bodyIndex = mapRNG->GetRandomIntLessThan( numBody );
        theObject.m_paperDollSprites[PAPER_DOLL_BODY] = m_bodySprites[bodyIndex];

        bodyItemSet = m_bodyItemSets[bodyIndex];
    }

    if( numEars > 0 ) {
        int earIndex = mapRNG->GetRandomIntLessThan( numEars );
        theObject.m_paperDollSprites[PAPER_DOLL_EARS] = m_earSprites[earIndex];
    }

    if( numHair > 0 ) {
        int hairIndex = mapRNG->GetRandomIntLessThan( numHair );
        theObject.m_paperDollSprites[PAPER_DOLL_HAIR] = m_hairSprites[hairIndex];
    }


    // Entity Attributes
    theObject.m_canSee  = m_canSee;
    theObject.m_canWalk = m_canWalk;
    theObject.m_canFly  = m_canFly;
    theObject.m_canSwim = m_canSwim;
    theObject.m_canDie  = m_canDie;


    // Item Sets
    std::string itemSetsCSV = m_validItemSets;

    if( bodyItemSet != "" ) {
        itemSetsCSV = Stringf( "%s,%s", itemSetsCSV.c_str(), bodyItemSet.c_str() );
    }

    theObject.m_inventory->AddItemSets( itemSetsCSV );
}


float ActorDef::GetSpritePPU() const {
    return m_spritePPU;
}


ActorDef::ActorDef( const XMLElement& element ) {
    s_defClass = "ActorDef";

    // Parse Values
    m_defType = ParseXMLAttribute( element, "name", m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(ActorDef) Missing required attribute 'name'" );

    m_canSee  = ParseXMLAttribute( element, "canSee",  m_canSee );
    m_canWalk = ParseXMLAttribute( element, "canWalk", m_canWalk );
    m_canFly  = ParseXMLAttribute( element, "canFly",  m_canFly );
    m_canSwim = ParseXMLAttribute( element, "canSwim", m_canSwim );
    m_canDie  = ParseXMLAttribute( element, "canDie",  m_canDie );

    const XMLElement* childEle = element.FirstChildElement();

    while( childEle != nullptr ) {
        std::string tagName = childEle->Name();

        if( tagName == "Parent" ) {
            std::string parentName = ParseXMLAttribute( *childEle, "name", "" );
            GUARANTEE_OR_DIE( parentName != "", "(ActorDef) Parent tag missing required attribute 'name'" );
            m_parentActors.push_back( parentName );

        } else if( tagName == "Body" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Body tag missing required attribute 'spriteSheet'" );
            m_bodySprites.push_back( spriteName );

            std::string setName = ParseXMLAttribute( *childEle, "itemSet", "" );
            m_bodyItemSets.push_back( setName );

        } else if( tagName == "Ears" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Ears tag missing required attribute 'spriteSheet'" );
            m_earSprites.push_back( spriteName );

        } else if( tagName == "Hair" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Hair tag missing required attribute 'spriteSheet''" );
            m_hairSprites.push_back( spriteName );

        } else if( tagName == "ItemSet" ) {
            std::string setName = ParseXMLAttribute( *childEle, "name", "" );
            GUARANTEE_OR_DIE( setName != "", "(ActorDef) ItemSet tag missing required attribute 'name'" );
            m_validItemSets = Stringf( "%s,%s", m_validItemSets.c_str(), setName.c_str() );

        } else if( tagName == "Stats" ) {
            m_statsManager = new StatsManager( *childEle );

        } else if( tagName == "Money" ) {
            m_money = ParseXMLAttribute( *childEle, "value", m_money );
        }

        childEle = childEle->NextSiblingElement();
    }


    if( m_statsManager == nullptr ) {
        m_statsManager = new StatsManager();
    }
}


ActorDef::~ActorDef() {
    CLEAR_POINTER( m_statsManager );
}
