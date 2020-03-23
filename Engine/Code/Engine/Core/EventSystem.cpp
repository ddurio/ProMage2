#include "Engine/Core/EventSystem.hpp"

#include "Engine/DevConsole/DevConsole.hpp"
#include "Engine/Utils/NamedStrings.hpp"


EventSystem* g_theEventSystem = new EventSystem();

EventSubscription::EventSubscription( EventCallbackFunction callback ) :
    m_callbackProxy(CallCFunction) {

    size_t callbackSize = sizeof( callback );
    memset( m_callbackFunction, 0, MAX_POINTER_SIZE );
    memcpy( m_callbackFunction, (char*)&callback, callbackSize );
}


bool EventSubscription::operator==( const EventSubscription& otherSub ) const {
    // Same if callback object and function are the same
    bool objectMatches = (m_callbackObject == otherSub.m_callbackObject);
    bool funcMatches = (memcmp( &m_callbackFunction, &(otherSub.m_callbackFunction), MAX_POINTER_SIZE ) == 0);
    return objectMatches && funcMatches;
}


bool EventSubscription::operator()( EventArgs& args ) const {
    return m_callbackProxy( *this, args );
}


bool EventSubscription::CallCFunction( const EventSubscription& eventSub, EventArgs& args ) {
    EventCallbackFunction callback = *(EventCallbackFunction*)&(eventSub.m_callbackFunction);
    return callback( args );
}


EventSystem::EventSystem() {
    
}


void EventSystem::Startup() {

}


void EventSystem::Shutdown() {

}


void EventSystem::BeginFrame() {

}


void EventSystem::EndFrame() {

}


void EventSystem::Subscribe( const std::string& eventName, EventCallbackFunction callback ) {
    SubscribeEventCallbackFunction( eventName, callback );
}


void EventSystem::Unsubscribe( const std::string& eventName, EventCallbackFunction callback ) {
    UnsubscribeEventCallbackFunction( eventName, callback );
}


void EventSystem::SubscribeEventCallbackFunction( const std::string& eventName, EventCallbackFunction callback ) {
    EventSubscription* sub = new EventSubscription( callback );
    AddSubscription( eventName, sub );
}


void EventSystem::UnsubscribeEventCallbackFunction( const std::string& eventName, EventCallbackFunction callback ) {
    EventSubscription unsub = EventSubscription( callback );
    RemoveSubscription( eventName, unsub );
}


int EventSystem::FireEvent( const std::string& eventName ) {
    EventArgs* emptyArgs = new EventArgs();
    return FireEvent( eventName, *emptyArgs );
}


int EventSystem::FireEvent( const std::string& eventName, EventArgs& args ) {
    std::string eventLower = StringToLower( eventName );

    m_mutex.lock_shared();
    SubscriptionList subs = m_subscriptionsByEvent[eventLower];
    m_mutex.unlock_shared();

    int numSubs = (int)subs.size(); // This seems dangerous since each sub can (un)sub callbacks during the loop
    int subIndex;

    for( subIndex = 0; subIndex < numSubs; subIndex++ ) {
        EventSubscription* sub = subs[subIndex];
        bool consumed = (*sub)( args );

        // No one else needs to be alerted
        if( consumed ) {
            subIndex++;
            break;
        }
    }

    return subIndex;
}


void EventSystem::GetSubscribedEventsList( Strings& eventsList ) const {
    std::map<std::string, SubscriptionList>::const_iterator listIter;
    std::shared_lock localLock( m_mutex );

    for( listIter = m_subscriptionsByEvent.begin(); listIter != m_subscriptionsByEvent.end(); listIter++ ) {
        const SubscriptionList& subList = listIter->second;
        int numSubs = (int)subList.size();

        if( numSubs > 0 ) {
            eventsList.push_back( listIter->first );
        }
    }
}


EventSystem::~EventSystem() {

}


void EventSystem::AddSubscription( const std::string& eventName, EventSubscription* sub ) {
    std::unique_lock localLock( m_mutex );

    std::string eventLower = StringToLower( eventName );
    SubscriptionList& subs = m_subscriptionsByEvent[eventLower];
    subs.push_back( sub );
}


void EventSystem::RemoveSubscription( const std::string& eventName, const EventSubscription& unsub ) {
    std::unique_lock localLock( m_mutex );

    std::string eventLower = StringToLower( eventName );
    SubscriptionList& subs = m_subscriptionsByEvent[eventLower];
    SubscriptionList::iterator subIter;

    for( subIter = subs.begin(); subIter != subs.end(); subIter++ ) {
        if( **subIter == unsub ) {
            subs.erase( subIter );
            return;
        }
    }

    g_theDevConsole->PrintString( "(EventSystem) WARNING -- No matching subscription was found to be removed.", DevConsole::CHANNEL_WARNING );
}
