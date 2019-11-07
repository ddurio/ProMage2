#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"

#include "Game/XMLUtils.hpp"


typedef std::map< std::string, FloatRange, StringCmpCaseI > HeatMaps;


class Map;
class RNG;
class Tile;


enum CustomAttrRequirement {
    REQUIRE_ALL,
    REQUIRE_ONE,
    REQUIRE_NONE
};


class MapGenStep {
    friend class EditorMapGenStep;

    public:
    explicit MapGenStep( const XMLElement& element, const Strings& motifHierarchy );
    explicit MapGenStep() {};
    virtual ~MapGenStep() {};

    static int AddCustomCondition( const std::string& eventName, const Strings& attrNames, CustomAttrRequirement requirement = REQUIRE_ALL );
    static int AddCustomResult( const std::string& eventName, const Strings& attrNames, CustomAttrRequirement requirement = REQUIRE_ALL );

    static void RemoveCustomCondition( int conditionIndex );
    static void RemoveCustomResult( int resultIndex );

    static MapGenStep* CreateMapGenStep( const XMLElement& element, const Strings& motifHierarchy );
    static MapGenStep* CreateMapGenStep( const MapGenStep* stepToCopy );
    static MapGenStep* CreateMapGenStep( const std::string& stepType );


    void Run( Map& map ) const;

    struct CustomEvent;
    virtual std::vector< CustomEvent > GetCustomResults() const;
    virtual std::string GetName() const;
    const Strings& GetMotifs() const;

    virtual void SaveToXml( XmlDocument& document, XMLElement& element ) const;
    virtual bool RecalculateMotifVars( EventArgs& args );

    void SetMotifs( const Strings& motifsToSet );
    void AddParentMotifs( const Strings& parentMotifs );
    void AddChildMotifs( const Strings& motifsToAdd );


    struct CustomEvent {
        public:
        std::string name = "";
        bool isEnabled = true;
        CustomAttrRequirement requirement = REQUIRE_ALL;

        Strings attrNames;
        Strings attrValues;

        CustomEvent() {};
        CustomEvent( const CustomEvent& event, const Strings& parsedValues );
        Strings ParseCustomEvent( const XMLElement& element, MapGenStep* genStep ) const;
        EventArgs CreateEventArgs() const;
    };


    protected:
    // General
    std::string m_stepType          = "";
    Strings m_motifHierarchy;
    NamedStrings m_motifVars;

    float m_chanceToRun             = 1.f;
    IntRange m_numIterations        = IntRange::ONE;

    // Conditions
    std::string m_ifIsType          = "";
    Strings m_ifHasTags;
    HeatMaps m_ifHeatMap;
    static std::vector< CustomEvent > s_customConditions;
    std::vector< CustomEvent > m_customConditions;
    
    // Results
    std::string m_setType           = "";
    Strings m_setTags;
    HeatMaps m_setHeatMap;
    static std::vector< CustomEvent > s_customResults;
    std::vector< CustomEvent > m_customResults;

    static DevConsoleChannel s_mgsChannel;


    virtual void RunOnce( Map& map ) const = 0;

    virtual bool IsTileValid( Map& theMap, const Tile& tile ) const;
    virtual void ChangeTile( Map& theMap, int tileIndex ) const;
    void ChangeTile( Map& theMap, int tileX, int tileY ) const;

    void ChangeTileType( Tile& tile, const std::string& customType = "" ) const;
    void ChangeTileTags( Tile& tile, const Strings& customTags = Strings() ) const;
    void ChangeTileHeatMaps( RNG* mapRNG, Tile& tile, const HeatMaps& customHeatMaps = HeatMaps() ) const;
    void ChangeTilesCustomResults( Map& theMap, Tile& tile ) const;

    Tile& GetTile( Map& map, int tileIndex ) const;
    Tile& GetTile( Map& map, int tileX, int tileY ) const;
};