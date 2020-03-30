#pragma once
#include "Engine/Core/EngineCommon.hpp"

bool OpenFileForRead( const char* filename, std::ifstream& out_fileHandle );
bool OpenFileForWrite( const char* filename, std::ofstream& out_fileHandle );

bool ReadFromFile( const char* filename, char** out_buffer, size_t& out_size ); // May want a std::string version?
bool WriteToFile( const char* filename, const char* buffer, size_t size );
bool WriteToFile( std::ofstream& fileHandle, const char* buffer, size_t size );
bool WriteToFile( std::ofstream& fileHandle, const std::string& content );

