# Ashen Keep: Blood Hunt — Game Design Case Study

## Project Goal

Create a small but complete dark-fantasy action experience that can be played from start to finish.

The player explores a vampire fortress, encounters hostile hunters, manages movement and survival, and ultimately defeats the Hunter Captain to stop the cleansing ritual.

My design priorities were:

- a clear objective and readable gameplay loop;
- movement that gives the player meaningful positioning choices;
- encounters that escalate without becoming confusing;
- visible feedback for danger, damage, death and completion;
- a complete beginning-to-end flow rather than isolated mechanics.

## Core Gameplay Loop

**Explore → Detect threat → Position / evade → Engage → Survive → Clear encounter → Advance → Defeat the captain**

Every major system was built to support this loop.

## Player Movement

The player can sprint and use **Mist Step** as a short evasive movement tool.

### Design intent

- Sprint makes traversal less passive.
- Mist Step gives the player a defensive choice during combat.
- Both systems create spacing decisions instead of forcing the player to only trade damage.

### Iteration focus

Movement was tuned together with camera behaviour and encounter spacing so that mobility felt useful without making enemies irrelevant.

## Enemy Encounters

Hunters use perception, pursuit and attack behaviour to create pressure while the player moves through the fortress.

### Design intent

- make enemies react to player presence;
- create readable threat escalation;
- turn navigation into encounter pacing rather than static target placement.

### Problems found during testing

Some enemies initially remained static or could not reliably reach the player, especially in the basement area.

### Iteration

- checked navigation coverage;
- corrected NavMesh and movement setup;
- retested aggro and pursuit;
- adjusted encounter behaviour until enemies consistently participated in the intended gameplay loop.

## Health, Damage and Failure

The project includes player health, environmental damage and clear death handling.

### Design intent

Damage exists to make positioning and timing meaningful.

The player should understand:

- when they are in danger;
- when a decision caused damage;
- when the current attempt has failed.

## Hunter Captain Encounter

The Hunter Captain acts as the final difficulty spike.

### Design intent

The fight needed to feel meaningfully stronger than standard encounters while still remaining completable.

### Iteration

The boss was repeatedly tested and adjusted so the final encounter felt like a climax rather than an arbitrary difficulty wall.

## Objectives and Game Flow

The prototype includes:

- a clickable main menu;
- pause flow;
- objective progression;
- win state;
- lose state;
- final completion screen.

These systems were added to make the project function as a complete game experience, not only an Unreal Editor demonstration.

## Player Feedback

Animations, audio and UI were added to improve readability.

### Feedback goals

- enemy movement communicates threat;
- death animation confirms enemy defeat;
- UI communicates player state and objectives;
- audio and music support tension and completion.

## What I Owned

- gameplay concept and player loop;
- movement and player abilities;
- health / damage / death rules;
- enemy encounters and navigation debugging;
- captain encounter and balance iteration;
- objective progression;
- menus, pause and win / lose states;
- gameplay UI;
- animation and audio integration;
- playtesting and debugging;
- final packaged build.

## What I Learned

The main lesson from this project was that implementing a mechanic is not enough.

A mechanic needs to:

1. have a clear purpose;
2. change player decisions;
3. communicate its state;
4. fit the rest of the gameplay loop;
5. survive actual playtesting.

Building Ashen Keep end-to-end made me think about encounter pacing, feedback, difficulty, failure states and how separate systems combine into one playable experience.

## Next Design Pass

If I continued development, I would formalize balancing with tracked values for:

- enemy health and damage;
- player survival time;
- encounter completion time;
- ability usage;
- boss attempts / completion rate.

I would then run structured external playtests and compare intended difficulty with observed player behaviour.

## Portfolio Links

- [GitHub profile](https://github.com/milenastrahova)
- [ArtStation](https://www.artstation.com/milenastrahova)
