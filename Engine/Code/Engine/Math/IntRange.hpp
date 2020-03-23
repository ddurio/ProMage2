#pragma once
#include "Engine/Core/EngineCommon.hpp"


struct IntRange {
    public:
    int min = 0;
    int max = 0;

    explicit IntRange( int initialMin, int initialMax );
    explicit IntRange( const std::string& initialRange );
    ~IntRange() {};

    static const IntRange ZERO;
    static const IntRange ONE;
    static const IntRange NEGONE;

    bool operator==( const IntRange& compare ) const;
	bool operator!=( const IntRange& compare ) const;

    const std::string GetAsString() const;
    void SetFromText( const std::string& valuesAsTExt );

    bool IsIntInRange( int inValue ) const;
};
