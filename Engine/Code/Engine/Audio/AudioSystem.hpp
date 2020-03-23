#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
#include "ThirdParty/fmod/fmod.hpp"
#include <string>
#include <vector>
#include <map>


//-----------------------------------------------------------------------------------------------
typedef size_t ChannelID;
typedef size_t SoundID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t)(-1); // for bad SoundIDs and SoundPlaybackIDs
constexpr size_t AUDIO_MISSING_ID = (size_t)(-1); // for bad IDs in general


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
    virtual ChannelID           GetOrCreateChannel( const std::string& channelName );

    virtual SoundPlaybackID		PlaySound( SoundID soundID, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
    virtual SoundPlaybackID     PlaySoundAt( SoundID soundID, const Vec3& position, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
    virtual void				StopSound( SoundPlaybackID soundPlaybackID );

    virtual void				SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance );	// balance is in [-1,1], where 0 is L/R centered
    virtual void                SetSoundPlaybackChannel( SoundPlaybackID soundPlaybackID, ChannelID channelID );
    virtual void                SetSoundPlaybackPaused( SoundPlaybackID soundPlaybackID, bool isPaused );
    virtual void				SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed );		// speed is frequency multiplier (1.0 == normal)
    virtual void				SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume );	// volume is in [0,1]
    virtual void                SetListenerAttributes( const Vec3& position, const Vec3& forward, const Vec3& up, int listenerID = 0 );

    virtual void                SetChannelVolume( ChannelID channelID, float volume );
    virtual void                SetChannelPaused( ChannelID channelID, bool isPaused );
    virtual void                SetChannelChild( ChannelID parentChannelID, ChannelID childChannelID );

    virtual bool                IsSoundPlaying( SoundPlaybackID soundPlaybackID ) const;


    protected:
    FMOD::System*						m_fmodSystem;
    std::map< std::string, SoundID >	m_registeredSoundIDs;
    std::vector< FMOD::Sound* >			m_registeredSounds;

    std::map< std::string, ChannelID >  m_registeredChannelIDs;
    std::vector< FMOD::ChannelGroup* >  m_registeredChannels;

    static DevConsoleChannel s_audioChannel;


    virtual void				ValidateResult( FMOD_RESULT result ) const;
};

