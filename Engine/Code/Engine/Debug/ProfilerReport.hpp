#pragma once
#include "Engine/Core/EngineCommon.hpp"


struct ProfilerNode;


enum ProfilerReportColumn : unsigned int {
    REPORT_COLUMN_NONE                  = 0U,
    REPORT_COLUMN_NUM_CALLS             = BIT_FLAG( 0 ),
    REPORT_COLUMN_PERCENT_TOTAL         = BIT_FLAG( 1 ),
    REPORT_COLUMN_SECONDS_TOTAL         = BIT_FLAG( 2 ),
    REPORT_COLUMN_AVERAGE_TOTAL_SECONDS = BIT_FLAG( 3 ),
    REPORT_COLUMN_MAX_SECONDS_TOTAL     = BIT_FLAG( 4 ),
    REPORT_COLUMN_PERCENT_SELF          = BIT_FLAG( 5 ),
    REPORT_COLUMN_SECONDS_SELF          = BIT_FLAG( 6 ),
    REPORT_COLUMN_AVERAGE_SELF_SECONDS  = BIT_FLAG( 7 ),
    REPORT_COLUMN_MAX_SECONDS_SELF      = BIT_FLAG( 8 ),
};


ProfilerReportColumn operator|( ProfilerReportColumn flagA, ProfilerReportColumn flagB );


struct ProfilerReportNode {
    std::string label = "";
    int numCalls = 0;
    float frameTotalSeconds = -1.f;

    uint64_t totalHPC = 0;
    uint64_t selfHPC = 0;
    uint64_t maxTotalHPC = 0;
    uint64_t maxSelfHPC = 0;

    std::vector< ProfilerReportNode* > children;


    void Render( ProfilerReportColumn columnFlags ) const;
    bool RenderSelf( ProfilerReportColumn columnFlags ) const;

    ProfilerReportNode* GetOrCreateChild( const char* labelToFind );
    uint64_t AppendChildTreeView( ProfilerNode* treeToAppend, float frameSeconds = -1.f );
    uint64_t AppendChildFlatView( ProfilerNode* treeToAppend, float frameSeconds = -1.f );

    void Sort( const std::string& sortBy, bool orderDesc );


    struct SortTotalDesc {
        bool operator()( const ProfilerReportNode* nodeA, const ProfilerReportNode* nodeB );
    };

    struct SortTotalAsc {
        bool operator()( const ProfilerReportNode* nodeA, const ProfilerReportNode* nodeB );
    };

    struct SortSelfDesc {
        bool operator()( const ProfilerReportNode* nodeA, const ProfilerReportNode* nodeB );
    };

    struct SortSelfAsc {
        bool operator()( const ProfilerReportNode* nodeA, const ProfilerReportNode* nodeB );
    };
};

