#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Async/AsyncQueue.hpp"
#include "Engine/Utils/NamedStrings.hpp"

#include "Game/MapGen/Map/MapDef.hpp"


class EditorMapDef : public Definition< Map, EditorMapDef >, public MapDef {
    friend class Definition< Map, EditorMapDef >;

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
    void InsertStepBefore( int stepIndexToInsertBefore, MapGenStep* stepToInsert );
    void InsertStepAfter( int stepIndexToInsertAfter, MapGenStep* stepToInsert );
    void DeleteStep( int stepIndexToDelete );

    static int SetupChangeTileMGS();
    static bool SaveAllToXml( EventArgs& args );


    private:
    struct AsyncPayload {
        Map* theMap = nullptr;
        int numStepsToDo = -1;
        int numStepsDone = 0;
    };

    using Definition< Map, EditorMapDef >::s_defClass;
    using Definition< Map, EditorMapDef >::s_definitions;
    using MapDef                         ::m_defType;

    int m_numSteps = 3; // FillAndEdge, Context, Colliders

    mutable std::vector< Map* >* m_mapPerStep   = nullptr;
    mutable bool m_allJobsStarted               = false;
    mutable int m_numJobsRunning                = 0;

    mutable std::vector< std::thread > m_threads;
    mutable AsyncQueue< AsyncPayload > m_workerPayloads;
    mutable AsyncQueue< AsyncPayload > m_mainPayloads;



    explicit EditorMapDef( const XMLElement& element );
    ~EditorMapDef();

    bool IsFinished() const;
    bool CompleteStep( AsyncPayload& payload ) const;

    void SpinUpThreads() const;
    void LaunchJobs( bool useCustomSeed, unsigned int customSeed ) const;
    void ProcessWorkerPayloads() const;
    void ProcessMainPayloads() const;
    void SpinDownThreads() const;

    void DefineFromContextTiles( Map& theMap ) const override;

    bool SaveOneToXml( EventArgs& args );
};
