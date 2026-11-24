-- Durable copy of the MQTT client sessions of the broker (src/broker/persist.rs).
--
-- The broker applies this file at every start and only creates what is missing. Table names are unqualified,
-- the schema is whatever the `search_path` of the connection selects. The tables are separate from the history
-- schema of the C++ core (DigitalTwinStore/sql/0001_schema.sql), and may live in the same database.
--
-- Only sessions that outlive their connection are stored (Session Expiry above 0, which is every MQTT 3.1.1
-- session with clean_session = 0). All times are Unix milliseconds, like in the broker's memory.

-- One row per session. `disconnected_at_ms` is NULL while a client is connected. A session that is still
-- marked as connected when the broker starts was cut off by a stop or a crash and is stamped with the start time.
-- The session ends `expiry_s` seconds after `disconnected_at_ms`.
CREATE TABLE IF NOT EXISTS mqtt_session (
    session_id         BIGINT      PRIMARY KEY,
    client_id          TEXT        NOT NULL UNIQUE,
    username           TEXT,
    protocol           SMALLINT    NOT NULL,   -- 4 = MQTT 3.1.1, 5 = MQTT 5
    expiry_s           BIGINT      NOT NULL,
    disconnected_at_ms BIGINT,
    created_at         TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- Subscriptions of a session with the granted QoS (0 to 2) and the MQTT 5 No Local option.
CREATE TABLE IF NOT EXISTS mqtt_subscription (
    session_id BIGINT   NOT NULL REFERENCES mqtt_session ON DELETE CASCADE,
    filter     TEXT     NOT NULL,
    qos        SMALLINT NOT NULL,
    no_local   BOOLEAN  NOT NULL,
    PRIMARY KEY (session_id, filter)
);

-- Messages queued for a session and not yet acknowledged by the subscriber, in the order of `seq`.
-- `pubrec_received`: QoS 2 only, the PUBREC arrived, so the PUBLISH must not be sent again.
-- A packet identifier belongs to one message of the session at a time, hence the unique key.
-- `origin`: 0 = published by a client, 1 = published by the server itself.
CREATE TABLE IF NOT EXISTS mqtt_outbound (
    session_id      BIGINT   NOT NULL REFERENCES mqtt_session ON DELETE CASCADE,
    seq             BIGINT   NOT NULL,
    packet_id       INTEGER  NOT NULL,
    qos             SMALLINT NOT NULL,
    pubrec_received BOOLEAN  NOT NULL DEFAULT false,
    origin          SMALLINT NOT NULL,
    topic           TEXT     NOT NULL,
    payload         BYTEA    NOT NULL,
    received_at_ms  BIGINT   NOT NULL,
    PRIMARY KEY (session_id, seq),
    UNIQUE (session_id, packet_id)
);

-- Packet identifiers of QoS 2 PUBLISH packets received from the client that wait for their PUBREL.
CREATE TABLE IF NOT EXISTS mqtt_inbound_qos2 (
    session_id BIGINT  NOT NULL REFERENCES mqtt_session ON DELETE CASCADE,
    packet_id  INTEGER NOT NULL,
    PRIMARY KEY (session_id, packet_id)
);
