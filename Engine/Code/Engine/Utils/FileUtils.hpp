#pragma once
#include "Engine/Core/EngineCommon.hpp"

// File Utils -- using file steams (deprecate these functions)
bool OpenFileForRead( const char* filename, std::ifstream& out_fileHandle );
bool OpenFileForWrite( const char* filename, std::ofstream& out_fileHandle );

bool ReadFromFile( const char* filename, char** out_buffer, size_t& out_size ); // May want a std::string version?
bool WriteToFile( const char* filename, const char* buffer, size_t size );
bool WriteToFile( std::ofstream& fileHandle, const char* buffer, size_t size );
bool WriteToFile( std::ofstream& fileHandle, const std::string& content );

// File Utils -- using C style fopen/fclose (migrate to these functions)
bool ReadFromFile( const char* fileName, Buffer& out_fileBuffer );
bool WriteToFile( const char* fileName, const Buffer& content );
