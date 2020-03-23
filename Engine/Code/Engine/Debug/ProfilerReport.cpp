#include "Engine/Debug/ProfilerReport.hpp"

#include "Engine/Core/ImGuiSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Debug/Profiler.hpp"


ProfilerReportColumn operator|( ProfilerReportColumn flagA, ProfilerReportColumn flagB ) {
    return (ProfilerReportColumn)((unsigned int)flagA | (unsigned int)flagB);
}


void ProfilerReportNode::Render( ProfilerReportColumn columnFlags ) const {
    bool isRootNode = (frameTotalSeconds < 0.f);

    // Only renderSelf if NOT the root node
    if( isRootNode || RenderSelf( columnFlags ) ) {
        int numChildren = (int)children.size();

        for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
            const ProfilerReportNode* child = children[childIndex];
            child->Render( columnFlags );
        }

        if( !isRootNode ) {
            ImGui::TreePop();
        }
    }
}


bool ProfilerReportNode::RenderSelf( ProfilerReportColumn columnFlags ) const {
    int numChildren = (int)children.size();
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen;

    if( numChildren == 0 ) {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
    }

    // Render tree
    bool isOpen = ImGui::TreeNodeEx( label.c_str(), nodeFlags );
    //bool isHovered = ImGui::IsItemHovered(); // Use to highlight rest of row
    ImGui::NextColumn();
    
    // Num Calls
    if( (columnFlags & REPORT_COLUMN_NUM_CALLS) == REPORT_COLUMN_NUM_CALLS ) {
        ImGui::Text( "%d", numCalls );
        ImGui::NextColumn();
    }

    // Percent Total
    if( (columnFlags & REPORT_COLUMN_PERCENT_TOTAL) == REPORT_COLUMN_PERCENT_TOTAL ) {
        float nodeTotalSeconds = (float)ConvertHPCTOSeconds( totalHPC );
        float percentTotal = (nodeTotalSeconds / frameTotalSeconds) * 100.f;
        ImGui::Text( "%0.2f%%", percentTotal );
        ImGui::NextColumn();
    }

    // Seconds Total
    if( (columnFlags & REPORT_COLUMN_SECONDS_TOTAL) == REPORT_COLUMN_SECONDS_TOTAL ) {
        std::string timeStr = GetProfileTimeString( totalHPC );
        ImGui::Text( "%s", timeStr.c_str() );
        ImGui::NextColumn();
    }

    // Avg Total
    if( (columnFlags & REPORT_COLUMN_AVERAGE_TOTAL_SECONDS) == REPORT_COLUMN_AVERAGE_TOTAL_SECONDS ) {
        double avgTotalSeconds = ConvertHPCTOSeconds( totalHPC ) / numCalls;
        uint64_t avgTotalHPC = ConvertSecondsToHPC( avgTotalSeconds );
        std::string timeStr = GetProfileTimeString( avgTotalHPC );
        ImGui::Text( "%s", timeStr.c_str() );
        ImGui::NextColumn();
    }

    // Max Total
    if( (columnFlags & REPORT_COLUMN_MAX_SECONDS_TOTAL) == REPORT_COLUMN_MAX_SECONDS_TOTAL ) {
        std::string timeStr = GetProfileTimeString( maxTotalHPC );
        ImGui::Text( "%s", timeStr.c_str() );
        ImGui::NextColumn();
    }

    // Percent Self
    if( (columnFlags & REPORT_COLUMN_PERCENT_SELF) == REPORT_COLUMN_PERCENT_SELF ) {
        float nodeSelfSeconds = (float)ConvertHPCTOSeconds( selfHPC );
        float percentSelf = (nodeSelfSeconds / frameTotalSeconds) * 100.f;
        ImGui::Text( "%0.2f%%", percentSelf );
        ImGui::NextColumn();
    }

    // Seconds Self
    if( (columnFlags & REPORT_COLUMN_SECONDS_SELF) == REPORT_COLUMN_SECONDS_SELF ) {
        std::string timeStr = GetProfileTimeString( selfHPC );
        ImGui::Text( "%s", timeStr.c_str() );
        ImGui::NextColumn();
    }

    // Avg Self
    if( (columnFlags & REPORT_COLUMN_AVERAGE_SELF_SECONDS) == REPORT_COLUMN_AVERAGE_SELF_SECONDS ) {
        double avgSelfSeconds = ConvertHPCTOSeconds( selfHPC ) / numCalls;
        uint64_t avgSelfHPC = ConvertSecondsToHPC( avgSelfSeconds );
        std::string timeStr = GetProfileTimeString( avgSelfHPC );
        ImGui::Text( "%s", timeStr.c_str() );
        ImGui::NextColumn();
    }

    // Max Self
    if( (columnFlags & REPORT_COLUMN_MAX_SECONDS_SELF) == REPORT_COLUMN_MAX_SECONDS_SELF ) {
        std::string timeStr = GetProfileTimeString( maxSelfHPC );
        ImGui::Text( "%s", timeStr.c_str() );
        ImGui::NextColumn();
    }

    return isOpen;
}


ProfilerReportNode* ProfilerReportNode::GetOrCreateChild( const char* labelToFind ) {
    int numChildren = (int)children.size();

    // Get
    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
       if( StringICmp( children[childIndex]->label, labelToFind ) ) {
           return children[childIndex];
        }
    }

    // Or create
    ProfilerReportNode* newNode = new ProfilerReportNode();
    newNode->label = labelToFind;
    children.push_back( newNode );

    return children.back();
}


uint64_t ProfilerReportNode::AppendChildTreeView( ProfilerNode* treeToAppend, float frameSeconds /*= -1.f */ ) {
    // Update info for this node
    uint64_t treeTotalHPC = treeToAppend->endHPC - treeToAppend->startHPC;
    uint64_t treeSelfHPC = treeTotalHPC;

    if( frameSeconds < 0.f ) {
        frameSeconds = (float)ConvertHPCTOSeconds( treeTotalHPC );
    }

    totalHPC += treeTotalHPC;

    // Add tree as child
    ProfilerReportNode* childReport = GetOrCreateChild( treeToAppend->label );
    ProfilerNode* treeBranch = treeToAppend->firstChild;

    if( treeBranch != nullptr ) {
        while( treeBranch != nullptr ) {
            uint64_t branchHPC = childReport->AppendChildTreeView( treeBranch, frameSeconds );
            treeSelfHPC -= branchHPC;

            treeBranch = treeBranch->nextSibling;
        }
    }

    // Update child info
    childReport->numCalls++;
    childReport->frameTotalSeconds = frameSeconds;
    childReport->totalHPC    += treeSelfHPC; // children will have added their total time during recursion
    childReport->selfHPC     += treeSelfHPC;
    childReport->maxTotalHPC = (childReport->maxTotalHPC > treeTotalHPC) ? childReport->maxTotalHPC : treeTotalHPC; // layman's Max
    childReport->maxSelfHPC  = (childReport->maxSelfHPC  > treeSelfHPC)  ? childReport->maxSelfHPC  : treeSelfHPC;  // layman's Max

    return treeTotalHPC;
}


uint64_t ProfilerReportNode::AppendChildFlatView( ProfilerNode* treeToAppend, float frameSeconds /*= -1.f */ ) {
    // Update info for this node
    uint64_t treeTotalHPC = treeToAppend->endHPC - treeToAppend->startHPC;
    uint64_t treeSelfHPC = treeTotalHPC;

    if( frameSeconds < 0.f ) {
        frameSeconds = (float)ConvertHPCTOSeconds( treeTotalHPC );
    }

    totalHPC += treeTotalHPC;
    numCalls++;

    // Add tree as child
    ProfilerReportNode* childReport = GetOrCreateChild( treeToAppend->label );
    ProfilerNode* treeBranch = treeToAppend->firstChild;

    if( treeBranch != nullptr ) {
        while( treeBranch != nullptr ) {
            uint64_t branchHPC = AppendChildFlatView( treeBranch, frameSeconds );
            treeSelfHPC -= branchHPC;

            treeBranch = treeBranch->nextSibling;
        }
    }

    // Update child info
    childReport->numCalls++;
    childReport->frameTotalSeconds = frameSeconds;
    childReport->totalHPC    += treeTotalHPC;
    childReport->selfHPC     += treeSelfHPC;
    childReport->maxTotalHPC = (childReport->maxTotalHPC > treeTotalHPC) ? childReport->maxTotalHPC : treeTotalHPC; // layman's Max
    childReport->maxSelfHPC  = (childReport->maxSelfHPC  > treeSelfHPC)  ? childReport->maxSelfHPC  : treeSelfHPC;  // layman's Max

    return treeTotalHPC;
}


void ProfilerReportNode::Sort( const std::string& sortBy, bool orderDesc ) {
    if( StringICmp( sortBy, "Total Time" ) ) {
        if( orderDesc ) {
            std::sort( children.begin(), children.end(), SortTotalDesc() );
        } else {
            std::sort( children.begin(), children.end(), SortTotalAsc() );
        }
    } else if( StringICmp( sortBy, "Self Time" ) ) {
        if( orderDesc ) {
            std::sort( children.begin(), children.end(), SortSelfDesc() );
        } else {
            std::sort( children.begin(), children.end(), SortSelfAsc() );
        }
    }

    int numChildren = (int)children.size();

    for( int childIndex = 0; childIndex < numChildren; childIndex++ ) {
        children[childIndex]->Sort( sortBy, orderDesc );
    }
}


bool ProfilerReportNode::SortTotalDesc::operator()( const ProfilerReportNode* nodeA, const ProfilerReportNode* nodeB ) {
    return (nodeA->totalHPC > nodeB->totalHPC);
}


bool ProfilerReportNode::SortTotalAsc::operator()( const ProfilerReportNode* nodeA, const ProfilerReportNode* nodeB ) {
    return (nodeA->totalHPC < nodeB->totalHPC);
}


bool ProfilerReportNode::SortSelfDesc::operator()( const ProfilerReportNode* nodeA, const ProfilerReportNode* nodeB ) {
    return (nodeA->selfHPC > nodeB->selfHPC);
}


bool ProfilerReportNode::SortSelfAsc::operator()( const ProfilerReportNode* nodeA, const ProfilerReportNode* nodeB ) {
    return (nodeA->selfHPC < nodeB->selfHPC);
}
