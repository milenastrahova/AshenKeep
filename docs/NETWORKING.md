# Networking and Replication

## Authority Model

Ashen Keep uses a server-authoritative model.

Clients request actions through Server RPCs. The server validates resources, cooldowns and gameplay state before applying results.

## Player State

Replicated player state includes:

- sprinting;
- Mist Step;
- death.

OnRep callbacks update the local presentation after replicated state changes.

## Gameplay RPCs

Representative server requests include:

- sprint state changes;
- Mist Step activation;
- melee attack;
- Blood Burst activation.

The client does not directly author damage or resource rewards.

## Blood Burst Flow

`mermaid
sequenceDiagram
    participant Client
    participant Server
    participant Targets
    participant OtherClients

    Client->>Server: ServerActivateBloodBurst()
    Server->>Server: Validate cooldown and Blood Essence
    Server->>Targets: Apply authoritative radial damage
    Server->>Server: Apply kill rewards
    Server-->>OtherClients: MulticastBloodBurstCue (Unreliable)
`

The multicast is unreliable because it represents a short-lived cosmetic cue. Missing one visual packet must not affect gameplay state.

## Objective Flow

`mermaid
sequenceDiagram
    participant Server
    participant Captain
    participant Objective
    participant Clients

    Server->>Captain: Authoritative damage
    Captain->>Captain: Replicated death state
    Objective->>Captain: Periodic completion check
    Objective->>Objective: Set replicated ritual complete state
    Objective-->>Clients: Reliable victory multicast / OnRep
    Clients->>Clients: Create local victory UI
`

## Replication Rules

- Gameplay damage and resource rewards run only on authority.
- Match-critical ritual completion uses replicated state and reliable presentation.
- Cosmetic cues use unreliable multicast.
- UI is created only on local controllers.
- Dedicated servers do not create player widgets.