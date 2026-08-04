# Automation Test Results

**Status:** Passed  
**Verified:** 2026-08-04 16:55  
**Engine:** Unreal Engine 5.8  
**Configuration:** Win64 Development Editor, NullRHI unattended test run  
**Test command:** `Automation RunTests AshenKeep`

## Result

At least seven successful Unreal Automation Framework results were detected. No failed or error results were found.

## Covered Areas

- attribute defaults and valid ranges;
- attribute damage, restoration and resource consumption;
- reusable targeting and vision-cone mathematics;
- lock-on target scoring;
- actor replication configuration;
- Server and NetMulticast RPC reflection flags;
- performance-oriented Tick and timer configuration.

## Successful Tests

- `DefaultValues`
- `DeterministicMath`
- `ReplicationConfiguration`
- `RPCFunctionFlags`
- `Score`
- `TickConfiguration`
- `VisionCone`

## Reproduction

`powershell
powershell -ExecutionPolicy Bypass -File ".\Tools\Run_AshenKeep_AutomationTests.ps1"
`

The full local Unreal log is generated during each verification run and is intentionally not committed.