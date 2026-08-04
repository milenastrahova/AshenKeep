# Testing Strategy

## Current Manual Test Matrix

### Player

- movement and camera input;
- sprint start/stop;
- stamina depletion and regeneration;
- Mist Step cooldown and stamina cost;
- melee attack cooldown;
- lock-on acquisition and release;
- death state;
- HUD updates.

### Combat

- player damage against hunters;
- hunter damage against player;
- Blood Burst resource validation;
- Blood Burst multi-target damage;
- kill rewards;
- enemy death and ragdoll;
- captain defeat.

### Objective

- ritual remains incomplete while captain is alive;
- ritual completes after captain death;
- victory UI appears once;
- dedicated server does not create UI.

### Multiplayer

- server-authoritative sprint;
- replicated Mist Step;
- replicated death;
- Blood Burst damage created only once;
- cosmetic cues visible on clients;
- ritual completion visible on all players.

## Planned Unreal Automation Tests

The next code pass will add tests for:

- attribute clamping;
- resource spending;
- cooldown state transitions;
- Blood Burst activation validation;
- captain/objective completion;
- replication-oriented state changes where testable without a full network session.

## Test Naming

`	ext
AshenKeep.Attributes.*
AshenKeep.Abilities.*
AshenKeep.Objective.*
AshenKeep.Networking.*
`

The goal is to keep deterministic gameplay logic testable independently from visual assets.