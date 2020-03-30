#pragma once


struct AABB2;
struct FloatRange;
struct IntRange;
struct Vec2;


class RNG {
    public:
    explicit RNG();
    explicit RNG( unsigned int seed );

    unsigned int GetSeed() const;
    unsigned int GetPosition() const;

    void SetSeed( unsigned int seed );
    void SetPosition( unsigned int position );

    unsigned int GetRandomSeed();

    int GetRandomIntLessThan( int maxNotInclusive );
    int GetRandomIntInRange( int minInclusive, int maxInclusive );
    int GetRandomIntInRange( IntRange rangeInclusive );

    float GetRandomFloatZeroToOne();
    float GetRandomFloatInRange( float minInclusive, float maxInclusive );
    float GetRandomFloatInRange( FloatRange rangeInclusive );

    Vec2 GetRandomPointInBounds( const AABB2& bounds, float radius = 0.f );

    bool PercentChance( float fractionChance );

    protected:
    unsigned int m_seed = 0;
    unsigned int m_position = 0;

    static unsigned int s_defaultSeed;
};
