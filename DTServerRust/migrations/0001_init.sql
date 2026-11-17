-- History of the digital twin server (plan section 6.1).

-- Registry: twin id -> project and topic base name. Survives restarts, so values can be recorded
-- before the first connectToTwin after a restart.
CREATE TABLE twin (
    twin_id     uuid        PRIMARY KEY,
    project_id  uuid        NOT NULL,
    name        text        NOT NULL,
    first_seen  timestamptz NOT NULL DEFAULT now(),
    last_seen   timestamptz NOT NULL DEFAULT now()
);

-- A topic name belongs to exactly one twin (plan 6.3: the first registered twin keeps the name).
CREATE UNIQUE INDEX twin_name_key ON twin (name);

-- value_type: 1 Real, 2 Integer, 3 Boolean, 4 Rational, 5 Complex
-- value_num:  Real, Integer (as double precision), Boolean (0 / 1)
-- value_text: Rational ("n/d"); Integer whose magnitude exceeds 2^53 (exact decimal digits)
-- value_json: Complex ({"imaginaryValue": x, "realValue": y})
--
-- Range partitioned by month on ts. There is deliberately no primary key or unique constraint: PostgreSQL
-- requires the partition key in every unique constraint of a partitioned table, and a time series has no natural
-- unique key. There is no DEFAULT partition either: a default partition would block CREATE TABLE ... PARTITION OF
-- for a month as soon as it contains a row of that month. Partitions are created by the server (current month,
-- next month, and on demand for rows within the accepted time window).
CREATE TABLE measurement (
    twin_id      uuid             NOT NULL,
    project_id   uuid             NOT NULL,
    element_path text             NOT NULL,
    ts           timestamptz      NOT NULL,
    received_at  timestamptz      NOT NULL,
    value_type   smallint         NOT NULL,
    value_num    double precision,
    value_text   text,
    value_json   jsonb
) PARTITION BY RANGE (ts);

-- On the partitioned table: PostgreSQL creates the matching index on every existing and future partition.
CREATE INDEX measurement_twin_element_ts_idx ON measurement (twin_id, element_path, ts DESC);
