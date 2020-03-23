#include "Engine/Utils/BufferUtils.hpp"

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/Plane2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Vertex/VertexPCU.hpp"


// Helper Functions --------------------------------------------------------
bool IsPlatformBigEndian() {
    static constexpr int i = 0x01020304;
    static bool platformIsBigEndian = (((char*)&i)[0] == 0x01);

    return platformIsBigEndian;
}


void Reverse2BytesInPlace( void* ptrTo16BitWord ) {
	unsigned short u = *(unsigned short*) ptrTo16BitWord;
	*(unsigned short*) ptrTo16BitWord =	((u & 0x00ff) << 8)  |
										((u & 0xff00) >> 8);
}


void Reverse4BytesInPlace( void* ptrTo32BitDword ) {
	unsigned int u = *(unsigned int*) ptrTo32BitDword;
	*(unsigned int*) ptrTo32BitDword =	((u & 0x000000ff) << 24) |
										((u & 0x0000ff00) << 8)  |
										((u & 0x00ff0000) >> 8)  |
										((u & 0xff000000) >> 24);
}


void Reverse8BytesInPlace( void* ptrTo64BitQword ) {
	int64_t u = *(int64_t*) ptrTo64BitQword;
	*(int64_t*) ptrTo64BitQword =	((u & 0x00000000000000ff) << 56) |
									((u & 0x000000000000ff00) << 40) |
									((u & 0x0000000000ff0000) << 24) |
									((u & 0x00000000ff000000) << 8)  |
									((u & 0x000000ff00000000) >> 8)  |
									((u & 0x0000ff0000000000) >> 24) |
									((u & 0x00ff000000000000) >> 40) |
									((u & 0xff00000000000000) >> 56);
}


void ReverseBytesInPlacePtrSizeT( void* ptrToPtrOrSizeT ) {
    if constexpr( sizeof( void* ) == 8 ) {
        Reverse8BytesInPlace( ptrToPtrOrSizeT );
    } else {
        Reverse4BytesInPlace( ptrToPtrOrSizeT );
    }
}


// Reader ------------------------------------------------------------------
BufferReader::BufferReader( const unsigned char* bufferData, int bufferSize, BufferEndian endianMode /*= BufferEndian::NATIVE_ENDIAN */ ) :
    m_bufferStart( bufferData ),
    m_bufferSize( bufferSize ) {
    SetEndianMode( endianMode );
}


BufferReader::BufferReader( const Buffer& buffer, BufferEndian endianMode /*= NATIVE_ENDIAN */ ) :
    BufferReader( buffer.data(), (int)buffer.size(), endianMode ) {
}


BufferReader::~BufferReader() {

}


AABB2 BufferReader::ParseAABB2() {
    Vec2 mins = ParseVec2();
    Vec2 maxs = ParseVec2();

    return AABB2( mins, maxs );
}


bool BufferReader::ParseBool() {
    int numBytesDesired = 1;
    GuaranteeBytesRemaining( numBytesDesired );

    const unsigned char& c = *(unsigned char*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    if( (c != 0x00) && (c != 0x01) ) {
        std::string errMsg = Stringf( "(BufferReader) ERROR -- Invalid byte value (%u) parsed when looking for bool", c);
        ERROR_AND_DIE( errMsg );
    }

    return (c == 0x01);
}


char BufferReader::ParseChar() {
    int numBytesDesired = sizeof( char );
    GuaranteeBytesRemaining( numBytesDesired );

    const char& c = *(char*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    return c;
}


unsigned char BufferReader::ParseUChar() {
    int numBytesDesired = sizeof( unsigned char );
    GuaranteeBytesRemaining( numBytesDesired );

    const unsigned char& c = *(unsigned char*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    return c;
}


double BufferReader::ParseDouble() {
    int numBytesDesired = sizeof( double );
    GuaranteeBytesRemaining( numBytesDesired );

    double d = *(double*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    if( IsEndianOppositeNative() ) {
        Reverse8BytesInPlace( &d );
    }

    return d;
}


float BufferReader::ParseFloat() {
    int numBytesDesired = sizeof( float );
    GuaranteeBytesRemaining( numBytesDesired );

    float f = *(float*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    if( IsEndianOppositeNative() ) {
        Reverse4BytesInPlace( &f );
    }

    return f;
}


int BufferReader::ParseInt() {
    int numBytesDesired = sizeof( int );
    GuaranteeBytesRemaining( numBytesDesired );

    int i = *(int*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    if( IsEndianOppositeNative() ) {
        Reverse4BytesInPlace( &i );
    }

    return i;
}


unsigned int BufferReader::ParseUInt() {
    int numBytesDesired = sizeof( unsigned int );
    GuaranteeBytesRemaining( numBytesDesired );

    unsigned int i = *(unsigned int*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    if( IsEndianOppositeNative() ) {
        Reverse4BytesInPlace( &i );
    }

    return i;
}


uint16_t BufferReader::ParseUInt16() {
    int numBytesDesired = sizeof( uint16_t );
    GuaranteeBytesRemaining( numBytesDesired );

    uint16_t i = *(uint16_t*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    if( IsEndianOppositeNative() ) {
        Reverse2BytesInPlace( &i );
    }

    return i;
}


uint64_t BufferReader::ParseUInt64() {
    int numBytesDesired = sizeof( uint64_t );
    GuaranteeBytesRemaining( numBytesDesired );

    uint64_t i = *(uint64_t*)&(m_bufferStart[m_readOffset]);
    m_readOffset += numBytesDesired;

    if( IsEndianOppositeNative() ) {
        Reverse8BytesInPlace( &i );
    }

    return i;
}


IntVec2 BufferReader::ParseIntVec2() {
    int x = ParseInt();
    int y = ParseInt();

    return IntVec2( x, y );
}


IntVec3 BufferReader::ParseIntVec3() {
    int x = ParseInt();
    int y = ParseInt();
    int z = ParseInt();

    return IntVec3( x, y, z );
}


Plane2 BufferReader::ParsePlane2() {
    Vec2  normal    = ParseVec2();
    float distance  = ParseFloat();

    return Plane2( normal, distance );
}


Rgba BufferReader::ParseRgb() {
    unsigned char r = ParseUChar();
    unsigned char g = ParseUChar();
    unsigned char b = ParseUChar();

    return Rgba( r, g, b, 255 );
}


Rgba BufferReader::ParseRgba() {
    unsigned char r = ParseUChar();
    unsigned char g = ParseUChar();
    unsigned char b = ParseUChar();
    unsigned char a = ParseUChar();

    return Rgba( r, g, b, a );
}


std::string BufferReader::ParseStringNullTerminated() {
    std::string outText = "";

    while( true ) {
        char nextChar = ParseChar();

        if( nextChar == '\0' ) {
            break;
        }

        outText.push_back( nextChar );
    }

    return outText;
}


std::string BufferReader::ParseStringAfterLength() {
    std::string outText = "";
    unsigned int strLength = ParseUInt();
    outText.reserve( strLength );
    
    for( unsigned int charIndex = 0; charIndex < strLength; charIndex++ ) {
        char nextChar = ParseChar();
        outText.push_back( nextChar );
    }

    return outText;
}


Vec2 BufferReader::ParseVec2() {
    float x = ParseFloat();
    float y = ParseFloat();

    return Vec2( x, y );
}


Vec3 BufferReader::ParseVec3() {
    float x = ParseFloat();
    float y = ParseFloat();
    float z = ParseFloat();

    return Vec3( x, y, z );
}


VertexPCU BufferReader::ParseVertexPCU() {
    Vec3 position = ParseVec3();
    Rgba color = ParseRgba();
    Vec2 uv = ParseVec2();

    return VertexPCU( position, color, uv );
}


void BufferReader::SetEndianMode( BufferEndian endianMode ) {
    m_endianMode = endianMode;

    if( m_endianMode != NATIVE_ENDIAN ) {
        bool platformIsBig = IsPlatformBigEndian();
        m_isOppositeEndian = (platformIsBig ^ (m_endianMode == BIG_ENDIAN));
    } else {
        m_isOppositeEndian = false;
    }
}


BufferEndian BufferReader::GetEndianMode() const {
    return m_endianMode;
}


bool BufferReader::IsBigEndianMode() const {
    return (m_endianMode == BIG_ENDIAN || (m_endianMode == NATIVE_ENDIAN && IsPlatformBigEndian()));
}


bool BufferReader::IsEndianOppositeNative() const {
    return m_isOppositeEndian;
}


void BufferReader::SetTempEndianMode( BufferEndian tempEndianMode ) {
    m_originalEndianMode = m_endianMode;
    SetEndianMode( tempEndianMode );
}


void BufferReader::ClearTempEndianMode() {
    SetEndianMode( m_originalEndianMode );
}


unsigned int BufferReader::GetBufferOffset() const {
    return m_readOffset;
}


void BufferReader::SkipBytes( int numBytesToSkip ) {
    m_readOffset += numBytesToSkip;
}


void BufferReader::SetBufferOffset( unsigned int bufferOffset ) {
    m_readOffset = bufferOffset;
}


void BufferReader::GuaranteeBytesRemaining( size_t numBytesDesired ) {
    int numBytesRemaining = m_bufferSize - m_readOffset;

    if( numBytesDesired > numBytesRemaining ) {
        std::string errMsg = Stringf( "(BufferReader) ERROR -- Requested %d bytes.. only %d remaining!", numBytesDesired, numBytesRemaining );
        ERROR_AND_DIE( errMsg.c_str() );
    }
}


// Writer ------------------------------------------------------------------
BufferWriter::BufferWriter( Buffer& buffer, BufferEndian endianMode /*= NATIVE_ENDIAN */ ) :
    m_buffer( buffer ) {
    SetEndianMode( endianMode );
}


BufferWriter::~BufferWriter() {

}


void BufferWriter::AppendAABB2( const AABB2& aabb2 ) {
    AppendVec2( aabb2.mins );
    AppendVec2( aabb2.maxs );
}


void BufferWriter::AppendBool( bool b ) {
    unsigned char boolAsChar = (b ? 0x01 : 0x00);
    AppendUChar( boolAsChar );
}


void BufferWriter::AppendChar( char c ) {
    char* newCharPtr = (char*)AppendUninitializedBytes( sizeof( char ) );
    *newCharPtr = c;
}


void BufferWriter::AppendUChar( unsigned char c ) {
    unsigned char* newCharPtr = (unsigned char*)AppendUninitializedBytes( sizeof( unsigned char ) );
    *newCharPtr = c;
}


void BufferWriter::AppendDouble( double d ) {
    double* newDoublePtr = (double*)AppendUninitializedBytes( sizeof( double ) );
    *newDoublePtr = d;
}


void BufferWriter::AppendFloat( float f ) {
    float* newFloatPtr = (float*)AppendUninitializedBytes( sizeof( float ) );

    if( IsEndianOppositeNative() ) {
        Reverse4BytesInPlace( &f );
    }

    *newFloatPtr = f;
}


void BufferWriter::AppendInt( int i ) {
    int* newIntPtr = (int*)AppendUninitializedBytes( sizeof( int ) );

    if( IsEndianOppositeNative() ) {
        Reverse4BytesInPlace( &i );
    }

    *newIntPtr = i;
}


void BufferWriter::AppendUInt( unsigned int i ) {
    unsigned int* newIntPtr = (unsigned int*)AppendUninitializedBytes( sizeof( unsigned int ) );

    if( IsEndianOppositeNative() ) {
        Reverse4BytesInPlace( &i );
    }

    *newIntPtr = i;
}


void BufferWriter::AppendUInt16( uint16_t i ) {
    uint16_t* newIntPtr = (uint16_t*)AppendUninitializedBytes( sizeof( uint16_t ) );

    if( IsEndianOppositeNative() ) {
        Reverse2BytesInPlace( &i );
    }

    *newIntPtr = i;
}


void BufferWriter::AppendUInt64( uint64_t i ) {
    uint64_t* newIntPtr = (uint64_t*)AppendUninitializedBytes( sizeof( uint64_t ) );

    if( IsEndianOppositeNative() ) {
        Reverse8BytesInPlace( &i );
    }

    *newIntPtr = i;
}


void BufferWriter::AppendIntVec2( const IntVec2& vec ) {
    AppendInt( vec.x );
    AppendInt( vec.y );
}


void BufferWriter::AppendIntVec3( const IntVec3& vec ) {
    AppendInt( vec.x );
    AppendInt( vec.y );
    AppendInt( vec.z );
}


void BufferWriter::AppendPlane2( const Plane2& plane ) {
    AppendVec2( plane.normal );
    AppendFloat( plane.distance );
}


void BufferWriter::AppendRgb( const Rgba& color ) {
    unsigned char colorAsBytes[4];
    color.GetAsBytes( colorAsBytes );

    unsigned char* newCharPtr = AppendUninitializedBytes( 3 );
    newCharPtr[0] = colorAsBytes[0];
    newCharPtr[1] = colorAsBytes[1];
    newCharPtr[2] = colorAsBytes[2];
}


void BufferWriter::AppendRgba( const Rgba& color ) {
    unsigned char* newCharPtr = AppendUninitializedBytes( 4 );
    color.GetAsBytes( newCharPtr );
}


void BufferWriter::AppendStringNullTerminated( const std::string& text ) {
    int strLength = (int)text.size();

    for( int charIndex = 0; charIndex < strLength; charIndex++ ) {
        AppendChar( text[charIndex] );
    }

    AppendChar( '\0' );
}


void BufferWriter::AppendStringAfterLength( const std::string& text ) {
    unsigned int strLength = (unsigned int)text.size();
    AppendUInt( strLength );

    for( unsigned int charIndex = 0; charIndex < strLength; charIndex++ ) {
        AppendChar( text[charIndex] );
    }
}


void BufferWriter::AppendVec2( const Vec2& vec ) {
    AppendFloat( vec.x );
    AppendFloat( vec.y );
}


void BufferWriter::AppendVec3( const Vec3& vec ) {
    AppendFloat( vec.x );
    AppendFloat( vec.y );
    AppendFloat( vec.z );
}


void BufferWriter::AppendVertexPCU( const VertexPCU& vert ) {
    AppendVec3( vert.position );
    AppendRgba( vert.color );
    AppendVec2( vert.uvTexCoords );
}


void BufferWriter::OverwriteUIntAtOffset( unsigned int i, int bufferOffset ) {
    GUARANTEE_OR_DIE( bufferOffset > 0 && bufferOffset < m_buffer.size(), "(BufferWriter) ERROR -- Attempt to overwrite invalid offset" );
    
    unsigned int* startOfUInt = (unsigned int*)(m_buffer.data() + bufferOffset);

    if( IsEndianOppositeNative() ) {
        Reverse4BytesInPlace( &i );
    }

    *startOfUInt = i;
}


void BufferWriter::SetEndianMode( BufferEndian endianMode ) {
    m_endianMode = endianMode;

    if( m_endianMode != NATIVE_ENDIAN ) {
        bool platformIsBig = IsPlatformBigEndian();
        m_isOppositeEndian = (platformIsBig ^ (m_endianMode == BIG_ENDIAN));
    } else {
        m_isOppositeEndian = false;
    }
}


BufferEndian BufferWriter::GetEndianMode() const {
    return m_endianMode;
}


bool BufferWriter::IsBigEndianMode() const {
    return (m_endianMode == BIG_ENDIAN || (m_endianMode == NATIVE_ENDIAN && IsPlatformBigEndian()));
}


bool BufferWriter::IsEndianOppositeNative() const {
    return m_isOppositeEndian;
}


void BufferWriter::SetTempEndianMode( BufferEndian tempEndianMode ) {
    m_originalEndianMode = m_endianMode;
    SetEndianMode( tempEndianMode );
}


void BufferWriter::ClearTempEndianMode() {
    SetEndianMode( m_originalEndianMode );
}


unsigned int BufferWriter::GetBufferOffset() const {
    return (unsigned int)m_buffer.size();
}


unsigned char* BufferWriter::AppendUninitializedBytes( int numBytes ) {
    int initialSize = (int)m_buffer.size();
    m_buffer.resize( initialSize + numBytes );

    unsigned char* startOfNewBytes = m_buffer.data() + initialSize;
    return startOfNewBytes;
}
