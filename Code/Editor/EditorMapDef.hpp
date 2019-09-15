#pragma once
#include "Editor/EditorCommon.hpp"

#include "Engine/Core/AsyncQueue.hpp"

#include "Game/MapGen/Map/MapDef.hpp"


class EditorMapDef : public Definition< Map, EditorMapDef >, public MapDef {
    friend class Definition< Map, EditorMapDef >;

    public:
    void DefineObject( std::vector< Map* >* mapSteps ) const;
    void DefineObject( Map& theMap ) const override;

    using Definition< Map, EditorMapDef >::LoadFromFile;
    using Definition< Map, EditorMapDef >::GetDefinition;
    using Definition< Map, EditorMapDef >::DestroyDefs;


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
    ~EditorMapDef() {};

    bool IsFinished() const;
    bool FinishStep( AsyncPayload& payload ) const;
    //void DefineFromMGS( Map& theMap ) const override;

    void SpinUpThreads() const;
    void LaunchJobs() const;
    void ProcessWorkerPayloads() const;
    void ProcessMainPayloads() const;
    void SpinDownThreads() const;
};
