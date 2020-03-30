#include "Engine/Math/AABB2.hpp"

#include "Engine/Math/MathUtils.hpp"

#include "algorithm"


const AABB2 AABB2::ZEROTOONE = AABB2( Vec2::ZERO, Vec2::ONE );
const AABB2 AABB2::UVZEROTOONE = AABB2( Vec2( 0.f, 1.f ), Vec2( 1.f, 0.f ) );

AABB2::AABB2( const AABB2& copyFrom ) {
    mins = copyFrom.mins;
    maxs = copyFrom.maxs;
}


AABB2::AABB2( Vec2 initialMins, Vec2 initialMaxs ) {
    mins = initialMins;
    maxs = initialMaxs;
}


AABB2::AABB2( const std::string& valuesAsText ) {
    SetFromText( valuesAsText );
}


AABB2::AABB2( float minX, float minY, float maxX, float maxY ) :
    AABB2( Vec2( minX, minY ), Vec2( maxX, maxY ) ) {
}


const AABB2 AABB2::operator+( const AABB2& aabb2ToAdd ) const {
    Vec2 tempMins = mins + aabb2ToAdd.mins;
    Vec2 tempMaxs = maxs + aabb2ToAdd.maxs;
    return AABB2( tempMins, tempMaxs );
}


const AABB2 AABB2::operator-( const AABB2& aabb2ToSubtract ) const {
    Vec2 tempMins = mins - aabb2ToSubtract.mins;
    Vec2 tempMaxs = maxs - aabb2ToSubtract.maxs;
    return AABB2( tempMins, tempMaxs );
}


const AABB2 AABB2::operator*( float uniformScale ) const {
    Vec2 tempMins = mins * uniformScale;
    Vec2 tempMaxs = maxs * uniformScale;
    return AABB2( tempMins, tempMaxs );
}


const AABB2 AABB2::operator/( float inverseScale ) const {
    Vec2 tempMins = mins / inverseScale;
    Vec2 tempMaxs = maxs / inverseScale;
    return AABB2( tempMins, tempMaxs );
}


const AABB2 AABB2::operator+( const Vec2& vecToAdd ) const {
    Vec2 tempMins = mins + vecToAdd;
    Vec2 tempMaxs = maxs + vecToAdd;
    return AABB2( tempMins, tempMaxs );
}


const AABB2 AABB2::operator-( const Vec2& vecToSubtract ) const {
    Vec2 tempMins = mins - vecToSubtract;
    Vec2 tempMaxs = maxs - vecToSubtract;
    return AABB2( tempMins, tempMaxs );
}


void AABB2::operator+=( const AABB2& aabb2ToAdd ) {
    mins += aabb2ToAdd.mins;
    maxs += aabb2ToAdd.maxs;
}


void AABB2::operator-=( const AABB2& aabb2ToSubtract ) {
    mins -= aabb2ToSubtract.mins;
    maxs -= aabb2ToSubtract.maxs;
}


void AABB2::operator+=( const Vec2& vecToAdd ) {
    mins += vecToAdd;
    maxs += vecToAdd;
}


void AABB2::operator-=( const Vec2& vecToSubtract ) {
    mins -= vecToSubtract;
    maxs -= vecToSubtract;
}


void AABB2::operator*=( const float uniformScale ) {
    mins *= uniformScale;
    maxs *= uniformScale;
}


void AABB2::operator/=( const float uniformDivisor ) {
    mins /= uniformDivisor;
    maxs /= uniformDivisor;
}


void AABB2::operator=( const AABB2& copyFrom ) {
    mins = copyFrom.mins;
    maxs = copyFrom.maxs;
}


bool AABB2::operator==( const AABB2& compare ) const {
    bool sameMins = (mins == compare.mins);
    bool sameMaxs = (mins == compare.maxs);
    return (sameMins && sameMaxs);
}


bool AABB2::operator!=( const AABB2& compare ) const {
    bool sameMins = (mins == compare.mins);
    bool sameMaxs = (mins == compare.maxs);
    return !(sameMins && sameMaxs);
}


const AABB2 operator*( float uniformScale, const AABB2& aabb2ToScale ) {
    Vec2 tempMins = aabb2ToScale.mins * uniformScale;
    Vec2 tempMaxs = aabb2ToScale.maxs * uniformScale;
    return(AABB2( tempMins, tempMaxs ));
}


const AABB2 operator+( const Vec2& vecToAdd, const AABB2& aabb2ToAdd ) {
    Vec2 tempMins = aabb2ToAdd.mins + vecToAdd;
    Vec2 tempMaxs = aabb2ToAdd.maxs + vecToAdd;
    return AABB2( tempMins, tempMaxs );
}


bool AABB2::IsPointInside( const Vec2& point ) const {
    Vec2 closestPoint = GetClosestPointOnAABB2( point );
    return (closestPoint == point);
}


std::string AABB2::GetAsString() const {
    std::string minsStr = mins.GetAsString();
    std::string maxsStr = maxs.GetAsString();
    std::string aabbStr = Stringf( "%s %s", minsStr.c_str(), maxsStr.c_str() );

    return aabbStr;
}


const Vec2 AABB2::GetClosestPointOnAABB2( const Vec2& point ) const {
    float closestX = Clamp( point.x, mins.x, maxs.x );
    float closestY = Clamp( point.y, mins.y, maxs.y );
    return Vec2( closestX, closestY );
}


const Vec2 AABB2::GetClosestPointOnAABB2Edge( const Vec2& point ) const {
    Vec2 closestPoint = GetClosestPointOnAABB2( point );

    if( closestPoint == point ) {
        // Point is inside.. special case
        float distanceToMinX = std::abs( mins.x - point.x );
        float distanceToMaxX = std::abs( maxs.x - point.x );
        float distanceToMinY = std::abs( mins.y - point.y );
        float distanceToMaxY = std::abs( maxs.y - point.y );

        float closestX = (distanceToMinX < distanceToMaxX) ? mins.x : maxs.x; // Equal defaults to right? ok I guess
        float closestY = (distanceToMinY < distanceToMaxY) ? mins.y : maxs.y; // Equal defaults to up? That's good!

        bool moveX = std::min( distanceToMinX, distanceToMaxX ) < std::min( distanceToMinY, distanceToMaxY );

        if( moveX ) {
            closestPoint = Vec2( closestX, point.y );
        } else {
            closestPoint = Vec2( point.x, closestY );
        }
    }

    return closestPoint;
}


const Vec2 AABB2::GetDimensions() const {
    float lenghtX = maxs.x - mins.x;
    float lenghtY = maxs.y - mins.y;
    return Vec2( lenghtX, lenghtY );
}


const Vec2 AABB2::GetCenter() const {
    Vec2 center = (mins + maxs) * 0.5f;
    return center;
}


float AABB2::GetAspectRatio() const {
    Vec2 dims = GetDimensions();
    float ratio = dims.x / dims.y;

    return ratio;
}


const AABB2 AABB2::GetPaddedAABB2( float padding ) const {
    Vec2 paddingVec = Vec2( padding, padding );
    Vec2 tempMins = mins - paddingVec;
    Vec2 tempMaxs = maxs + paddingVec;
    return AABB2( tempMins, tempMaxs );
}


const AABB2 AABB2::GetTranslated( const Vec2& translation ) const {
    AABB2 translated( mins, maxs );
    translated.Translate( translation );
    return translated;
}


void AABB2::GrowToIncludePoint( const Vec2& pointToInclude ) {
    if( !IsPointInside(pointToInclude) ) {
        if( pointToInclude.x < mins.x ) {
            mins.x = pointToInclude.x;
        } else if( pointToInclude.x > maxs.x ) {
            maxs.x = pointToInclude.x;
        }

        if( pointToInclude.y < mins.y ) {
            mins.y = pointToInclude.y;
        } else if( pointToInclude.y > maxs.y ) {
            maxs.y = pointToInclude.y;
        }
    }
}


void AABB2::Translate( const Vec2& translation ) {
    mins += translation;
    maxs += translation;
}


void AABB2::AlignWithinBox( const AABB2& box, const Vec2& alignment ) {
    Vec2 boxDimensions = box.GetDimensions();
    Vec2 myDimensions = GetDimensions();

    Vec2 spareSpace = boxDimensions - myDimensions;
    Vec2 localSpace = spareSpace * alignment; // position relative to bounding box
    mins = box.mins + localSpace;
    maxs = mins + myDimensions;
}


AABB2 AABB2::GetBoxWithin( const Vec2& boxDimensions, const Vec2& alignment ) const {
    Vec2 myDimensions = GetDimensions();

    Vec2 spareSpace = myDimensions - boxDimensions;
    Vec2 localSpace = spareSpace * alignment; // position relative to bounding box

    Vec2 boxMins = mins + localSpace;
    Vec2 boxMaxs = boxMins + boxDimensions;

    return AABB2( boxMins, boxMaxs );
}


Vec2 AABB2::GetPointWithin( const Vec2& alignment ) const {
    Vec2 myDimensions = GetDimensions();
    Vec2 localSpace = myDimensions * alignment;
    Vec2 point = mins + localSpace;
    return point;
}


Vec2 AABB2::GetPointAlignment( const Vec2& point ) const {
    Vec2 myDimensions = GetDimensions();
    Vec2 localSpace = point - mins;
    Vec2 alignment = Vec2::ZERO;

    if( myDimensions.x != 0 ) {
        alignment.x = localSpace.x / myDimensions.x;
    }

    if( myDimensions.y != 0 ) {
        alignment.y = localSpace.y / myDimensions.y;
    }

    return alignment;
}


AABB2 AABB2::CarveBoxOffBottom( float fractionOfHeight, float additionalHeight /*= 0.f */ ) {
    Vec2 myDimensions = GetDimensions();
    float boxWidth = myDimensions.x;
    float boxHeight = myDimensions.y * fractionOfHeight + additionalHeight;

    Vec2 boxDimensions = Vec2( boxWidth, boxHeight );
    Vec2 boxMins = mins;
    Vec2 boxMaxs = boxMins + boxDimensions;

    mins.y += boxHeight;

    return AABB2( boxMins, boxMaxs );
}


AABB2 AABB2::CarveBoxOffTop( float fractionOfHeight, float additionalHeight /*= 0.f */ ) {
    Vec2 myDimensions = GetDimensions();
    float boxWidth = myDimensions.x;
    float boxHeight = myDimensions.y * fractionOfHeight + additionalHeight;

    Vec2 boxDimensions = Vec2( boxWidth, boxHeight );
    Vec2 boxMaxs = maxs;
    Vec2 boxMins = boxMaxs - boxDimensions;

    maxs.y -= boxHeight;

    return AABB2( boxMins, boxMaxs );
}


void AABB2::SetFromText( const std::string& valuesAsText ) {
    Strings splitStr = SplitStringOnDelimeter( valuesAsText, ' ', false );
    int numParams = (int)splitStr.size();

    GUARANTEE_OR_DIE( numParams == 2, Stringf( "(AABB2) Invalid initialization string (%s)", valuesAsText.c_str() ) );

    mins = Vec2( splitStr[0] );
    maxs = Vec2( splitStr[1] );
}
