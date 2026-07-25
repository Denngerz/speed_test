# Speed Test

A command-line internet speed test written in C. Measures download and upload
speed against speed test servers, detects the user's location, and selects the
best available server based on connection latency.

## Features

- Download speed test
- Upload speed test
- User location detection (by country)
- Automatic selection of the fastest reachable server for the user's country
- Full automated test, or each action run individually
- Results reported in megabits per second (Mbps)

## Dependencies

- [libcurl](https://curl.se/libcurl/) — HTTP transfers
- [cJSON](https://github.com/DaveGamble/cJSON) — JSON parsing
- A C compiler (gcc) and `make`

### Installing dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential libcurl4-openssl-dev libcjson-dev
```

## Building

```bash
make
```

This compiles the sources into `build/speed_tester`. To remove build artifacts:

```bash
make clean
```

## Usage

```bash
./build/speed_tester [options]
```

### Options

| Flag        | Description                                                              |
|-------------|--------------------------------------------------------------------------|
| `-a`        | Run the full automated test (location → best server → download → upload) |
| `-l`        | Detect and print the user's location                                     |
| `-d`        | Run a download speed test (requires `-s`)                                |
| `-u`        | Run an upload speed test (requires `-s`)                                 |
| `-s <host>` | Specify the server host to test against                                  |

### Examples

Full automated test:

```bash
./build/speed_tester -a
```

Detect location only:

```bash
./build/speed_tester -l
```

Download test against a specific server:

```bash
./build/speed_tester -d -s speedtest.kis.lt:8080 (example)
```

Upload test against a specific server:

```bash
./build/speed_tester -u -s speedtest.kis.lt:8080 (example)
```

## How it works

The program reads a list of speed test servers from
`data/speedtest_server_list.json`. In full-test mode it detects the user's
country via a public IP geolocation API, then selects the server in that country
with the lowest connection latency, and runs the download and upload tests
against it.

Download and upload speeds are measured over HTTP using libcurl. Each test is
capped at 15 seconds. Speeds are reported in megabits per second.

## Project structure
include/ Header files (module interfaces)
src/ Source files (module implementations)
data/ Server list JSON
build/ Compiled output (generated)
Makefile
README.md

| Module          | Responsibility                        |
|-----------------|---------------------------------------|
| `server_list`   | Loads and parses the server list JSON |
| `location`      | Detects the user's country            |
| `speedtest`     | Download and upload speed measurement |
| `server_select` | Selects the best server for a country |
| `http_utils`    | Shared HTTP helpers and callbacks     |