# Ashen Keep: Blood Hunt

**[Portfolio Overview](PORTFOLIO.md) • [Game Design Case Study](docs/GAME_DESIGN_CASE_STUDY.md) • [ArtStation](https://www.artstation.com/milenastrahova)**

**Ashen Keep: Blood Hunt** is a dark-fantasy action game prototype built in **Unreal Engine 5.8**, focused on gameplay design, enemy encounters, player abilities, boss progression, objectives, UI feedback and iterative playtesting.

The player controls an ancient vampire who awakens during an invasion of the fortress. Monster hunters are performing a cleansing ritual before dawn; the objective is to fight through the keep, defeat the Hunter Captain and stop the purge.

## Quick Review

**Role:** Solo game designer / developer  
**Engine:** Unreal Engine 5.8  
**Primary focus:** Gameplay loop, enemy encounters, player abilities, boss encounter, objectives, UI feedback, playtesting  
**Playable state:** Packaged vertical slice  
**Best recruiter entry point:** [Game Design Case Study](docs/GAME_DESIGN_CASE_STUDY.md)

### Portfolio navigation

- **Ashen Keep** — complete gameplay vertical slice
- [Blood Cathedral](https://github.com/milenastrahova/BloodCathedral) — stealth/action design prototype
- [CyberCorp HQ](https://github.com/milenastrahova/CyberCorp-HQ-Technical-Art) — interaction, objective flow and technical art
- [ArtStation](https://www.artstation.com/milenastrahova) — visual portfolio


## Game Design Case Study

For a recruiter-friendly breakdown of the player loop, encounter design, playtesting and iteration process, see [Game Design Case Study](docs/GAME_DESIGN_CASE_STUDY.md).

## Gameplay Showcase

[Watch the gameplay showcase](media/ashen-keep-gameplay.mp4)

## Repository Scope

This public repository is intentionally **code-focused**. Third-party Unreal Engine Marketplace/Fab assets and generated packaged-build folders are excluded for licensing and repository-size reasons.

Included here:

- complete project C++ source;
- gameplay and networking tests;
- Unreal configuration;
- architecture, networking, testing and performance documentation;
- curated screenshots and gameplay showcase media.

The playable Windows Shipping build is distributed separately through **GitHub Releases**.

## Game Design Focus
- Clear player loop: explore → detect threat → position/evade → engage → survive → advance → defeat the captain
- Player mobility built around sprint and Mist Step for spacing and defensive decision-making
- Enemy perception, pursuit and attack behaviour used to create readable encounter pressure
- Health, damage and death states that make positioning meaningful
- A distinct Hunter Captain encounter as the final difficulty spike
- Objectives, pause flow and win/lose states that turn the prototype into a complete playable experience
- Iterative playtesting and balancing of AI, navigation, encounter difficulty and player feedback

## Technical Foundation
- Server-authoritative networked gameplay
- Unreal Engine C++ gameplay architecture
- Replicated player, enemy and objective state
- Reusable Actor Components
- C++/Blueprint separation
- Runtime UMG dashboard constructed in C++
- Packaged standalone Windows build

## Gameplay Systems

### Vampire Character

- Third-person movement and camera
- Sprinting with stamina consumption and regeneration
- **Mist Step** dodge with cooldown, stamina cost and temporary collision-state changes
- Melee attack with range and cooldown control
- Lock-on targeting
- Death and local HUD creation
- Vampiric kill recovery

### Blood Burst

UAshenBloodBurstComponent implements a reusable area-of-effect vampire ability:

- blood-resource cost;
- radius query;
- server-authoritative damage;
- cooldown management;
- kill rewards;
- unreliable multicast for cosmetic cues;
- Blueprint event hook for VFX and audio.

### Enemy and Boss Gameplay

- Hunter AI pursuit and attack behaviour
- Configurable chase speed, attack range, damage and cooldown
- Replicated death state
- Ragdoll/physics death presentation
- Distinct Hunter Captain encounter
- Enemy health UI

### Purge Ritual Objective

The objective actor is authoritative on the server and replicates ritual completion to clients. Victory is triggered after the Hunter Captain is defeated and the result is displayed locally on every connected player.

## Networking

The project uses Unreal's replication model:

- Server, Reliable RPCs for authoritative gameplay requests;
- replicated state with OnRep callbacks;
- reliable multicast for match-critical victory presentation;
- unreliable multicast for short-lived cosmetic ability cues;
- authority checks before damage, rewards and objective completion;
- replicated sprint, Mist Step, death and ritual state.

See [docs/NETWORKING.md](docs/NETWORKING.md) for the detailed flow.

## C++ and Blueprint Responsibilities

### C++

C++ owns:

- gameplay rules;
- networking and authority;
- attributes and resources;
- cooldowns and timers;
- combat queries;
- AI behaviour;
- objective completion;
- lock-on logic;
- runtime HUD behaviour.

### Blueprints

Blueprints are used for:

- asset assignment;
- animation assets;
- designer-facing balancing values;
- level composition;
- VFX and audio event implementation;
- visual presentation.

This keeps core rules maintainable and testable while preserving fast iteration for visual work.

## Main Runtime Classes

| Class | Responsibility |
|---|---|
| AAshenPlayerCharacter | Input, movement, sprint, Mist Step, melee combat, replication and player death |
| UAshenAttributeComponent | Health, stamina and Blood Essence state |
| UAshenBloodBurstComponent | Reusable server-authoritative area ability |
| UAshenLockOnComponent | Target acquisition and lock-on state |
| AAshenTrainingEnemy | Hunter AI, combat, health UI, animation and replicated death |
| AAshenEnemyAIController | Enemy perception/navigation control |
| AAshenPurgeRitualObjective | Replicated win condition and victory UI |
| UAshenPlayerHUDWidget | C++-constructed vampire HUD |
| AAshenDamageVolume | Environmental damage/hazard volume |

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Controls

- `WASD` - Move
- Mouse - Look
- `Shift` - Sprint
- `Ctrl` - Mist Step
- `Space` - Jump
- Left Mouse Button - Attack
- `Q` - Blood Burst
- `Tab` - Lock on
- `Esc` - Pause
## Performance-Oriented Decisions

- Core gameplay avoids unnecessary global searches after initialization.
- Repeating resource updates and objective checks use timers.
- Cosmetic network cues use unreliable multicast where packet loss is acceptable.
- Damage, rewards and objective completion execute on the authority.
- Gameplay tuning data is exposed through editor properties rather than hard-coded into Blueprints.
- Components isolate reusable systems from the player class.
- Timer handles are cleared during lifecycle transitions where required.

A formal profiling plan is documented in [docs/PERFORMANCE.md](docs/PERFORMANCE.md).

## Build Requirements

- Unreal Engine 5.8
- Windows 10/11
- Visual Studio with Desktop/Game Development for C++
- Enhanced Input

## Reviewing the Project

This repository is intended for source-code and architecture review.

1. Clone the repository.
2. Open Source/AshenKeep to review the gameplay module.
3. Read the architecture, networking, testing and performance documents in docs/.
4. Download the packaged Windows build from the Releases section to play the vertical slice.

The complete editor project also uses third-party visual assets that are intentionally not redistributed through this public repository.

<!-- PERFORMANCE_PROFILING_START -->
## Measured Performance

Standalone profiling of the heaviest captured environment and Hunter Captain combat encounter remained near 9.2 ms frame time, or approximately 108 FPS on the test machine.

| Metric | Heavy Scene | Boss Combat |
|---|---:|---:|
| Frame time | 9.27 ms | 9.22 ms |
| Game Thread | 8.15 ms | 8.60 ms |
| Draw Thread | 9.32 ms | 9.17 ms |
| GPU time | 6.61 ms | 6.74 ms |
| Draw calls | 1,952 | 2,145 |
| Primitives | 963.0K | 1,098.5K |

The measured bottleneck was primarily the Draw Thread rather than GPU execution. Texture memory was approximately 895.88 MB inside a 1000 MB pool.

See [docs/PERFORMANCE_PROFILING.md](docs/PERFORMANCE_PROFILING.md) for profiling screenshots, GPU passes, memory measurements, bottleneck analysis and the optimization plan.
<!-- PERFORMANCE_PROFILING_END -->

## Repository Structure

```text
AshenKeep/
|-- Config/
|-- Source/
|   `-- AshenKeep/
|       |-- Private/
|       |   `-- Tests/
|       `-- Public/
|-- docs/
|-- media/
|-- AshenKeep.uproject
`-- README.md
```
## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Networking](docs/NETWORKING.md)
- [Performance](docs/PERFORMANCE.md)
- [Testing Strategy](docs/TESTING.md)


## Portfolio Relevance

This project demonstrates my **game design process as well as implementation ability**:

- defining a clear gameplay loop and player objective;
- designing movement, combat and encounter rules;
- tuning enemy behaviour and boss difficulty through playtesting;
- debugging navigation and encounter-flow problems;
- using UI, animation and audio to improve player feedback;
- carrying a prototype through to a complete packaged vertical slice;
- supporting the design with C++ and Blueprint implementation.

## Author

**Milena Strahova**

- GitHub: https://github.com/milenastrahova
- ArtStation: https://www.artstation.com/milenastrahova

## Automated Verification

Ashen Keep includes seven Unreal Automation Framework tests covering deterministic attribute rules, targeting math, replication setup, RPC flags and performance-oriented Tick configuration.

Run them with:

```powershell
powershell -ExecutionPolicy Bypass -File ".\Tools\Run_AshenKeep_AutomationTests.ps1"
```

Named Unreal Insights CPU scopes are documented in [docs/PERFORMANCE.md](docs/PERFORMANCE.md).
