-- History of the digital twins, written by the C++ core (DigitalTwinStore) and read by the Rust REST API.
-- See Markdownfiles/Plan_Datenbank_CppKern.md.
--
-- This file is the single source of the schema: the store embeds and applies it, the Rust tests load it.
-- It contains plain statements only (no functions, no dollar quoting), each ended by a semicolon.
-- The monthly partitions of `measurement` are not created here; the store creates them.

CREATE TABLE project (
    project_id   uuid        PRIMARY KEY,
    name         text,
    first_seen   timestamptz NOT NULL DEFAULT now()
);

CREATE TABLE twin (
    twin_no      integer     GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    twin_id      uuid        NOT NULL UNIQUE,
    project_id   uuid        NOT NULL REFERENCES project,
    name         text        NOT NULL,
    first_seen   timestamptz NOT NULL DEFAULT now(),
    last_seen    timestamptz NOT NULL DEFAULT now()
);

-- One row per loaded commit of a twin.
CREATE TABLE model_version (
    version_no   integer     GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    twin_no      integer     NOT NULL REFERENCES twin,
    commit_id    uuid        NOT NULL,
    first_loaded timestamptz NOT NULL DEFAULT now(),
    last_loaded  timestamptz NOT NULL DEFAULT now(),
    UNIQUE (twin_no, commit_id)
);

-- The model tree of one commit.
-- kind:       1 Package, 2 Component, 3 Port, 4 Measurable, 5 Controllable, 6 Attribute
-- value_type: only for variables (kind >= 4): 1 Real, 2 Integer, 3 Boolean, 4 Rational, 5 Complex, 6 String
-- path:       topic path relative to the twin name, levels separated by '/'
CREATE TABLE element (
    element_no   integer     GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    version_no   integer     NOT NULL REFERENCES model_version,
    parent_no    integer     REFERENCES element,
    kind         smallint    NOT NULL,
    name         text        NOT NULL,
    path         text        NOT NULL,
    value_type   smallint,
    UNIQUE (version_no, path)
);

-- Header of a checkpoint. Its rows are in `measurement` (source 3, checkpoint_no set, received_at = taken_at).
-- reason:    1 load, 2 interval, 3 new partition, 4 after data loss, 5 shutdown
-- change_no: the change number of the checkpoint's rows; every row of the same version with a larger
--            change_no was written after the checkpoint.
CREATE TABLE checkpoint (
    checkpoint_no bigint     GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    version_no   integer     NOT NULL REFERENCES model_version,
    taken_at     timestamptz NOT NULL,
    change_no    bigint      NOT NULL,
    reason       smallint    NOT NULL,
    variables    integer     NOT NULL
);

CREATE INDEX checkpoint_version_taken_idx ON checkpoint (version_no, taken_at DESC);

-- Source of change_no. The single writer takes blocks of 1000 numbers: nextval returns the first number of
-- a block [n, n + 999].
CREATE SEQUENCE measurement_change_no_seq AS bigint INCREMENT BY 1000 START WITH 1000;

-- Changes and checkpoint rows. Range partitioned by month on received_at (arrival at the broker, UTC);
-- partitions are named measurement_yYYYYmMM.
--
-- change_no:   one number per update of the model; all rows written by one update share it. Later updates
--              have larger numbers. Checkpoint rows carry the number current at the time of the checkpoint.
-- ts:          time_point of the client payload that caused the row
-- source:      1 client, 2 derived by the model, 3 checkpoint
-- value_num:   Real, Integer (as double precision), Boolean (0 / 1)
-- value_text:  Rational ("n/d"), String, Integer whose magnitude exceeds 2^53 (exact decimal digits)
-- value_json:  Complex ({"imaginaryValue": x, "realValue": y})
--
-- No primary key, no foreign keys and no DEFAULT partition, so bulk inserts stay cheap and a partition can
-- always be created.
CREATE TABLE measurement (
    element_no    integer          NOT NULL,
    change_no     bigint           NOT NULL,
    received_at   timestamptz      NOT NULL,
    ts            timestamptz      NOT NULL,
    source        smallint         NOT NULL,
    checkpoint_no bigint,
    value_num     double precision,
    value_text    text,
    value_json    jsonb
) PARTITION BY RANGE (received_at);

CREATE INDEX measurement_element_received_idx ON measurement (element_no, received_at DESC, change_no DESC);
