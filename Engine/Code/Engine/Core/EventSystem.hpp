#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Utils/NamedProperties.hpp"

#include "map"
#include "shared_mutex"


class EventSubscription;

typedef NamedProperties EventArgs;
typedef bool( *EventCallbackFunction )(EventArgs& args);
typedef bool( *EventCallbackProxy )(const EventSubscription& eventSub, EventArgs& args);


class EventSubscription {
    friend class EventSystem;

    private:
    static const int MAX_POINTER_SIZE = 16;


    // Constructors
    explicit EventSubscription( EventCallbackFunction callback ); // C Functions

    template <typename T>
    EventSubscription( T *obj, bool (T::*methodPtr)( EventArgs& args ) ) { // Methods
        GUARANTEE_OR_DIE( obj != nullptr, "(EventSystem) ERROR -- Invalid object pointer for event subscription")
        m_callbackObject = obj;

        // Save method pointer
        // -- Generally 8 bytes large.. multiple inheritance creates 16 bytes instead
        size_t methodSize = sizeof( methodPtr );
        GUARANTEE_OR_DIE( methodSize <= MAX_POINTER_SIZE, Stringf( "(EventSystem): ERROR -- Larger pointer buffer needed (%d)", methodSize ) );

        memset( m_callbackFunction, 0, MAX_POINTER_SIZE );
        memcpy( m_callbackFunction, (char*)&methodPtr, methodSize );
        m_callbackProxy = CallMethod<T, decltype(methodPtr)>;
    }


    // Methods
    bool operator==( const EventSubscription& otherSub ) const;
    bool operator()( EventArgs& args ) const;

    static bool CallCFunction( const EventSubscription& eventSub, EventArgs& args );

    template <typename OBJ_T, typename METHOD_PTR>
    static bool CallMethod( const EventSubscription& eventSub, EventArgs& args ) {
        METHOD_PTR methodPointer = *(METHOD_PTR*)&(eventSub.m_callbackFunction);
        OBJ_T* objectPointer = (OBJ_T*)(eventSub.m_callbackObject);

        return (objectPointer->*methodPointer)(args); // Call it
    }


    // Variables
    void* m_callbackObject = nullptr;
    char m_callbackFunction[MAX_POINTER_SIZE];
    EventCallbackProxy  m_callbackProxy = nullptr;
};





class EventSystem {
    typedef std::vector<EventSubscription*> SubscriptionList;

    public:
    EventSystem();

    void Startup();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    template <typename T>
    void Subscribe( const std::string& eventName, T *obj, bool (T::*methodPtr)(EventArgs& args) );

    template <typename T>
    void Unsubscribe( const std::string& eventName, T *obj, bool (T::*methodPtr)(EventArgs& args) );

    void Subscribe( const std::string& eventName, EventCallbackFunction callback );
    void Unsubscribe( const std::string& eventName, EventCallbackFunction callback );

    void SubscribeEventCallbackFunction( const std::string& eventName, EventCallbackFunction callback );
    void UnsubscribeEventCallbackFunction( const std::string& eventName, EventCallbackFunction callback );

    int FireEvent( const std::string& eventName );
    int FireEvent( const std::string& eventName, EventArgs& args );

    void GetSubscribedEventsList( Strings& eventsList ) const;

    private:
    mutable std::shared_mutex m_mutex;


    ~EventSystem(); // No game or engine code should actually delete this until the program shuts down

    void AddSubscription( const std::string& eventName, EventSubscription* sub );
    void RemoveSubscription( const std::string& eventName, const EventSubscription& unsub );

    std::map<std::string, SubscriptionList> m_subscriptionsByEvent;
};


template <typename T>
void EventSystem::Subscribe( const std::string& eventName, T* obj, bool (T::*methodPtr)(EventArgs& args) ) {
    EventSubscription* newSub = new EventSubscription( obj, methodPtr );
    AddSubscription( eventName, newSub );
}


template <typename T>
void EventSystem::Unsubscribe( const std::string& eventName, T *obj, bool (T::*methodPtr)(EventArgs& args) ) {
    EventSubscription unsub = EventSubscription( obj, methodPtr );
    RemoveSubscription( eventName, unsub );
}
