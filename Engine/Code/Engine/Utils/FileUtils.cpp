#include "Engine/Utils/FileUtils.hpp"

#include "filesystem"
#include "fstream"


// Deprecate
bool OpenFileForRead( const char* filename, std::ifstream& out_fileHandle ) {
    std::filesystem::path filePath( filename );
    filePath.remove_filename();

    // This makes no sense.. if the directories don't exist, there's no way the file does..
    if( std::filesystem::create_directories( filePath ) ) { // Will create directories if they don't exist
        ERROR_RECOVERABLE( Stringf( "(FileUtils) Failed to create directories for file %s", filename ) );
        return false;
    }

    out_fileHandle = std::ifstream( filename, std::ios::binary | std::ios::ate );

    if( !out_fileHandle.is_open() ) {
        ERROR_RECOVERABLE( Stringf( "(FileUtils) Failed to open file %s", filename ) );
        return false;
    }

    return true;
}


bool OpenFileForWrite( const char* filename, std::ofstream& out_fileHandle ) {
    std::filesystem::path filePath( filename );
    filePath.remove_filename();

    if( std::filesystem::create_directories( filePath ) ) { // Will create directories if they don't exist
        ERROR_RECOVERABLE( Stringf( "(FileUtils) Failed to create directories for file %s", filename ) );
        return false;
    }

    out_fileHandle = std::ofstream( filename, std::ios::binary | std::ios::ate );

    if( !out_fileHandle.is_open() ) {
        ERROR_RECOVERABLE( Stringf( "(FileUtils) Failed to open file %s", filename ) );
        return false;
    }

    return true;
}


bool ReadFromFile( const char* filename, char** out_buffer, size_t& out_size ) {
    std::ifstream fileHandle;

    if( !OpenFileForRead( filename, fileHandle ) ) {
        return false;
    }

    std::streamsize size = fileHandle.tellg();

    if( size <= 0 ) {
        ERROR_RECOVERABLE( Stringf( "(FileUtils) Failed to open file %s", filename ) );
        return false;
    }

    delete *out_buffer;
    *out_buffer = new char[size];

    fileHandle.seekg( 0, std::ios::beg );
    if( !fileHandle.read( *out_buffer, size ) ) {
        ERROR_RECOVERABLE( Stringf( "(FileUtils) Failed to read file %s", filename ) );
        return false;
    }

    fileHandle.close();

    out_size = size;
    return true;
}


bool WriteToFile( const char* filename, const char* buffer, size_t size ) {
    std::ofstream fileHandle;

    if( !OpenFileForWrite( filename, fileHandle ) ) {
        return false;
    }

    if( !fileHandle.write( buffer, size ) ) {
        ERROR_RECOVERABLE( Stringf( "(FileUtils) Failed to write file %s", filename ) );
        return false;
    }

    fileHandle.close();
    return true;
}


bool WriteToFile( std::ofstream& fileHandle, const char* buffer, size_t size ) {
    if( !fileHandle.write( buffer, size ) ) {
        ERROR_RECOVERABLE( "(FileUtils) Failed to write file handle" );
        return false;
    }

    return true;
}


bool WriteToFile( std::ofstream& fileHandle, const std::string& content ) {
    return WriteToFile( fileHandle, content.c_str(), content.size() );
}


// Migrate to these

// No guarantee the out buffer remains unchanged if this function fails
bool ReadFromFile( const char* fileName, Buffer& out_fileBuffer ) {
    FILE* fileHandle = nullptr;
    errno_t openResult = fopen_s( &fileHandle, fileName, "rb" );

    if( openResult != 0 || fileHandle == nullptr ) {
        return false;
    }

    fseek( fileHandle, 0, SEEK_END );
    long int numBytesInFile = ftell( fileHandle );
    rewind( fileHandle );

    out_fileBuffer.clear();
    out_fileBuffer.resize( numBytesInFile, 0x00 );

    size_t numBytesRead = fread( out_fileBuffer.data(), sizeof( unsigned char ), numBytesInFile, fileHandle );
    fclose( fileHandle );

    return (numBytesRead == numBytesInFile);
}


bool WriteToFile( const char* fileName, const Buffer& content ) {
    FILE* fileHandle = nullptr;
    errno_t openResult = fopen_s( &fileHandle, fileName, "wb" );

    if( openResult != 0 || fileHandle == nullptr ) {
        return false;
    }

    size_t numBytesInBuffer = content.size();
    size_t numBytesWritten = fwrite( content.data(), sizeof( unsigned char ), numBytesInBuffer, fileHandle );
    fclose( fileHandle );

    return (numBytesWritten == numBytesInBuffer);
}

