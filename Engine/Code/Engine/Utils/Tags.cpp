#include "Engine/Utils/Tags.hpp"


Strings Tags::GetTags() const {
    return m_tags;
}


void Tags::SetTags( std::string tagsCSV ) {
    Strings splitTags = SplitStringOnDelimeter( tagsCSV, ',', false );
    SetTags( splitTags );
}


void Tags::SetTags( const Strings& tagsVec ) {
    Strings::const_iterator newTagsIter = tagsVec.begin();

    for( newTagsIter; newTagsIter != tagsVec.end(); newTagsIter++ ) {
        if( (*newTagsIter)[0] == '!' ) {
            ClearTags( *newTagsIter );
            continue;
        }


        Strings::const_iterator oldTagsIter = m_tags.begin();
        bool tagFound = false;

        // Don't add multiple versions of the same tag
        for( oldTagsIter; oldTagsIter != m_tags.end(); oldTagsIter++ ) {
            if( StringICmp( *newTagsIter, *oldTagsIter ) ) {
                tagFound = true;
                break;
            }
        }

        if( !tagFound ) {
            m_tags.push_back( *newTagsIter );
        }
    }
}


void Tags::ClearTags( std::string tagsCSV ) {
    Strings splitTags = SplitStringOnDelimeter( tagsCSV, ',' );
    Strings::const_iterator newTagsIter = splitTags.begin();

    for( newTagsIter; newTagsIter != splitTags.end(); newTagsIter++ ) {
        if( (*newTagsIter)[0] == '!' ) {
            SetTags( *newTagsIter );
            continue;
        }


        Strings::const_iterator oldTagsIter = m_tags.begin();

        for( oldTagsIter; oldTagsIter != m_tags.end(); oldTagsIter++ ) {
            if( StringICmp( *newTagsIter, *oldTagsIter ) ) {
                m_tags.erase( oldTagsIter );
                break;
            }
        }
    }
}


bool Tags::HasTags( std::string tagsCSV ) const {
    Strings splitTags = SplitStringOnDelimeter( tagsCSV, ',' );
    return HasTags( splitTags );
}


bool Tags::HasTags( const Strings& tagsVec ) const {
    Strings::const_iterator newTagsIter = tagsVec.begin();
    bool hasAllTags = true;

    for( newTagsIter; newTagsIter != tagsVec.end(); newTagsIter++ ) {
        // Default changes based on set/cleared tag is desired
        bool hasTag = (*newTagsIter)[0] == '!';

        Strings::const_iterator oldTagsIter = m_tags.begin();

        for( oldTagsIter; oldTagsIter != m_tags.end(); oldTagsIter++ ) {
            if( StringICmp( *newTagsIter, *oldTagsIter ) ) {
                hasTag = !hasTag;
                break;
            }
        }

        hasAllTags &= hasTag;
    }

    return hasAllTags;
}


bool Tags::HasTags( const Tags& tagsToMatch ) const {
    int numTagsToMatch = (int)tagsToMatch.m_tags.size();

    for( int tagIndex = 0; tagIndex < numTagsToMatch; tagIndex++ ) {
        const std::string& tag = tagsToMatch.m_tags[tagIndex];
        bool matched = HasTags( tag );

        if( !matched ) {
            return false;
        }
    }

    return true;
}


bool Tags::HasAtLeastOneTag( const Tags& tagsToMatch ) const {
    int numTagsToMatch = (int)tagsToMatch.m_tags.size();

    for( int tagIndex = 0; tagIndex < numTagsToMatch; tagIndex++ ) {
        const std::string& tag = tagsToMatch.m_tags[tagIndex];
        bool matched = HasTags( tag );

        if( matched ) {
            return true;
        }
    }

    return false;
}
