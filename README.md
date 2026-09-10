# Network Analyzer

A C++ packet analysis tool inspired by Wireshark, built on [PcapPlusPlus](https://pcapplusplus.github.io/). It parses `.pcap` files layer by layer (Ethernet → IP → TCP/UDP → application protocols), and includes a working pipeline that reconstructs playable audio (`.wav`) from RTP voice-call captures.

## What it does

- Reads `.pcap` files and dissects every packet through the full protocol stack: Ethernet, VLAN, IPv4/IPv6, ICMP, TCP, UDP, MPLS, PPPoE, HTTP, DNS, ARP, and RTP
- Detects and works around a real gap in PcapPlusPlus's RTP auto-detection heuristic (see [Notable Engineering Problem](#notable-engineering-problem-silent-rtp-detection-failure) below)
- Reconstructs RTP audio streams into playable `.wav` files: reorders packets by sequence number (handling 16-bit wraparound), decodes G.711 µ-law payloads, and writes standard WAV output
- Cross-references SIP/SDP signaling data to identify what codec a given RTP payload type actually represents
- In progress: TCP stream reassembly and SMTP parsing, to reconstruct plaintext email content from mail-protocol captures

## Why this project

Built to understand network protocols from the ground up — not just reading Wireshark's output, but re-deriving what it's showing me: parsing headers by hand, reconstructing a stream's meaning from raw bytes, and hitting (and diagnosing) real gaps in a mature third-party library along the way.

## Architecture

Every core class (`FileHandler`, `PacketDissector`, `DataHandler`, `Logger`, `RtpStreamCollector`, `AudioReconstructor`) is a singleton accessed via `GetInstance()` — a deliberate choice for a single-pass CLI tool with no need for multiple concurrent instances.

## Tech stack

- **C++17**, Visual Studio 2022 (developed on Windows/MSVC; portable to CMake — see below)
- **[PcapPlusPlus](https://github.com/seladb/PcapPlusPlus)** — packet capture parsing, protocol layer dissection
- **Npcap** — packet capture driver dependency required by PcapPlusPlus on Windows

## Notable engineering problem: silent RTP detection failure

PcapPlusPlus identifies RTP traffic using a content heuristic on UDP payload bytes rather than relying on signaling context. While validating extracted RTP payloads byte-for-byte against Wireshark, I found that PcapPlusPlus's heuristic produced **false negatives** on a confirmed, SDP-negotiated RTP stream in a real captured VoIP call — `packet.getLayerOfType<pcpp::RtpLayer>()` silently returned `nullptr` for packets Wireshark correctly identified as RTP (Wireshark has the advantage of reading the SIP/SDP negotiation directly, rather than guessing from content alone).

**Diagnosis process:**
1. Selected one RTP packet Wireshark confirmed by sequence number and SSRC
2. Searched program output for that sequence number — found nothing
3. Traced the SIP/SDP negotiation in the same capture to confirm the port did carry RTP
4. Implemented a manual fallback: when the layer isn't auto-detected but is known (via SDP) to be RTP, parse the 12-byte RTP header directly off the UDP payload bytes
5. Re-validated the recovered payload byte-for-byte against Wireshark's ground truth

This is documented in more detail in the codebase and reflects the general debugging discipline used throughout the project: get an independent ground-truth data point, diff byte-for-byte rather than trusting "it ran without error," and narrow failures by elimination rather than guessing at fixes.

## Setup

### Option A — Visual Studio 2022

**1. Install prerequisites**
- Visual Studio 2022 with the **Desktop development with C++** workload
- [Npcap](https://npcap.com/#download) (install with "Install Npcap in WinPcap API-compatible mode" checked, and also grab the **Npcap SDK** separately)

**2. Get PcapPlusPlus**

Either build from source or use a package manager:

*From source (what this project was built against):*
git clone https://github.com/seladb/PcapPlusPlus.git

Follow PcapPlusPlus's own [Windows build instructions](https://pcapplusplus.github.io/docs/install/build-source/windows) to produce the `.lib` files (this requires CMake and links against the Npcap SDK you installed above).

*Or, faster — via vcpkg:*


**3. Configure project settings in Visual Studio**

Right-click the project → **Properties** (set Configuration to match what you're building, e.g. Debug x64):

| Setting | Location | Value |
|---|---|---|
| Include paths | VC++ Directories → Include Directories | Add PcapPlusPlus's `Common++/header`, `Packet++/header`, `Pcap++/header` folders |
| Library paths | VC++ Directories → Library Directories | Add the folder containing PcapPlusPlus's built `.lib` files |
| Linked libraries | Linker → Input → Additional Dependencies | `Packet++.lib`, `Pcap++.lib`, `Common++.lib`, `ws2_32.lib`, `Iphlpapi.lib`, `wpcap.lib`, `Packet.lib` |

**4. Build and run**

Build with Ctrl+Shift+B, then from a terminal in the output folder:


### Option B — Without Visual Studio (CMake)

PcapPlusPlus is fully CMake-compatible, so this project can be built with any C++17 compiler (MSVC, GCC, Clang) via CMake instead of the VS IDE directly.

**1. Install prerequisites**
- CMake (3.15+)
- A C++17 compiler (MSVC Build Tools, GCC, or Clang)
- Npcap + Npcap SDK (Windows) or `libpcap-dev` (Linux/macOS)

**2. Install PcapPlusPlus**

Via a package manager (recommended for this route):

Or build from source per PcapPlusPlus's [Build From Source](https://pcapplusplus.github.io/docs/install/build-source) docs for your platform.

**3. Add a `CMakeLists.txt`** to the project root:
```cmake
cmake_minimum_required(VERSION 3.15)
project(NetworkAnalyzer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PcapPlusPlus REQUIRED)

add_executable(NetworkAnalyzer
    "Network Analyzer/Main.cpp"
    "Network Analyzer/File_Handler.cpp"
    "Network Analyzer/Data_Handler.cpp"
    "Network Analyzer/Logger.cpp"
    "Network Analyzer/Packet_Dissector.cpp"
    "Network Analyzer/RtpStreamCollector.cpp"
    "Network Analyzer/AudioReconstructor.cpp"
)

target_link_libraries(NetworkAnalyzer PcapPlusPlus::Pcap++)
```

**4. Build**
mkdir build && cd build
cmake ..
cmake --build . --config Release
