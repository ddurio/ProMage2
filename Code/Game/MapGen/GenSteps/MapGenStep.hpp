#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Utils/XMLUtils.hpp"


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
    explicit MapGenStep( const XMLElement& element );
    virtual ~MapGenStep() {};

    static int AddCustomCondition( const std::string& eventName, const Strings& attrNames, CustomAttrRequirement requirement = REQUIRE_ALL );
    static int AddCustomResult( const std::string& eventName, const Strings& attrNames, CustomAttrRequirement requirement = REQUIRE_ALL );
    static void RemoveCustomCondition( int conditionIndex );
    static void RemoveCustomResult( int resultIndex );
    static MapGenStep* CreateMapGenStep( const XMLElement& element );

    void Run( Map& map ) const;

    virtual std::string GetName() const;


    protected:
    static DevConsoleChannel s_mgsChannel;


    virtual bool IsTileValid( Map& theMap, const Tile& tile ) const;
    virtual void ChangeTile( Map& theMap, int tileIndex ) const;
    virtual void ChangeTile( Map& theMap, int tileX, int tileY ) const;

    Tile& GetTile( Map& map, int tileIndex ) const;
    Tile& GetTile( Map& map, int tileX, int tileY ) const;



    private:
    struct CustomEvent {
        public:
        std::string name = "";
        bool isEnabled = true;
        CustomAttrRequirement requirement = REQUIRE_ALL;

        Strings attrNames;
        Strings attrValues;

        CustomEvent() {};
        CustomEvent( const CustomEvent& event, const Strings& parsedValues );
        Strings ParseCustomEvent( const XMLElement& element ) const;
        EventArgs CreateEventArgs() const;
    };

    // General
    std::string m_stepType          = "";
    float m_chanceToRun             = 1.f;
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