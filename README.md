# Kigali Smart Parking Management System (2026 Exam)
<img width="792" height="513" alt="Screenshot From 2026-06-11 10-51-27" src="https://github.com/user-attachments/assets/0a22686e-52e6-4b6e-b1ce-c479df7585ce" />


## Compile and Run

```bash
g++ -o parking 2026_exam.cpp -std=c++11
./parking
```

## Default Parking Rates

| Vehicle Type | Rate (RWF/hour) |
|--------------|-----------------|
| Motorcycle   | 500             |
| Car          | 1,000           |
| Truck        | 2,000           |

Fees are calculated only at exit. Partial hours are billed as full hours (e.g. 15 min = 1 hour, 1h 20min = 2 hours).

## Menu Options

| Option | Description |
|--------|-------------|
| 1 | Add parking slot(s) with unique ID, vehicle type, and zone |
| 2 | Register vehicle entry and auto-allocate a matching available slot |
| 3 | Update hourly parking price (does not change completed history) |
| 4 | Process vehicle exit, calculate fee, release slot, store transaction |
| 5 | View all available slots |
| 6 | View currently parked vehicles |
| 7 | View completed parking history |
| 8 | View daily revenue (total from completed transactions) |
| 9 | Display all configured slots |
| 10 | Display current tariffs |
| 11 | Display all system data |
| 12 | Exit program |

## Sample Test Flow

1. **Add slots (Option 1)** — add 2 slots:
   - `M1`, Motorcycle, Zone A
   - `C1`, Car, Zone B

2. **Register entry (Option 2)** — plate `RAB123A`, Car, entry time `08:00`

3. **Exit (Option 4)** — plate `RAB123A`, exit time `09:15` → billed 2 hours × 1,000 = **2,000 RWF**

4. **Update price (Option 3)** — change Car rate to 1,500 RWF/hour

5. **Register and exit another car** — verify new rate applies; old history still shows 1,000 RWF rate

6. **Try duplicate entry** — register same plate twice → rejected

7. **No slot available** — fill all Car slots, try another Car entry → graceful message

## Data Structures Used

- `vector<ParkingSlot>` — store and traverse all slots
- `unordered_map<string, int>` — O(1) slot lookup by Slot ID
- `unordered_map<string, ActiveParking>` — O(1) active vehicle lookup by plate
- `vector<ParkingTransaction>` — append-only completed transaction history

## OOP Features

- **Encapsulation** — `TariffManager`, `ParkingSlot`, `ActiveParking`
- **Abstraction** — `Printable` interface for display operations
- **Inheritance & Polymorphism** — `ParkingSlot` and `ParkingTransaction` override `display()`

## Architecture & Requirements Checklist

See **[ARCHITECTURE.md](ARCHITECTURE.md)** for:

- Mermaid system architecture diagrams
- **Data Flow Diagrams** (Context Level 0, Level 1, end-to-end lifecycle)
- Component responsibility map
- Entry/exit sequence diagrams
- Data structure justification
- **Full exam requirements checklist** (every task, rule, and design requirement marked ✅/⚠️)

## Run Validation Tests

```bash
bash test_validation.sh
```

Tests menu input errors, invalid plates/slots, duplicate entries, fee calculation, and exit-time rules.
