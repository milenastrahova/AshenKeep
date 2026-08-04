# Ashen Keep Architecture

## Design Goals

Ashen Keep separates authoritative gameplay rules from visual presentation.

The main goals are:

- keep game rules in C++;
- make abilities reusable through components;
- expose tuning values to designers;
- use Blueprint events for cosmetic work;
- keep multiplayer state server-authoritative;
- avoid coupling the objective, player and enemy implementations.

## Runtime Layers

`mermaid
flowchart TD
    INPUT[Enhanced Input] --> PLAYER[AAshenPlayerCharacter]

    PLAYER --> ATTR[UAshenAttributeComponent]
    PLAYER --> LOCK[UAshenLockOnComponent]
    PLAYER --> BURST[UAshenBloodBurstComponent]
    PLAYER --> HUD[UAshenPlayerHUDWidget]

    AIC[AAshenEnemyAIController] --> ENEMY[AAshenTrainingEnemy]
    ENEMY --> ATTR

    PLAYER -->|Damage / abilities| ENEMY
    ENEMY -->|Attack damage| PLAYER

    ENEMY --> OBJECTIVE[AAshenPurgeRitualObjective]
    OBJECTIVE -->|Replicated victory| CLIENTS[Local Player UI]
`

## Composition

Reusable state and behaviour are implemented as Actor Components rather than being duplicated inside actors.

### Attribute Component

Responsible for health, stamina and Blood Essence values.

### Blood Burst Component

Owns activation validation, resource cost, radius damage, cooldown and kill rewards.

### Lock-On Component

Owns target discovery and target-state management.

## Character Responsibilities

AAshenPlayerCharacter coordinates input and character-level state. It delegates resource logic and independent abilities to components.

The character owns:

- input binding;
- camera-relative movement;
- sprint state;
- Mist Step;
- melee attack;
- death state;
- HUD creation;
- network RPC entry points.

## Enemy Responsibilities

AAshenTrainingEnemy owns hunter-specific behaviour and state:

- target tracking;
- attack range and cooldown;
- damage application;
- health display;
- animation state;
- replicated death and physics presentation.

AI navigation/control is separated into AAshenEnemyAIController.

## Objective Responsibilities

AAshenPurgeRitualObjective does not own the boss. It observes the assigned or discovered Hunter Captain and completes only on authority after the captain dies.

Completion is replicated and the victory UI is created only for local player controllers.

## C++ / Blueprint Boundary

C++ provides deterministic, network-aware gameplay. Blueprints provide presentation and data:

- meshes;
- animation Blueprints or animation assets;
- materials;
- VFX;
- audio;
- designer balancing;
- level references.

Blueprint events are used as presentation hooks instead of moving authoritative gameplay rules into visual scripting.