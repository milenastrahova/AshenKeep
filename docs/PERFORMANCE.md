# Performance and Profiling

## Implemented Optimizations

### Event-Driven UI

Player and enemy health widgets subscribe to attribute delegates. They update when health, stamina or Blood Essence changes instead of recalculating bars every frame.

### Timer-Driven Systems

- Enemy AI uses a configurable think timer.
- The ritual objective uses a completion-check timer.
- Sprint drain and regeneration use timers.
- Damage volumes use periodic timers.
- Cooldowns use timer handles.

### Conditional Tick

- Lock-on Tick starts disabled and is enabled only while a target is locked.
- Attribute components do not Tick.
- Damage volumes, AI controllers and ritual objectives do not Tick.
- Character/enemy animation updates use a reduced tick interval.

### Network Cost

- Server authority owns damage, rewards and objective completion.
- Short-lived cosmetic cues use unreliable multicast.
- Reliable RPC/multicast is reserved for gameplay requests and match-critical results.
- Replicated booleans keep state payloads compact.

## Unreal Insights Trace Scopes

The following named CPU scopes are implemented:

```text
AshenKeep_AI_Update
AshenKeep_LockOn_FindBestTarget
AshenKeep_Player_PerformAttack
AshenKeep_Enemy_TryAttack
AshenKeep_BloodBurst_Perform
AshenKeep_Objective_Evaluate
AshenKeep_HUD_BuildInterface
```

They can be searched directly in Unreal Insights.

## Measurement Plan

Use one repeatable fortress-combat scene and record:

| Metric | Tool | Result |
|---|---|---|
| Game thread | Unreal Insights / `stat unit` | Pending capture |
| GPU frame | GPU Visualizer / `stat gpu` | Pending capture |
| Draw calls | `stat scenerendering` | Pending capture |
| Memory | `stat memory` | Pending capture |
| Network traffic | `stat net` in 2-player PIE | Pending capture |

Performance values must be measured before publishing; no invented numbers are used.