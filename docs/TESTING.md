# Automated Testing

Ashen Keep includes Unreal Automation Framework tests under:

```text
Source/AshenKeep/Private/Tests/AshenCoreAutomationTests.cpp
```

## Test Groups

### Attributes

- `AshenKeep.Attributes.DefaultValues`
- `AshenKeep.Attributes.DeterministicMath`

These tests validate default resource configuration, replication settings, damage clamping, restoration clamping and resource-consumption rules.

### Targeting Math

- `AshenKeep.Math.Targeting.VisionCone`
- `AshenKeep.Math.Targeting.Score`

These tests validate the reusable 2D dot-product vision-cone logic and the lock-on scoring function used by both player targeting and enemy AI.

### Networking

- `AshenKeep.Network.ReplicationConfiguration`
- `AshenKeep.Network.RPCFunctionFlags`

These tests verify replicated actors/components and inspect Unreal reflection flags for Server RPC, reliable multicast and unreliable multicast configuration.

### Performance Configuration

- `AshenKeep.Performance.TickConfiguration`

This test verifies that timer-driven actors do not use Tick and that lock-on starts with component Tick disabled.

## Running Tests

From the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File ".\Tools\Run_AshenKeep_AutomationTests.ps1"
```

The runner launches `UnrealEditor-Cmd` with `NullRHI`, executes every `AshenKeep.*` test and saves a complete log on the desktop.

## Design Principle

Deterministic calculations are extracted into pure reusable functions. This allows gameplay math to be tested without loading a map or depending on visual assets.