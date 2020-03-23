#include "Engine/Math/RNG.hpp"

#include "Engine/Core/Time.hpp"
#include "Engine/Debug/UnitTests.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/RawNoise.hpp"
#include "Engine/Math/Vec2.hpp"


unsigned int RNG::s_defaultSeed = 0;

RNG::RNG() {
    m_seed = s_defaultSeed++;
};


RNG::RNG( unsigned int seed ) :
    m_seed(seed) {
}


unsigned int RNG::GetSeed() const {
    return m_seed;
}


unsigned int RNG::GetPosition() const {
    return m_position;
}


void RNG::SetSeed( unsigned int seed ) {
    m_seed = seed;
}


void RNG::SetRandomSeed() {
    float currentSeconds = (float)GetCurrentSystemTimeSeconds();
    SetSeed( (unsigned int)currentSeconds );
}


void RNG::SetPosition( unsigned int position ) {
    m_position = position;
}


unsigned int RNG::GetRandomSeed() {
    return Get1dNoiseUint( m_position++, m_seed );
}


int RNG::GetRandomIntLessThan( int maxNotInclusive ) {
    unsigned int random = Get1dNoiseUint( m_position++, m_seed );
    return random % maxNotInclusive;
}


int RNG::GetRandomIntInRange( int minInclusive, int maxInclusive ) {
    if( minInclusive == maxInclusive ) {
        return minInclusive;
    }

    int range = maxInclusive - minInclusive + 1;
    unsigned int random = Get1dNoiseUint( m_position++, m_seed );

    return (random % range) + minInclusive;
}


int RNG::GetRandomIntInRange( IntRange rangeInclusive ) {
    return GetRandomIntInRange( rangeInclusive.min, rangeInclusive.max );
}


float RNG::GetRandomFloatZeroToOne() {
    return Get1dNoiseZeroToOne( m_position++, m_seed );
}


float RNG::GetRandomFloatInRange( float minInclusive, float maxInclusive ) {
    if( minInclusive == maxInclusive ) {
        return minInclusive;
    }

    float range = maxInclusive - minInclusive;
    return (GetRandomFloatZeroToOne() * range) + minInclusive;
}


float RNG::GetRandomFloatInRange( FloatRange rangeInclusive ) {
    return GetRandomFloatInRange( rangeInclusive.min, rangeInclusive.max );
}


Vec2 RNG::GetRandomPointInBounds( const AABB2& bounds, float radius /*= 0.f */ ) {
    float minX = bounds.mins.x + radius;
    float maxX = bounds.maxs.x - radius;

    float minY = bounds.mins.y + radius;
    float maxY = bounds.maxs.y - radius;

    float randomX = GetRandomFloatInRange( minX, maxX );
    float randomY = GetRandomFloatInRange( minY, maxY );

    return Vec2( randomX, randomY );
}


bool RNG::PercentChance( float fractionChance ) {
    if( fractionChance >= 1.f ) {
        return true;
    } else if( fractionChance <= 0.f ) {
        return false;
    }

    float chanceRoll = GetRandomFloatZeroToOne();
    return (chanceRoll <= fractionChance);
}


// UnitTests ----------------------------------------------------------
static void CountIntRngResult( int randomInt, int arraySize, int* resultsArray, int minLegal, int maxLegal ) {
    if( randomInt >= minLegal && randomInt <= maxLegal ) {
        // Result in legal range; increment count in array
        ++resultsArray[randomInt];
    } else {
        ++resultsArray[arraySize - 1]; // N-1 spot in array is reserved for illegal "out of bounds" count
        static int timesSeenThisMessage = 0;
        if( timesSeenThisMessage < 3 ) {
            printf( "\n  ERROR: An RNG function returned random integer %i - outside legal range [%i,%i]!", randomInt, minLegal, maxLegal );
            ++timesSeenThisMessage;
        }
    }
}


static void VerifyAndReportIntRngTest( int arraySize, int* resultsArray, int minLegal, int maxLegal, int numRolls, UnitTest* theTest ) {
    int numValidResults = 1 + (maxLegal - minLegal);
    int averageCountPerResult = numRolls / numValidResults;
    int maxCountDeviation = averageCountPerResult / 5; // Allow up to +/- 20% variation on expectation
    int illegalResultCount = resultsArray[arraySize - 1];

    // Check if there were any illegal (out-of-range) results, e.g. returning 3 or 9 if we want a number in [4,7]
    UnitTest::VerifyResult( illegalResultCount == 0, "RNG function returned one or more illegal (out-of-range) numbers!", theTest );
    if( illegalResultCount > 0 ) {
        float illegalResultPercent = 100.f * static_cast<float>(illegalResultCount) / static_cast<float>(numRolls);
        printf( "\n  Out of %i rolls, %i (%.0f%%) were outside the legal range [%i,%i] the RNG should have provided", numRolls, illegalResultCount, illegalResultPercent, minLegal, maxLegal );
    }

    // Check the result counts to see if all expected results were [roughly] evenly distributed
    int worstCountDeviation = 0;
    int worstCountDeviationIndex = 0;
    for( int resultIndex = minLegal; resultIndex <= maxLegal; ++resultIndex ) {
        int resultCount = resultsArray[resultIndex];
        int countDeviation = abs( resultCount - averageCountPerResult );
        if( countDeviation > worstCountDeviation ) {
            worstCountDeviation = countDeviation;
            worstCountDeviationIndex = resultIndex;
        }
    }

    UnitTest::VerifyResult( worstCountDeviation <= maxCountDeviation, "RNG function returned a grossly uneven random distribution of numbers!", theTest );
    if( worstCountDeviation > maxCountDeviation ) {
        int worstCount = resultsArray[worstCountDeviationIndex];
        float worstDeviationPercent = 100.f * static_cast<float>(worstCountDeviation) / static_cast<float>(averageCountPerResult);
        printf( "\n  Expected %i rolls per possible result (in [%i,%i]); result [%i] got %i rolls (%.0f%% deviation)", averageCountPerResult, minLegal, maxLegal, worstCountDeviationIndex, worstCount, worstDeviationPercent );
    }
}


UNITTEST( "Basics", "RNG", 0 ) {
    RNG rng;

    // Test result buckets for GetRandomIntLessThan and GetRandomIntInRange
    constexpr int RESULTS_ARRAY_SIZE = 500;
    int resultsIntLessThan7[RESULTS_ARRAY_SIZE]   = {}; // A random roll of "4" increments count in slot[4]; slot[N-1] is # of illegal results
    int resultsIntLessThan300[RESULTS_ARRAY_SIZE] = {}; // A random roll of "4" increments count in slot[4]; slot[N-1] is # of illegal results
    int resultsIntIn2Thru5[RESULTS_ARRAY_SIZE]    = {}; // A random roll of "4" increments count in slot[4]; slot[N-1] is # of illegal results
    int resultsIntIn6Thru8[RESULTS_ARRAY_SIZE]    = {}; // A random roll of "4" increments count in slot[4]; slot[N-1] is # of illegal results

    constexpr int NUM_RANDOM_ROLLS = 1'000'000;
    for( int i = 0; i < NUM_RANDOM_ROLLS; ++i ) {
        int randomIntLessThan7   = rng.GetRandomIntLessThan( 7 );
        int randomIntLessThan300 = rng.GetRandomIntLessThan( 300 );
        int randomIntIn2Thru5    = rng.GetRandomIntInRange( 2, 5 );
        int randomIntIn6Thru8    = rng.GetRandomIntInRange( 6, 8 );

        CountIntRngResult( randomIntLessThan7,   RESULTS_ARRAY_SIZE, resultsIntLessThan7,   0, 6 );
        CountIntRngResult( randomIntLessThan300, RESULTS_ARRAY_SIZE, resultsIntLessThan300, 0, 299 );
        CountIntRngResult( randomIntIn2Thru5,    RESULTS_ARRAY_SIZE, resultsIntIn2Thru5,    2, 5 );
        CountIntRngResult( randomIntIn6Thru8,    RESULTS_ARRAY_SIZE, resultsIntIn6Thru8,    6, 8 );
    }

    // How many of each result do we expect, on average?
    constexpr int AVERAGE_COUNT_PER_RESULT_LESS_THAN_7   = NUM_RANDOM_ROLLS / 7;
    constexpr int AVERAGE_COUNT_PER_RESULT_LESS_THAN_300 = NUM_RANDOM_ROLLS / 300;
    constexpr int AVERAGE_COUNT_PER_RESULT_IN_2_THRU_5   = NUM_RANDOM_ROLLS / 4;
    constexpr int AVERAGE_COUNT_PER_RESULT_IN_6_THRU_8   = NUM_RANDOM_ROLLS / 3;

    // How much deviation (+/-) away from expectation will we tolerate for each result count in each category?
    constexpr int MAX_DEVIATION_COUNT_LESS_THAN_7   = AVERAGE_COUNT_PER_RESULT_LESS_THAN_7 / 5;
    constexpr int MAX_DEVIATION_COUNT_LESS_THAN_300 = AVERAGE_COUNT_PER_RESULT_LESS_THAN_300 / 5;
    constexpr int MAX_DEVIATION_COUNT_IN_2_THRU_5   = AVERAGE_COUNT_PER_RESULT_IN_2_THRU_5 / 5;
    constexpr int MAX_DEVIATION_COUNT_IN_6_THRU_8   = AVERAGE_COUNT_PER_RESULT_IN_6_THRU_8 / 5;

    VerifyAndReportIntRngTest( RESULTS_ARRAY_SIZE, resultsIntLessThan7,   0, 6,   NUM_RANDOM_ROLLS, theTest );
    VerifyAndReportIntRngTest( RESULTS_ARRAY_SIZE, resultsIntLessThan300, 0, 299, NUM_RANDOM_ROLLS, theTest );
    VerifyAndReportIntRngTest( RESULTS_ARRAY_SIZE, resultsIntIn2Thru5,    2, 5,   NUM_RANDOM_ROLLS, theTest );
    VerifyAndReportIntRngTest( RESULTS_ARRAY_SIZE, resultsIntIn6Thru8,    6, 8,   NUM_RANDOM_ROLLS, theTest );
}
