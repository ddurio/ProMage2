#include "Game/ActorDef.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RNG.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Actor.hpp"
#include "Game/Inventory.hpp"
#include "Game/Map.hpp"
#include "Game/StatsManager.hpp"


template<>
Definition<Actor>::Definition( const XMLElement& element ) {
    // Parse Values
    m_defType                   = ParseXMLAttribute( element, "name",         m_defType );
    GUARANTEE_OR_DIE( m_defType != "", "(ActorDef) Missing required attribute 'name'" );

    bool canSee                 = ParseXMLAttribute( element, "canSee",  true );
    bool canWalk                = ParseXMLAttribute( element, "canWalk", true );
    bool canFly                 = ParseXMLAttribute( element, "canFly",  false );
    bool canSwim                = ParseXMLAttribute( element, "canSwim", false );
    bool canDie                 = ParseXMLAttribute( element, "canDie",  true );

    const XMLElement* childEle = element.FirstChildElement();

    Strings       parentOptions;
    Strings       bodyOptions;
    Strings       bodyItemSets;
    Strings       earOptions;
    Strings       hairOptions;
    std::string   itemSetsCSV;
    StatsManager* statsManager = new StatsManager();
    int money = 0;

    while( childEle != nullptr ) {
        std::string tagName = childEle->Name();

        if( tagName == "Parent" ) {
            std::string parentName = ParseXMLAttribute( *childEle, "name", "" );
            GUARANTEE_OR_DIE( parentName != "", "(ActorDef) Parent tag missing required attribute 'name'" );
            parentOptions.push_back( parentName );
        } else if( tagName == "Body" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Body tag missing required attribute 'spriteSheet'" );
            bodyOptions.push_back( spriteName );

            std::string setName = ParseXMLAttribute( *childEle, "itemSet", "" );
            bodyItemSets.push_back( setName );
        } else if( tagName == "Ears" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Ears tag missing required attribute 'spriteSheet'" );
            earOptions.push_back( spriteName );
        } else if( tagName == "Hair" ) {
            std::string spriteName = ParseXMLAttribute( *childEle, "spriteSheet", "" );
            GUARANTEE_OR_DIE( spriteName != "", "(ActorDef) Hair tag missing required attribute 'spriteSheet''" );
            hairOptions.push_back( spriteName );
        } else if( tagName == "ItemSet" ) {
            std::string setName = ParseXMLAttribute( *childEle, "name", "" );
            GUARANTEE_OR_DIE( setName != "", "(ActorDef) ItemSet tag missing required attribute 'name'" );
            itemSetsCSV = Stringf( "%s,%s", itemSetsCSV.c_str(), setName.c_str() );
        } else if( tagName == "Stats" ) {
            CLEAR_POINTER( statsManager );
            statsManager = new StatsManager( *childEle );
        } else if( tagName == "Money" ) {
            money = ParseXMLAttribute( *childEle, "value", money );
        }

        childEle = childEle->NextSiblingElement();
    }


    // Set Properties
    m_properties.SetValue( "canSee",        canSee );
    m_properties.SetValue( "canWalk",       canWalk );
    m_properties.SetValue( "canFly",        canFly );
    m_properties.SetValue( "canSwim",       canSwim );
    m_properties.SetValue( "canDie",        canDie );
    m_properties.SetValue( "parents",       parentOptions );
    m_properties.SetValue( "bodySprites",   bodyOptions );
    m_properties.SetValue( "bodyItemSets",  bodyItemSets );
    m_properties.SetValue( "earSprites",    earOptions );
    m_properties.SetValue( "hairSprites",   hairOptions );
    m_properties.SetValue( "validItemSets", itemSetsCSV );
    m_properties.SetValue( "statsManager",  (const StatsManager*)statsManager );
    m_properties.SetValue( "money",         money );


    g_theDevConsole->PrintString( Stringf( "(ActorDef) Loaded new ActorDef (%s)", m_defType.c_str() ) );
    s_definitions[m_defType] = this;
}


template<>
void Definition<Actor>::Define( Actor& theObject ) const {
    Map* theMap = theObject.m_map;
    RNG* mapRNG = theMap->GetMapRNG();

    Strings parentOptions;
    parentOptions = m_properties.GetValue( "parents", parentOptions );
    int numParents = (int)parentOptions.size();

    // Select parent actor type
    if( numParents > 0 ) {
        int parentIndex = mapRNG->GetRandomIntLessThan( numParents );
        const Definition<Actor>* parentDef = Definition<Actor>::GetDefinition( parentOptions[parentIndex] );
        parentDef->Define( theObject );
    } else { // No parents
        theObject.m_inventory = new Inventory( &theObject, theObject.m_map );

        const StatsManager* statsProto = m_properties.GetValue( "statsManager", (const StatsManager*)nullptr );
        GUARANTEE_OR_DIE( statsProto != nullptr, "(ActorDef) StatsManager prototype was nullptr!" );
        theObject.m_statsManager = new StatsManager( *statsProto );
        theObject.m_statsManager->m_myActor = &theObject;
    }

    int parentMoney = theObject.m_inventory->GetMoney();
    int childMoney = m_properties.GetValue( "money", 0 );

    int transferMoney = childMoney - parentMoney;
    theObject.m_inventory->TransferMoney( transferMoney );

    // Body appearance
    Strings bodyOptions;
    Strings bodyItemSets;
    Strings earOptions;
    Strings hairOptions;
    bodyOptions  = m_properties.GetValue( "bodySprites",  bodyOptions );
    bodyItemSets = m_properties.GetValue( "bodyItemSets", bodyItemSets );
    earOptions   = m_properties.GetValue( "earSprites",   earOptions );
    hairOptions  = m_properties.GetValue( "hairSprites",  hairOptions );

    int numBody = (int)bodyOptions.size();
    int numEars = (int)earOptions.size();
    int numHair = (int)hairOptions.size();
    std::string bodyItemSet = "";

    if( numBody > 0 ) {
        int bodyIndex = mapRNG->GetRandomIntLessThan( numBody );
        theObject.m_paperDollSprites[PAPER_DOLL_BODY] = bodyOptions[bodyIndex];

        bodyItemSet = bodyItemSets[bodyIndex];
    }

    if( numEars > 0 ) {
        int earIndex = mapRNG->GetRandomIntLessThan( numEars );
        theObject.m_paperDollSprites[PAPER_DOLL_EARS] = earOptions[earIndex];
    }

    if( numHair > 0 ) {
        int hairIndex = mapRNG->GetRandomIntLessThan( numHair );
        theObject.m_paperDollSprites[PAPER_DOLL_HAIR] = hairOptions[hairIndex];
    }


    // Entity Attributes
    theObject.m_canSee  = m_properties.GetValue( "canSee",  theObject.m_canSee  );
    theObject.m_canWalk = m_properties.GetValue( "canWalk", theObject.m_canWalk );
    theObject.m_canFly  = m_properties.GetValue( "canFly",  theObject.m_canFly  );
    theObject.m_canSwim = m_properties.GetValue( "canSwim", theObject.m_canSwim );
    theObject.m_canDie  = m_properties.GetValue( "canDie",  theObject.m_canDie  );


    // Item Sets
    std::string itemSetsCSV = "";
    itemSetsCSV = m_properties.GetValue( "validItemSets", itemSetsCSV );

    if( bodyItemSet != "" ) {
        itemSetsCSV = Stringf( "%s,%s", itemSetsCSV.c_str(), bodyItemSet.c_str() );
    }

    theObject.m_inventory->AddItemSets( itemSetsCSV );
}
