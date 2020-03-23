#include "Engine/Async/WriteImageJob.hpp"

#include "Engine/Core/Image.hpp"


WriteImageJob::WriteImageJob( JobSystem* jobSystem, Image* imageToWrite, const std::filesystem::path& filePath, const std::string& eventCallback /*= "" */ ) :
    Job( jobSystem, JOB_CATEGORY_GENERIC, eventCallback ),
    m_imageToWrite( imageToWrite ),
    m_filePath( filePath ) {
}


WriteImageJob::~WriteImageJob() {
    CLEAR_POINTER( m_imageToWrite );
}


void WriteImageJob::Execute() {
    m_imageToWrite->WriteToPNG( m_filePath );
}
