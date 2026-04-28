# PF-LAB-Battleship-Project
# Weird Battleship Game

A BAD C implementation of the classic Battleship board game built with [raylib](https://www.raylib.com/). Features a polished naval UI (ALL DONE IN RAYLIB), two game modes, and background music.

**By:** Jarar Khan, Mool Bhoomika & Ahad Kashif

---

## Features

- **Two game modes** — Pass & Play (local 2-player) and VS AI
- **Ship placement phase** with real-time placement preview and rotation
- **Animated battle phase** with aim reticle and last-shot highlight
- **AI opponent** with a built-in thinking delay for immersion
- **Stats panel** tracking ships remaining and shots fired for both players
- **Background music** via raylib's audio streaming
- **Full restart** from the game-over screen

## Controls

| Input | Action |
| Left Click | Place ship / Fire / Navigate menus |
| R | Rotate ship orientation (during placement) |

## How to Play

### 1. Main Menu
Choose **PASS & PLAY** for two local players or **VS AI** to fight the computer.

### 2. Ship Placement
- Each player places their 3 ships on the **left board**.
- Hover over the grid to see a placement preview — **yellow** means valid, **red** means invalid.
- Press **R** to toggle between horizontal and vertical orientation.
- In **PASS & PLAY**, Player 2 places their ships after Player 1 is done.
- In **VS AI**, the AI places its ships automatically.

### 3. Battle Phase
- Your fleet is shown on the **left board**; the enemy's is on the **right**.
- Click a cell on the **right board** to fire.
  - **Hit** — cell turns red
  - **Miss** — cell turns grey
- In **PASS & PLAY**, a handoff screen appears between turns so neither player sees the other's board.
- In **VS AI**, the AI fires back automatically after a short delay.

### 4. Game Over
The first player to sink all enemy ships wins. Click **RESTART** to play again.

---

## Building

### Prerequisites
- [raylib](https://www.raylib.com/) installed on your system
- A C compiler (gcc / clang / MSVC)
- 
### Assets
Place the following file in the same directory as the executable:
```
Soundtrack.mp3
```

---

## Project Structure

```
battleship/
├── main.c          # All game logic and rendering
├── Soundtrack.mp3  # Background music
└── README.md
```

---

## Technical Notes

- Grid is **10×10**, each cell is **36×36 px**, window is **1024×640 px**.
- Game states: `0` Main Menu → `1/2` Ship Placement → `3` Battle → `4` Game Over → `5` Switch Screen.
- AI uses simple **random targeting** (no hunt/target logic).
- All board state is stored in `char[10][10]` arrays using the characters `~` (water), `S` (ship), `X` (hit), `O` (miss).
- All the UI is done in Raylib and done mostly using the built in raylib functions DrawRectangle And DrawRectangleLines
- A major technical limitation of the current project state is the fact there are no sprites or textures or sound other than the bg music these features were cut due to time constraints and knowledge limitations
