# OrbDrive LoRa Firmware Freeze

**Freeze date:** 2026-09-07
**Frozen source branch:** main
**Freeze branch:** freeze/lora-fw-2026-09-07
**Purpose:** Preserve the actual deployed/accepted LoRa firmware baseline while application projects continue separately.

## Rule
No functional LoRa firmware changes are to be made as part of the Operator APK, Gateway Owner App, Main Company Web, or future SCADA projects.

Any future firmware change must be deliberate, separately reviewed, tested, and released as a new firmware version.

## Integration boundary
Applications communicate through the agreed JSON/MQTT/API contracts. The Gateway communicates with LoRa nodes using the firmware's binary gateway protocol.

## Timestamp rule
Where available, preserve node timestamp, gateway receipt timestamp, and server timestamp separately.

## Project separation
- LoRa firmware: this repository
- Operator APK: separate repository
- Gateway Owner App: separate repository
- Main Company Web: separate repository
- Centralized Municipal SCADA: separate repository

## Current application-design baseline
The Operator APK must not modify firmware authority, safety logic, LoRa radio authority, node authorization/rebind, or commercial enforcement. Firmware remains authoritative for device safety and execution.
