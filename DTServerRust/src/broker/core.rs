//! State shared by all sessions of one broker.

use std::collections::HashMap;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::{Arc, Mutex};

use super::limits::BrokerLimits;
use super::protocol::ProtocolMode;
use super::session::SessionOut;
use super::subscriptions::{SessionId, SubscriptionStore};
use super::tracker::ConnectionTracker;
use crate::auth::{Authenticator, LoginThrottle, ThrottleConfig};
use crate::hub::{InternalBus, Message};

pub(crate) struct Core {
    pub(crate) limits: BrokerLimits,
    /// Which MQTT versions clients may connect with.
    pub(crate) protocol: ProtocolMode,
    /// Checks the credentials of every CONNECT; `None` accepts all clients.
    pub(crate) authenticator: Option<Arc<dyn Authenticator>>,
    /// Throttling of failed logins and pending checks.
    pub(crate) throttle: Arc<LoginThrottle>,
    pub(crate) store: Mutex<SubscriptionStore>,
    pub(crate) sessions: Mutex<HashMap<SessionId, Arc<SessionOut>>>,
    pub(crate) tracker: Arc<ConnectionTracker>,
    pub(crate) internal: InternalBus,
    next_session_id: AtomicU64,
    closed: AtomicBool,
}

impl Core {
    #[cfg(test)]
    pub(crate) fn new(limits: BrokerLimits, protocol: ProtocolMode) -> Arc<Self> {
        Self::with_authenticator(limits, protocol, None, ThrottleConfig::default())
    }

    pub(crate) fn with_authenticator(
        limits: BrokerLimits,
        protocol: ProtocolMode,
        authenticator: Option<Arc<dyn Authenticator>>,
        throttle: ThrottleConfig,
    ) -> Arc<Self> {
        let tracker = ConnectionTracker::new(limits.max_connections, limits.max_connections_per_ip);
        Arc::new(Self {
            limits,
            protocol,
            authenticator,
            throttle: LoginThrottle::new(throttle),
            store: Mutex::new(SubscriptionStore::new()),
            sessions: Mutex::new(HashMap::new()),
            tracker,
            internal: InternalBus::default(),
            next_session_id: AtomicU64::new(1),
            closed: AtomicBool::new(false),
        })
    }

    pub(crate) fn next_session_id(&self) -> SessionId {
        self.next_session_id.fetch_add(1, Ordering::Relaxed)
    }

    /// Marks the broker as shut down and ends all internal subscriptions.
    pub(crate) fn close(&self) {
        self.closed.store(true, Ordering::SeqCst);
        self.internal.close();
    }

    pub(crate) fn is_closed(&self) -> bool {
        self.closed.load(Ordering::SeqCst)
    }

    /// Delivers a message to the internal subscribers and to every session with a matching subscription
    /// (once per session, the publishing session included unless its subscriptions use the MQTT 5 No Local
    /// option). `publisher` is the session the message came from, `None` for the server itself. Never blocks.
    pub(crate) fn dispatch(&self, message: Message, publisher: Option<SessionId>) {
        self.internal.offer(&message);
        let targets = self
            .store
            .lock()
            .unwrap()
            .matching_from(&message.topic, publisher);
        if targets.is_empty() {
            return;
        }
        let sessions = self.sessions.lock().unwrap();
        for id in targets {
            if let Some(out) = sessions.get(&id) {
                out.offer_publish(&message);
            }
        }
    }

    /// Removes a session from the registry and the subscription store.
    pub(crate) fn remove_session(&self, id: SessionId) {
        self.store.lock().unwrap().remove_all(id);
        self.sessions.lock().unwrap().remove(&id);
    }
}
