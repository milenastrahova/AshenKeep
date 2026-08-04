# Engineering Improvements

## Reusable Targeting Math

`UAshenTargetingMathLibrary` centralizes vision-cone and target-score calculations shared by AI and lock-on.

This removes duplicated vector math and gives the project deterministic functions that can be tested independently.

## Testable Attribute Rules

`UAshenAttributeComponent` exposes pure static helpers for:

- clamped damage;
- clamped restoration;
- resource-consumption validation.

Runtime mutation methods call the same helpers that automation tests validate.

## Event-Driven Presentation

HUD widgets bind to replicated attribute delegates and unbind during destruction. This replaces continuous per-frame bar polling and demonstrates lifecycle-safe delegate management.

## Profiling Instrumentation

Named CPU trace scopes were added to combat, targeting, AI, objectives and HUD construction. This allows the expensive gameplay paths to be located directly in Unreal Insights.

## Network Validation

Automation tests inspect reflected RPC flags, proving that:

- gameplay requests are reliable Server RPCs;
- transient Blood Burst cues are unreliable multicast;
- victory presentation is reliable multicast.