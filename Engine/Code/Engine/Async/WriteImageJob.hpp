#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Async/Job.hpp"

#include "filesystem"


class Image;


class WriteImageJob : public Job {
    public:
    WriteImageJob( JobSystem* jobSystem, Image* imageToWrite, const std::filesystem::path& filePath, const std::string& eventCallback = "" );
    ~WriteImageJob();


    private:
    Image* m_imageToWrite = nullptr;
    std::filesystem::path m_filePath = "";


    void Execute() override;
};
