#pragma once
#include "Engine/Core/EngineCommon.hpp"


class Tags {
    public:
    Strings GetTags() const;

    void SetTags( std::string tagsCSV );
    void SetTags( const Strings& tagsVec );

    void ClearTags( std::string tagsCSV );

    bool HasTags( std::string tagsCSV ) const;
    bool HasTags( const Strings& tagsVec ) const;
    bool HasTags( const Tags& tagsToMatch ) const;

    bool HasAtLeastOneTag( const Tags& tagsToMatch ) const;

    private:
    Strings m_tags;
};
