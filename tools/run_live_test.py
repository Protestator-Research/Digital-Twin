#!/usr/bin/env python3
"""Runs the live integration tests (DigitalTwinLibTests: LiveBackend, DigitalTwinClientTests: MarkdownLive) against a StructuraSystemsServer test instance.

It uses the helpers of the server repository (tests/integration/conftest.py) to start a server on a throw-away database
(structura_systems_test_dt) and drops the database afterwards. Run it with the interpreter of the server tests:

    <server repo>/tests/.venv/bin/python tools/run_live_test.py [port]

Environment: STRUCTURA_SERVER_REPO (default ../StructuraSystemsServer).
The Mongo credentials are read from MONGO_ROOT_USERNAME / MONGO_ROOT_PASSWORD of the .env of the server repository (never printed).
"""
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parents[1]
SERVER_REPO = Path(os.environ.get("STRUCTURA_SERVER_REPO", HERE.parent / "StructuraSystemsServer"))
OUT = HERE / "build" / "Debug" / "out"
TEST_BINARIES = [(OUT / "DigitalTwinLibTests", "LiveBackend.*"), (OUT / "DigitalTwinClientTests", "MarkdownLive.*")]
DB_NAME = "structura_systems_test_dt"


def read_env(path):
    values = {}
    for line in path.read_text().splitlines():
        if "=" in line and not line.lstrip().startswith("#"):
            key, _, value = line.partition("=")
            values[key.strip()] = value.strip().strip('"').strip("'")
    return values


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 18090
    dotenv = read_env(SERVER_REPO / ".env")
    # conftest reads the credentials from the environment when it is imported. They are put there only for the import
    # and removed again, so that neither the test binaries nor any other child process inherits them; only the server
    # (via its config file, written by conftest) gets them.
    os.environ["TEST_MONGO_USER"] = dotenv["MONGO_ROOT_USERNAME"]
    os.environ["TEST_MONGO_PASSWORD"] = dotenv["MONGO_ROOT_PASSWORD"]
    sys.path.insert(0, str(SERVER_REPO / "tests" / "integration"))
    try:
        import conftest  # noqa: E402
    finally:
        del os.environ["TEST_MONGO_USER"]
        del os.environ["TEST_MONGO_PASSWORD"]
    from pymongo import MongoClient

    assert DB_NAME.startswith("structura_systems_test_")
    limits = {"login_max_requests_per_ip": 100000, "login_max_failures_per_user": 1000}
    result = 1
    try:
        with conftest.start_server(DB_NAME, port, custom_config=limits):
            env = dict(os.environ)
            env["DT_TEST_SERVER_URL"] = f"http://127.0.0.1:{port}"
            env["DT_TEST_USER"] = "admin"
            env["DT_TEST_PASSWORD"] = conftest.ADMIN_PASSWORD
            # The test binary needs the Conan run environment of the Digital Twin build.
            run_env = HERE / "build" / "Debug" / "generators" / "conanrun.sh"
            result = 0
            for binary, suite in TEST_BINARIES:
                command = ["bash", "-c", 'source "$1" && exec "$2" --gtest_filter="$3"', "bash", str(run_env), str(binary), suite]
                result = subprocess.call(command, env=env, cwd=str(HERE)) or result
    finally:
        client = MongoClient(conftest.MONGO_URI, serverSelectionTimeoutMS=5000)
        client.drop_database(DB_NAME)
        client.close()
    return result


if __name__ == "__main__":
    sys.exit(main())
