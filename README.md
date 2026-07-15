# MMO Engine Architecture 🚀

Welcome to our custom C++ & Go MMO Engine project! This engine is designed from the ground up to support a highly scalable, server-authoritative multiplayer experience (inspired by games like GTA: Vice City).

It uses a modern tech stack encompassing C++, Vulkan, EnTT (ECS), GameNetworkingSockets, and a Go-based Microservices backend.

## Tech Stack
*   **Language:** C++20 for the core engine, Go 1.23+ for backend microservices.
*   **Graphics:** Vulkan (via `vulkan-headers` and `SDL2`).
*   **ECS (Entity Component System):** `EnTT` for highly performant game logic.
*   **Networking:** `GameNetworkingSockets` (Valve) for reliable UDP transport.
*   **HTTP Client:** `cpr` (C++ Requests) for REST API communication.
*   **Build System:** CMake + `vcpkg` for dependency management.

## Project Structure
*   `engine/` - The core C++ engine (ECS, Render Backend, Networking wrapper, Application loop).
*   `game/` - The C++ Game Client executable.
*   `server/world/` - The C++ authoritative UDP World Server executable.
*   `server/microservices/` - The Go backend microservices (e.g., Login Service).

## Prerequisites
To build this project, you need:
1.  **C++ Compiler:** GCC/Clang (Linux) or MSVC (Windows) with C++20 support.
2.  **CMake:** Version 3.15 or higher.
3.  **vcpkg:** For C++ dependency management.
4.  **Vulkan SDK:** Installed on your system (e.g., `vulkan-headers`, `glslc`, `glslangValidator`).
5.  **Go:** Version 1.23 or higher (for the microservices).
6.  **SDL2 Dependencies:** (e.g., `libsdl2-dev` on Debian/Ubuntu).

## How to Build (Linux)

### 1. Build the C++ Engine & Game
Assuming `vcpkg` is installed at `~/vcpkg`:

```bash
# Clone the repository
git clone <your-repo-url>
cd hello-world

# Configure CMake using the vcpkg toolchain
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake

# Compile the project
cmake --build build -j4
```

*Note: The first build may take a few minutes as `vcpkg` downloads and compiles all dependencies (SDL2, EnTT, GameNetworkingSockets, cpr, etc.).*

### 2. Run the MMO

You will need three separate terminal windows to run the full stack:

**Terminal 1 (Go Login Microservice):**
```bash
cd server/microservices/login_service
go run main.go
```
*The login service will start on `http://localhost:8080`.*

**Terminal 2 (C++ Authoritative World Server):**
```bash
# From the project root directory
./build/server/world/WorldServer
```
*The World Server will start listening for UDP connections on port `27015`.*

**Terminal 3 (C++ Game Client):**
```bash
# From the project root directory
./build/game/GameClient
```

### Architecture Flow (How it Works)
1.  **Authentication:** When the `GameClient` starts, it pauses the engine and makes a blocking HTTP POST request to the Go `login_service` on port `8080`.
2.  **Token Generation:** The Go service validates the credentials and returns a secure Session Token (JSON).
3.  **Server Connection:** The C++ client receives the token and initiates a reliable UDP connection to the `WorldServer` via `GameNetworkingSockets`.
4.  **Handshake:** The client immediately sends an `OpCode::Auth` packet containing the token.
5.  **Spawning:** The server verifies the token. Upon success, it creates a new ECS entity for the player and sends an `OpCode::SpawnEntity` packet.
6.  **Gameplay Loop:** The client captures input (W, A, S, D) and sends `OpCode::PlayerInput` packets to the server. The server calculates the new positions and broadcasts `OpCode::UpdateTransform` packets to all connected clients.
7.  **Rendering:** The client's Vulkan backend uses `PushConstants` to update the position of the entity and draws it on the screen.
