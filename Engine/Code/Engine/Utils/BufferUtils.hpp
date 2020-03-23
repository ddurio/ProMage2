#pragma once
#include "Engine/Core/EngineCommon.hpp"


enum BufferEndian {
    NATIVE_ENDIAN,
    LITTLE_ENDIAN,
    BIG_ENDIAN
};


struct AABB2;
struct IntVec2;
struct IntVec3;
struct Plane2;
struct Rgba;
struct Vec2;
struct Vec3;
struct VertexPCU;


bool IsPlatformBigEndian();
void Reverse2BytesInPlace( void* ptrTo16BitWord ); // for short
void Reverse4BytesInPlace( void* ptrTo32BitDword ); // for int, uint, float
void Reverse8BytesInPlace( void* ptrTo64BitQword ); // for double, uint64
void ReverseBytesInPlacePtrSizeT( void* ptrToPtrOrSizeT ); // For "pointer-sized" objects (e.g. size_t), either 4 or 8 bytes (in x86 and x64, respectively)


class BufferReader {
    public:
    BufferReader( const unsigned char* bufferData, int bufferSize, BufferEndian endianMode = NATIVE_ENDIAN );
    BufferReader( const Buffer& buffer, BufferEndian endianMode = NATIVE_ENDIAN );
    virtual ~BufferReader();

    // Parse Functions
    AABB2           ParseAABB2();
    bool            ParseBool();
    char            ParseChar();
    unsigned char   ParseUChar();
    double          ParseDouble();
    float           ParseFloat();
    int             ParseInt();
    unsigned int    ParseUInt();
    uint16_t        ParseUInt16();
    uint64_t        ParseUInt64();
    IntVec2         ParseIntVec2();
    IntVec3         ParseIntVec3();
    Plane2          ParsePlane2();
    Rgba            ParseRgb();
    Rgba            ParseRgba();
    std::string     ParseStringNullTerminated();
    std::string     ParseStringAfterLength();
    Vec2            ParseVec2();
    Vec3            ParseVec3();
    VertexPCU       ParseVertexPCU();

    // Endian Functions
    void SetEndianMode( BufferEndian endianMode );
    BufferEndian GetEndianMode() const;
    bool IsBigEndianMode() const;
    bool IsEndianOppositeNative() const;

    void SetTempEndianMode( BufferEndian tempEndianMode );
    void ClearTempEndianMode();

    // Helper Functions
    unsigned int GetBufferOffset() const;

    void SkipBytes( int numBytesToSkip );
    void SetBufferOffset( unsigned int bufferOffset );


    private:
    const unsigned char* m_bufferStart = nullptr;
    int m_bufferSize = 0;
    unsigned int m_readOffset = 0;

    BufferEndian m_endianMode = NATIVE_ENDIAN;
    BufferEndian m_originalEndianMode = NATIVE_ENDIAN; // Used for temporary endian changes
    bool m_isOppositeEndian = false;


    void GuaranteeBytesRemaining( size_t numBytesDesired );
};


class BufferWriter {
    public:
    BufferWriter( Buffer& buffer, BufferEndian endianMode = NATIVE_ENDIAN );
    virtual ~BufferWriter();

    void AppendAABB2( const AABB2& aabb2 );
    void AppendBool( bool b );
    void AppendChar( char c );
    void AppendUChar( unsigned char c );
    void AppendDouble( double d );
    void AppendFloat( float f );
    void AppendInt( int i );
    void AppendUInt( unsigned int i );
    void AppendUInt16( uint16_t i );
    void AppendUInt64( uint64_t i );
    void AppendIntVec2( const IntVec2& vec );
    void AppendIntVec3( const IntVec3& vec );
    void AppendPlane2( const Plane2& plane );
    void AppendRgb( const Rgba& color );
    void AppendRgba( const Rgba& color );
    void AppendStringNullTerminated( const std::string& text );
    void AppendStringAfterLength( const std::string& text );
    void AppendVec2( const Vec2& vec );
    void AppendVec3( const Vec3& vec );
    void AppendVertexPCU( const VertexPCU& vert );

    void OverwriteUIntAtOffset( unsigned int i, int bufferOffset );

    // Endian Functions
    void SetEndianMode( BufferEndian endianMode );
    BufferEndian GetEndianMode() const;
    bool IsBigEndianMode() const;
    bool IsEndianOppositeNative() const;

    void SetTempEndianMode( BufferEndian tempEndianMode );
    void ClearTempEndianMode();

    // Helper Functions
    unsigned int GetBufferOffset() const; // Returns location of next write


    private:
    Buffer& m_buffer;
    BufferEndian m_endianMode = NATIVE_ENDIAN;
    BufferEndian m_originalEndianMode = NATIVE_ENDIAN; // Used for temporary endian changes
    bool m_isOppositeEndian = false;


    unsigned char* AppendUninitializedBytes( int numBytes );
};
