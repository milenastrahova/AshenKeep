# Performance Considerations

## Current Engineering Decisions

### CPU

- Resource updates and cooldowns use timers.
- The purge objective uses a configurable timer rather than per-frame Tick.
- Actor Components isolate systems and reduce repeated logic.
- Gameplay queries use bounded ranges and configurable radii.
- Debug drawing is controlled through editor-exposed flags.

### GPU / Visuals

The vertical slice is intended for a controlled showcase map. Visual validation should cover:

- material complexity;
- movable light count;
- shadow cost;
- skeletal mesh cost;
- transparency/particle overdraw;
- post-processing cost;
- draw calls.

### Network

- Server-authoritative actions prevent divergent client state.
- Unreliable multicast is used for non-critical cosmetic cues.
- Replicated booleans represent compact gameplay state.
- Reliable communication is reserved for state-changing actions and victory.

### Memory

- Unreal object references use reflected object pointers.
- Reusable systems are components instead of duplicated actor logic.
- Assets should be validated through Size Map and Reference Viewer before release.

## Profiling Procedure

The next measured pass should record:

1. stat unit
2. stat game
3. stat gpu
4. stat memory
5. stat net
6. Unreal Insights CPU trace
7. RenderDoc or GPU Visualizer capture
8. packaged Shipping build frame-time sample

Results should be recorded as:

| Metric | Before | After | Test Scene |
|---|---:|---:|---|
| Game thread | TBD | TBD | Main fortress combat |
| GPU frame | TBD | TBD | Main fortress combat |
| Draw calls | TBD | TBD | Main fortress combat |
| Memory | TBD | TBD | Packaged build |
| Network traffic | TBD | TBD | 2-player PIE |

No performance number should be published until it has been measured in a repeatable test.