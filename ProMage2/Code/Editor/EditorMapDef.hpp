#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Async/AsyncQueue.hpp"
#include "Engine/Async/Job.hpp"
#include "Engine/Utils/NamedStrings.hpp"

#include "Game/MapGen/Map/MapDef.hpp"


class EditorMapDef : public Definition< Map, EditorMapDef >, public MapDef {
    friend class Definition< Map, EditorMapDef >;
    friend class EditorMapDefJob;

    public:
    void DefineObject( std::vector< Map* >* mapSteps, bool useCustomSeed = false, unsigned int customSeed = 0 ) const;
    void DefineObject( Map& theMap ) const override;

    using Definition< Map, EditorMapDef >::LoadFromFile;
    using Definition< Map, EditorMapDef >::SaveToFile;
    using Definition< Map, EditorMapDef >::GetDefinition;
    using Definition< Map, EditorMapDef >::DestroyDefs;

    void RenderMapDefParams();

    MapGenStep* GetStep( int stepIndex ) const; // Gets a NON-CONST version for editor changes
    Strings GetStepNames( int indexOffset = 0 ) const;
    Strings GetMapTypes() const;

    void ReorderStepUp( int stepIndexToMove );
    void ReorderStepDown( int stepIndexToMove );
    void InsertStepBefore( MapGenStep* stepToInsert, int indexToInsertBefore );
    void InsertStepAfter( MapGenStep* stepToInsert, int indexToInsertAfter );
    void DeleteStep( int stepIndexToDelete );

    static int SetupChangeTileMGS();
    static bool SaveAllToXml( EventArgs& args );
    static EditorMapDef* CreateNewMapDef( const std::string& mapType, const std::string& fillType );

    static const int NUM_PRE_STEPS  = 1;    // FillAndEdge
    static const int NUM_POST_STEPS = 3;    // EdgedTiles, SouthWall, Colliders


    private:
    using Definition< Map, EditorMapDef >::s_defClass;
    using Definition< Map, EditorMapDef >::s_definitions;
    using MapDef                         ::m_defType;

    int m_numSteps = NUM_PRE_STEPS + NUM_POST_STEPS;

    mutable std::vector< Map* >* m_mapPerStep       = nullptr;
    mutable bool                 m_allJobsStarted   = false;
    mutable std::atomic< int >   m_numJobsRunning   = 0;


    explicit EditorMapDef( const XMLElement& element );
    explicit EditorMapDef( const std::string& mapType, const std::string& fillType );
    ~EditorMapDef();

    using Definition< Map, EditorMapDef >::AddDefinition;

    bool IsFinished() const;

    void LaunchJobs( bool useCustomSeed, unsigned int customSeed ) const;
    void WaitForJobs() const;

    void DefineFromEdgedTiles( Map& theMap ) const override;
    void DefineFromWallTiles( Map& theMap ) const override;
};


class EditorMapDefJob : public Job {
    public:
    EditorMapDefJob( const EditorMapDef* eMapDef, Map* theMap, int numStepsToDo );
    ~EditorMapDefJob() {};

    const EditorMapDef* GetMapDef() const;


    private:
    const EditorMapDef* m_eMapDef = nullptr;
    Map* m_theMap = nullptr;
    int m_numStepsToDo = -1;
    int m_numStepsDone = 0;


    void Execute() override;
    bool CompleteStep();
};
