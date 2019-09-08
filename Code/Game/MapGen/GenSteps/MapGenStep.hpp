#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"


class Map;
class RNG;
class Tile;


class MapGenStep {
    public:
    explicit MapGenStep( const XMLElement& element );
    virtual ~MapGenStep() {};

    static void AddCustomCondition( const std::string& eventName, const Strings& attrNames, bool requireAllAttr = true );
    static void AddCustomResult( const std::string& eventName, const Strings& attrNames, bool requireAllAttr = true );
    static MapGenStep* CreateMapGenStep( const XMLElement& element );

    void Run( Map& map ) const;


    protected:
    static DevConsoleChannel s_mgsChannel;
    mutable RNG* m_mapRNG = nullptr;


    virtual bool IsTileValid( const Tile& tile ) const;
    virtual void ChangeTile( Map& map, int tileIndex ) const;
    virtual void ChangeTile( Map& map, int tileX, int tileY ) const;

    Tile& GetTile( Map& map, int tileIndex ) const;
    Tile& GetTile( Map& map, int tileX, int tileY ) const;



    private:
    struct CustomEvent {
        public:
        std::string name = "";
        bool requireAll = true;

        Strings attrNames;
        Strings attrValues;

        CustomEvent() {};
        CustomEvent( const CustomEvent& event, const Strings& parsedValues );
        Strings ParseCustomEvent( const XMLElement& element ) const;
        EventArgs CreateEventArgs() const;
    };

    // General
    std::string m_stepType          = "";
    FloatRange m_chanceToRun        = FloatRange::ONE;
    IntRange m_numIterations        = IntRange::ONE;

    // Conditions
    std::string m_ifIsType          = "";
    Strings m_ifHasTags;
    std::map< std::string, FloatRange, StringCmpCaseI > m_ifHeatMap;
    static std::vector< CustomEvent > s_customConditions;
    std::vector< CustomEvent > m_customConditions;
    
    // Results
    std::string m_setType           = "";
    Strings m_setTags;
    std::map< std::string, FloatRange, StringCmpCaseI > m_setHeatMap;
    static std::vector< CustomEvent > s_customResults;
    std::vector< CustomEvent > m_customResults;


    virtual void RunOnce( Map& map ) const = 0;
};