# System Architecture & Requirements Checklist

This document maps every exam requirement to the implementation in `2026_exam.cpp`.

---

## 1. System Architecture (High-Level)

```mermaid
flowchart TB
    subgraph UI["Presentation Layer"]
        MENU["Menu-Driven Console (main)"]
        VAL["Validation Helpers<br/>(readIntInRange, readValidPlate, ...)"]
    end

    subgraph CORE["Business Logic Layer"]
        SPS["SmartParkingSystem"]
        TM["TariffManager"]
    end

    subgraph MODEL["Domain / Entity Layer"]
        PS["ParkingSlot"]
        AP["ActiveParking"]
        PT["ParkingTransaction"]
        PRINT["Printable (abstract)"]
    end

    subgraph DATA["In-Memory Data Layer"]
        V1["vector&lt;ParkingSlot&gt;"]
        M1["unordered_map&lt;string, int&gt;<br/>slotIndexMap"]
        M2["unordered_map&lt;string, ActiveParking&gt;<br/>activeVehicles"]
        V2["vector&lt;ParkingTransaction&gt;<br/>history"]
        M3["unordered_map&lt;VehicleType, double&gt;<br/>hourlyRates"]
    end

    MENU --> VAL
    VAL --> SPS
    SPS --> TM
    SPS --> PS
    SPS --> AP
    SPS --> PT
    PS --> PRINT
    PT --> PRINT
    TM --> M3
    SPS --> V1
    SPS --> M1
    SPS --> M2
    SPS --> V2
```

---

## 2. Core System Components

```mermaid
flowchart LR
    subgraph Task1["Task 1 — Slot Configuration"]
        T1A["addParkingSlots()"]
        T1B["displayAllSlots()"]
    end

    subgraph Task2["Task 2 — Vehicle Entry"]
        T2A["registerVehicleEntry()"]
        T2B["findAvailableSlot()"]
    end

    subgraph Task3["Task 3 — Fees & Pricing"]
        T3A["updateParkingPrices()"]
        T3B["calculateBillableHours()"]
        T3C["TariffManager"]
    end

    subgraph Task4["Task 4 — Vehicle Exit"]
        T4A["processVehicleExit()"]
        T4B["readValidExitTime()"]
    end

    subgraph Reports["Operational Reports"]
        R1["displayAvailableSlots()"]
        R2["displayParkedVehicles()"]
        R3["displayParkingHistory()"]
        R4["displayDailyRevenue()"]
    end

    T1A --> T2B
    T2A --> T2B
    T2A --> T4A
    T4A --> T3B
    T4A --> T3C
    T4A --> R3
    T4A --> R4
```

---

## 3. Component Responsibilities

| Component | Role | File / Location |
|-----------|------|-----------------|
| `main()` | Menu loop, routes user choice to system methods | `2026_exam.cpp` |
| `SmartParkingSystem` | Central controller for all tasks and reports | `2026_exam.cpp` |
| `TariffManager` | Default rates, price updates, rate lookup at exit | `2026_exam.cpp` |
| `ParkingSlot` | Slot ID, type, zone, status (Available/Occupied) | `2026_exam.cpp` |
| `ActiveParking` | Live session: plate, type, entry time, assigned slot | `2026_exam.cpp` |
| `ParkingTransaction` | Completed exit record with frozen rate and fee | `2026_exam.cpp` |
| `Printable` | Abstract display interface (polymorphism) | `2026_exam.cpp` |
| Validation helpers | Input sanitization, range checks, business rules | `2026_exam.cpp` |

---

## 4. Data Flow Diagram (DFD)

A **Data Flow Diagram** shows how information moves through the system — from user input, through processing, into data stores, and back out as reports/receipts.

### 4.0 Context Diagram (Level 0)

Shows the whole system as one process and its external interactions.

```mermaid
flowchart LR
    USER(["👤 Parking Attendant<br/>(External Entity)"])

    SYS["0.0<br/>Kigali Smart Parking<br/>Management System"]

    USER -->|"Slot config<br/>(ID, type, zone)"| SYS
    USER -->|"Vehicle entry<br/>(plate, type, time)"| SYS
    USER -->|"Price update<br/>(type, new rate)"| SYS
    USER -->|"Vehicle exit<br/>(plate, exit time)"| SYS
    USER -->|"Report requests"| SYS

    SYS -->|"Slot list / availability"| USER
    SYS -->|"Entry confirmation<br/>+ allocated slot"| USER
    SYS -->|"Updated tariff table"| USER
    SYS -->|"Exit receipt<br/>+ parking fee"| USER
    SYS -->|"Parked vehicles /<br/>history / revenue"| USER
```

---

### 4.1 Level 1 Data Flow Diagram

Breaks the system into main processes, data stores, and labeled data flows.

```mermaid
flowchart TB
    USER(["👤 Attendant"])

    subgraph P["Processes"]
        P1["1.0<br/>Configure<br/>Parking Slots"]
        P2["2.0<br/>Register<br/>Vehicle Entry"]
        P3["3.0<br/>Manage<br/>Tariffs"]
        P4["4.0<br/>Process<br/>Vehicle Exit"]
        P5["5.0<br/>Generate<br/>Reports"]
    end

    subgraph DS["Data Stores (In-Memory)"]
        D1[("D1: Parking Slots<br/>vector + slotIndexMap")]
        D2[("D2: Active Sessions<br/>activeVehicles map")]
        D3[("D3: Tariff Rates<br/>hourlyRates map")]
        D4[("D4: Transaction History<br/>history vector")]
    end

    USER -->|"slot ID, type, zone"| P1
    P1 -->|"validated slot record"| D1
    D1 -->|"slot availability"| P1

    USER -->|"plate, type, entry time"| P2
    D1 -->|"available matching slot"| P2
    D3 -.->|"rate reference only<br/>(no fee yet)"| P2
    P2 -->|"ActiveParking session"| D2
    P2 -->|"status = OCCUPIED"| D1
    P2 -->|"allocation result"| USER

    USER -->|"vehicle type, new rate"| P3
    P3 -->|"updateRate()"| D3
    D3 -->|"current rates"| P3
    P3 -->|"confirmation"| USER

    USER -->|"plate, exit time"| P4
    D2 -->|"entry time, type, slot ID"| P4
    D3 -->|"active rate at exit"| P4
    P4 -->|"billable hours + total fee"| USER
    P4 -->|"status = AVAILABLE"| D1
    P4 -->|"remove session"| D2
    P4 -->|"ParkingTransaction<br/>(frozen rate + fee)"| D4

    USER -->|"report choice"| P5
    D1 -->|"slot data"| P5
    D2 -->|"parked vehicle data"| P5
    D4 -->|"completed transactions"| P5
    D3 -->|"tariff data"| P5
    P5 -->|"available slots /<br/>parked list /<br/>history / revenue"| USER
```

#### DFD Legend

| Symbol | Meaning | Implementation |
|--------|---------|----------------|
| 👤 External Entity | User outside the system | Parking attendant at console |
| `1.0 – 5.0` Process | Business logic action | Methods in `SmartParkingSystem` |
| `D1 – D4` Data Store | Where data is kept | `vector` / `unordered_map` in memory |
| Arrow label | Data that flows | Input fields, records, or report output |

#### Process → Code Mapping

| DFD Process | Menu Option | Code Function |
|-------------|-------------|---------------|
| 1.0 Configure Slots | 1 | `addParkingSlots()` |
| 2.0 Register Entry | 2 | `registerVehicleEntry()` |
| 3.0 Manage Tariffs | 3, 10 | `updateParkingPrices()`, `displayTariffs()` |
| 4.0 Process Exit | 4 | `processVehicleExit()` |
| 5.0 Generate Reports | 5–8, 9, 11 | `displayAvailableSlots()`, `displayParkedVehicles()`, etc. |

#### Data Store Contents

| Store | Fields Stored | Updated When |
|-------|---------------|--------------|
| **D1** Parking Slots | Slot ID, vehicle type, zone, status | Task 1 (add), Task 2 (occupy), Task 4 (release) |
| **D2** Active Sessions | Plate, type, entry time, slot ID | Task 2 (insert), Task 4 (delete) |
| **D3** Tariff Rates | Motorcycle/Car/Truck hourly rate | Startup (defaults), Task 3 (update) |
| **D4** History | Plate, times, hours, rate applied, fee | Task 4 only (append-only) |

---

### 4.2 End-to-End Data Flow (Lifecycle)

Shows how one vehicle's data moves through all stores from entry to exit.

```mermaid
flowchart LR
    subgraph Input["User Input"]
        I1["Slot: C1, Car, Zone B"]
        I2["Entry: RAB123A, Car, 08:00"]
        I3["Exit: RAB123A, 09:15"]
    end

    subgraph D1["D1 Slots"]
        S1["C1 | Car | Zone B | AVAILABLE"]
        S2["C1 | Car | Zone B | OCCUPIED"]
        S3["C1 | Car | Zone B | AVAILABLE"]
    end

    subgraph D2["D2 Active"]
        A1["RAB123A | Car | 08:00 | C1"]
    end

    subgraph D3["D3 Tariffs"]
        T1["Car = 1000 RWF/hr"]
    end

    subgraph D4["D4 History"]
        H1["RAB123A | 08:00→09:15 | 2hrs | 1000 | 2000 RWF"]
    end

    I1 --> S1
    I2 --> S1
    S1 --> S2
    I2 --> A1
    I3 --> A1
    A1 --> T1
    T1 --> H1
    A1 -.->|"erase"| X[" "]
    S2 --> S3
```

---

### 4.3 Process-Specific Flows (Sequence Diagrams)

#### 4.3a. Vehicle Entry Flow

```mermaid
sequenceDiagram
    actor User
    participant Menu
    participant System as SmartParkingSystem
    participant Map as activeVehicles
    participant Slots as vector ParkingSlot

    User->>Menu: Option 2 — Register entry
    Menu->>System: registerVehicleEntry()
    System->>System: Validate plate (unique, format)
    System->>System: findAvailableSlot(vehicleType)
    alt No matching slot
        System-->>User: Graceful error message
    else Slot found
        System->>Slots: setStatus(OCCUPIED)
        System->>Map: emplace(plate, ActiveParking)
        System-->>User: Slot allocated + entry time
    end
```

#### 4.3b. Vehicle Exit Flow

```mermaid
sequenceDiagram
    actor User
    participant System as SmartParkingSystem
    participant TM as TariffManager
    participant History as vector ParkingTransaction
    participant Slots as vector ParkingSlot

    User->>System: processVehicleExit()
    System->>System: Find plate in activeVehicles
    System->>System: readValidExitTime (exit > entry)
    System->>System: calculateBillableHours (ceil)
    System->>TM: getRate(vehicleType) — current active price
    TM-->>System: rate at exit time
    System->>System: totalFee = hours × rate
    System->>Slots: setStatus(AVAILABLE)
    System->>History: push_back(ParkingTransaction)
    System->>System: erase from activeVehicles
    System-->>User: Exit receipt + fee
```

---

## 5. Data Structures & DSA Operations

```mermaid
flowchart TB
    subgraph Linear["Linear Structures"]
        VS["vector&lt;ParkingSlot&gt;<br/>INSERT: push_back<br/>UPDATE: setStatus<br/>TRAVERSE: display reports"]
        VH["vector&lt;ParkingTransaction&gt;<br/>INSERT: push_back on exit<br/>TRAVERSE: history & revenue"]
    end

    subgraph NonLinear["Non-Linear Structures"]
        SM["unordered_map Slot ID → index<br/>INSERT: on add slot<br/>LOOKUP: O(1) findSlotIndex"]
        VM["unordered_map plate → ActiveParking<br/>INSERT: emplace on entry<br/>DELETE: erase on exit<br/>LOOKUP: duplicate check"]
        TR["unordered_map VehicleType → rate<br/>UPDATE: updateRate<br/>LOOKUP: getRate at exit"]
    end
```

| Structure | Justification |
|-----------|---------------|
| `vector<ParkingSlot>` | Sequential storage; easy traversal for slot reports |
| `unordered_map<string, int>` | O(1) slot lookup by ID during exit and validation |
| `unordered_map<string, ActiveParking>` | O(1) duplicate-plate check and active session lookup |
| `vector<ParkingTransaction>` | Append-only history; preserves original rate per transaction |
| `unordered_map<VehicleType, double>` | Fast rate lookup and update per vehicle type |

---

## 6. Full Requirements Checklist

Legend: ✅ Implemented | ⚠️ Partial / Note

### A. Integrated Situation — System Must Manage

| # | Requirement | Status | Implementation |
|---|-------------|--------|----------------|
| A1 | Parking slot configuration and availability | ✅ | `addParkingSlots()`, `displayAvailableSlots()`, `SlotStatus` enum |
| A2 | Vehicle entry registration | ✅ | `registerVehicleEntry()` |
| A3 | Parking duration tracking and fee calculation | ✅ | `calculateBillableHours()`, `processVehicleExit()` |
| A4 | Vehicle exit and payment recording | ✅ | `processVehicleExit()`, `ParkingTransaction` |
| A5 | Report: available slots | ✅ | `displayAvailableSlots()` — Menu 5 |
| A6 | Report: parked vehicles | ✅ | `displayParkedVehicles()` — Menu 6 |
| A7 | Report: vehicle history | ✅ | `displayParkingHistory()` — Menu 7 |
| A8 | Report: daily revenue | ✅ | `displayDailyRevenue()` — Menu 8 |
| A9 | In-memory data structures only (no database) | ✅ | All data in `vector` / `unordered_map` |
| A10 | Efficient and scalable design | ✅ | O(1) hash map lookups, vector traversal for reports |
| A11 | Good OOP design practices | ✅ | See Section 7 below |

---

### B. Task 1 — Parking Slot Configuration

| # | Requirement / Attribute | Status | Implementation |
|---|-------------------------|--------|----------------|
| B1 | Slot ID (unique) | ✅ | `readValidSlotId()`, `slotIndexMap`, duplicate rejection |
| B2 | Supported vehicle type (Motorcycle / Car / Truck) | ✅ | `VehicleType` enum, `readVehicleType()` |
| B3 | Zone (location) | ✅ | `readValidZone()`, stored in `ParkingSlot` |
| B4 | Slot status (Available / Occupied) | ✅ | `SlotStatus` enum, updated on entry/exit |
| B5 | Slots uniquely identified | ✅ | `normalizeKey()` + `slotIndexMap` |
| B6 | Appropriate data structure for slots | ✅ | `vector<ParkingSlot>` + `unordered_map` index |

---

### C. Task 2 — Vehicle Entry Management

| # | Requirement / Attribute | Status | Implementation |
|---|-------------------------|--------|----------------|
| C1 | Vehicle plate number (unique) | ✅ | `readValidPlate()`, case-insensitive duplicate check |
| C2 | Vehicle type (Car, Truck, Motorcycle) | ✅ | `VehicleType` enum |
| C3 | Entry time | ✅ | Hour (0–23) + minute (0–59) with validation |
| C4 | Allocated parking slot | ✅ | Auto-allocated via `findAvailableSlot()` |
| C5 | Cannot park same vehicle twice at once | ✅ | `activeVehicles` map blocks duplicate plate |
| C6 | Graceful handling when no slot available | ✅ | Message + tip; no crash |
| C7 | Appropriate data structures for records | ✅ | `unordered_map` for active sessions, `vector` for slots |

---

### D. Task 3 — Duration & Fee Calculation

| # | Requirement | Status | Implementation |
|---|-------------|--------|----------------|
| D1 | Calculate duration from entry and exit time | ✅ | `timeToMinutes()`, `calculateBillableHours()` |
| D2 | Apply rates based on vehicle type | ✅ | `TariffManager::getRate()` |
| D3 | Compute total parking fee | ✅ | `billableHours × rate` in `processVehicleExit()` |
| D4 | Allow update of parking price | ✅ | `updateParkingPrices()` — Menu 3 |
| D5 | Price updates must not affect completed records | ✅ | `rateApplied` stored in `ParkingTransaction` at exit |

#### Task 3 Rules

| # | Rule | Status | Implementation |
|---|------|--------|----------------|
| R3.1 | Default hourly prices for each vehicle type | ✅ | `TariffManager` constructor |
| R3.2 | Controlled option to update prices while running | ✅ | Menu option 3 |
| R3.3 | Fees use current active prices at exit time | ✅ | `tariffs.getRate()` called during exit |
| R3.4 | Price updates do not change history | ✅ | History stores `rateApplied` per transaction |
| R3.5 | Motorcycle default: 500 RWF/hour | ✅ | `hourlyRates[MOTORCYCLE] = 500.0` |
| R3.6 | Car default: 1,000 RWF/hour | ✅ | `hourlyRates[CAR] = 1000.0` |
| R3.7 | Truck default rate | ⚠️ | Exam lists Motorcycle & Car only; Truck set to **2,000 RWF** (reasonable default) |
| R3.8 | Fees calculated only at exit | ✅ | No fee on entry; fee in `processVehicleExit()` only |
| R3.9 | Partial hours charged as full hours | ✅ | `ceil(duration / 60.0)` — 15 min → 1 hr, 1h20 → 2 hr |

---

### E. Task 4 — Vehicle Exit & Parking Update

| # | Requirement | Status | Implementation |
|---|-------------|--------|----------------|
| E1 | Release occupied parking slot | ✅ | `slots[slotIndex].setStatus(AVAILABLE)` |
| E2 | Calculate and display parking fee | ✅ | Exit receipt with hours, rate, total |
| E3 | Update all relevant system records | ✅ | Slot status, remove from active, add to history |
| E4 | Store transaction for future reference | ✅ | `history.push_back(ParkingTransaction(...))` |

---

### F. Design & DSA Requirements

| # | Requirement | Status | Implementation |
|---|-------------|--------|----------------|
| F1 | Design system architecture | ✅ | This document + layered design in code |
| F2 | Identify core system components | ✅ | Section 3 above |
| F3 | Linear and/or non-linear data structures | ✅ | `vector` (linear) + `unordered_map` (hash table) |
| F4 | Justify data structure choices | ✅ | Comments in `2026_exam.cpp` header + Section 5 above |
| F5 | Insertion operations | ✅ | `push_back`, `emplace` |
| F6 | Deletion operations | ✅ | `activeVehicles.erase()` on exit |
| F7 | Update operations | ✅ | `setStatus()`, `updateRate()` |
| F8 | Traversal operations | ✅ | All `display*()` report functions |
| F9 | Encapsulation | ✅ | Private fields in classes, `TariffManager` |
| F10 | Abstraction | ✅ | `Printable` abstract base class |
| F11 | Inheritance | ✅ | `ParkingSlot`, `ParkingTransaction` extend `Printable` |
| F12 | Polymorphism | ✅ | Virtual `display()` overridden in subclasses |
| F13 | Validate inputs | ✅ | Plate, slot ID, zone, menu, time, rate validators |
| F14 | Handle exceptional cases gracefully | ✅ | Empty data, no slots, not found, exit before entry |
| F15 | Menu-driven / console interface | ✅ | 12-option menu in `main()` |
| F16 | Test inputs provided | ✅ | `README.md` sample flow + `test_validation.sh` |
| F17 | README with compile/run, rates, menu | ✅ | `README.md` |
| F18 | No internet required | ✅ | Standalone C++, no external dependencies |

---

### G. Extra Validation (Beyond Minimum — for Full Marks)

| # | Enhancement | Status | Implementation |
|---|-------------|--------|----------------|
| G1 | Reject non-numeric menu input | ✅ | `readMenuChoice()` loop |
| G2 | Reject out-of-range menu choice | ✅ | Range check 1–12 |
| G3 | Reject invalid plate format | ✅ | `isValidPlate()` — 3–15 alphanumeric |
| G4 | Reject invalid slot ID (symbols) | ✅ | `isValidSlotId()` |
| G5 | Reject empty input | ✅ | `readNonEmptyLine()` |
| G6 | Reject exit time before entry | ✅ | `readValidExitTime()` |
| G7 | Case-insensitive plate lookup | ✅ | `normalizeKey()` |
| G8 | Vehicle type must match slot type | ✅ | `findAvailableSlot()` + `supports()` |
| G9 | Max slot limit | ✅ | `MAX_SLOTS = 100` |
| G10 | Slot status double-check before entry | ✅ | Re-verify `isAvailable()` before occupy |

---

## 7. OOP Mapping (Quick Reference)

```mermaid
classDiagram
    class Printable {
        <<abstract>>
        +display() void
    }

    class ParkingSlot {
        -slotId string
        -supportedType VehicleType
        -zone string
        -status SlotStatus
        +setStatus()
        +isAvailable() bool
        +supports() bool
        +display() void
    }

    class ParkingTransaction {
        -plateNumber string
        -rateApplied double
        -totalFee double
        +display() void
    }

    class ActiveParking {
        -plateNumber string
        -entryHour int
        -slotId string
        +display() void
    }

    class TariffManager {
        -hourlyRates map
        +getRate() double
        +updateRate() void
    }

    class SmartParkingSystem {
        -slots vector
        -activeVehicles map
        -history vector
        -tariffs TariffManager
        +addParkingSlots()
        +registerVehicleEntry()
        +updateParkingPrices()
        +processVehicleExit()
    }

    Printable <|-- ParkingSlot
    Printable <|-- ParkingTransaction
    SmartParkingSystem --> ParkingSlot
    SmartParkingSystem --> ActiveParking
    SmartParkingSystem --> ParkingTransaction
    SmartParkingSystem --> TariffManager
```

---

## 8. Summary Scorecard

| Category | Total Items | Implemented | Notes |
|----------|-------------|-------------|-------|
| Integrated situation (A) | 11 | 11 ✅ | All covered |
| Task 1 — Slots (B) | 6 | 6 ✅ | |
| Task 2 — Entry (C) | 7 | 7 ✅ | |
| Task 3 — Fees (D + Rules) | 14 | 13 ✅ / 1 ⚠️ | Truck rate not in exam text |
| Task 4 — Exit (E) | 4 | 4 ✅ | |
| Design & DSA (F) | 18 | 18 ✅ | |
| Extra validation (G) | 10 | 10 ✅ | Bonus robustness |
| **Overall** | **70** | **69 ✅ / 1 ⚠️** | **~99% exam coverage** |

> **Only note:** The exam specifies default tariffs for Motorcycle (500) and Car (1,000) but includes Truck as a vehicle type without a stated rate. Our implementation uses **2,000 RWF/hour** for Truck. If your examiner asks, explain that Truck is supported as a type and given a logical default rate.
