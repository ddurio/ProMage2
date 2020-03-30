#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
typedef size_t SoundID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t)(-1); // for bad SoundIDs and SoundPlaybackIDs


//-----------------------------------------------------------------------------------------------
struct Vec3;


class AudioSystem {
    public:
    AudioSystem();
    virtual ~AudioSystem();

    public:
    virtual void                Startup();
    virtual void                Shutdown();

    virtual void				BeginFrame();
    virtual void				EndFrame();

    virtual SoundID				GetOrCreateSound( const std::string& soundFilePath, bool is3D = false );
    virtual SoundPlaybackID		PlaySound( SoundID soundID, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
    virtual SoundPlaybackID     PlaySoundAt( SoundID soundID, const Vec3& position, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
    virtual void				StopSound( SoundPlaybackID soundPlaybackID );
    virtual void				SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume );	// volume is in [0,1]
    virtual void				SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance );	// balance is in [-1,1], where 0 is L/R centered
    virtual void				SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed );		// speed is frequency multiplier (1.0 == normal)

    virtual void                SetListenerAttributes( const Vec3& position, const Vec3& forward, const Vec3& up, int listenerID = 0 );

    virtual void				ValidateResult( FMOD_RESULT result );

    protected:
    FMOD::System*						m_fmodSystem;
    std::map< std::string, SoundID >	m_registeredSoundIDs;
    std::vector< FMOD::Sound* >			m_registeredSounds;

    static DevConsoleChannel s_audioChannel;
};

