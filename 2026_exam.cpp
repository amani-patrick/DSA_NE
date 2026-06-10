/*
 * Kigali Smart Parking Management System - 2026 National Exam
 *
 * DATA STRUCTURES USED :
 *   vector<ParkingSlot>                   -> store all slots, easy traversal for reports
 *   unordered_map<string, int>            -> O(1) slot lookup by Slot ID
 *   unordered_map<string, ActiveParking>  -> O(1) check if plate is already parked
 *   vector<ParkingTransaction>            -> append-only history
 *
 * OOP Concepts:
 *   Printable (abstract) -> polymorphic display()
 *   ParkingSlot, ParkingTransaction inherit Printable
 *   TariffManager encapsulates pricing rules
 *   SmartParkingSystem orchestrates all business logic
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <regex>


using namespace std;

// ============================================================
// ENUMS - fixed set of values 
// ============================================================

enum VehicleType { MOTORCYCLE = 1, CAR = 2, TRUCK = 3 };
enum SlotStatus { AVAILABLE = 1, OCCUPIED = 2 };

string vehicleTypeToString(VehicleType type)
{
    switch(type)
    {
        case MOTORCYCLE: return "Motorcycle";
        case CAR:        return "Car";
        case TRUCK:      return "Truck";
        default:         return "Unknown";
    }
}

string slotStatusToString(SlotStatus status)
{
    return (status == AVAILABLE) ? "Available" : "Occupied";
}

// ============================================================
// VALIDATION HELPERS
// Inputs that are not letters instead of numbers, empty input,
// wrong ranges, or mismatched data - these functions block that.
// ============================================================

// Remove leading/trailing spaces 
string trim(string str)
{
    while(!str.empty() && isspace(str.front()))
        str.erase(str.begin());

    while(!str.empty() && isspace(str.back()))
        str.pop_back();

    return str;
}

// Uppercase + trim so "rab123a" matches "RAB123A" 
string normalizeKey(string value)
{
    value = trim(value);
    transform(value.begin(), value.end(), value.begin(), ::toupper);
    return value;
}

void clearInputBuffer()
{
    cin.clear();
    cin.ignore(10000, '\n');
}

// Only letters and digits allowed (blocks symbols like @, #, spaces inside ID)
bool isAlphanumeric(string value)
{
    if(value.empty())
        return false;

    for(size_t i = 0; i < value.size(); i++)
    {
        if(!isalnum(value[i]))
            return false;
    }

    return true;
}

// Plate: 3-15 alphanumeric characters
bool isValidPlate(string plate)
{
    plate = trim(plate);

    if(plate.size() < 3 || plate.size() > 15){

        return false;
    }
    regex pattern("^[R][A-Za-z]{2}[0-9]{3}[A-Z]$");

    // return isAlphanumeric(plate);
     return isAlphanumeric(plate) && regex_match(plate, pattern);

}



// Slot ID: 1-10 alphanumeric characters
bool isValidSlotId(string slotId)
{
    slotId = trim(slotId);

    if(slotId.size() < 1 || slotId.size() > 10)
        return false;

    return isAlphanumeric(slotId);
}

// Zone name: non-empty, max 30 chars
bool isValidZone(string zone)
{
    zone = trim(zone);
    return !zone.empty() && zone.size() <= 30;
}

/*
 * Keeps asking until user enters a valid integer in [minVal, maxVal].
 * Handles: "abc", "12.5", empty line after failed read, out-of-range numbers.
 */
int readIntInRange(string prompt, int minVal, int maxVal)
{
    int value;

    while(true)
    {
        cout << prompt;

        if(cin >> value)
        {
            if(value >= minVal && value <= maxVal)
            {
                clearInputBuffer();
                return value;
            }

            cout << "Out of range. Enter a whole number between "
                 << minVal << " and " << maxVal << ".\n";
        }
        else
        {
            cout << "Invalid input. Numbers only (example: "
                 << minVal << "). Try again.\n";
        }

        clearInputBuffer();
    }
}

/*
 * Keeps asking until user enters a positive number (for parking rates).
 * Rejects: 0, negative, text like "free", decimals are OK (e.g. 1000.5).
 */
double readPositiveDouble(string prompt)
{
    double value;

    while(true)
    {
        cout << prompt;

        if(cin >> value && value > 0)
        {
            clearInputBuffer();
            return value;
        }

        cout << "Invalid input. Enter a positive number greater than 0.\n";
        clearInputBuffer();
    }
}

// Keeps asking until user types a non-empty line
string readNonEmptyLine(string prompt)
{
    string line;

    while(true)
    {
        cout << prompt;
        getline(cin, line);
        line = trim(line);

        if(!line.empty())
            return line;

        cout << "Input cannot be empty. Please try again.\n";
    }
}

// Forward declaration (readValidPlate needs ActiveParking type)
class ActiveParking;

// Validates plate format with retry loop
string readValidPlate(string prompt, bool checkDuplicate,
                      const unordered_map<string, ActiveParking> &activeVehicles);

string readValidSlotId(const unordered_map<string, int> &slotIndexMap)
{
    while(true)
    {
        string slotId = readNonEmptyLine("Enter Slot ID: ");

        if(!isValidSlotId(slotId))
        {
            cout << "Invalid Slot ID. Use 1-10 letters/numbers only (example: C1, M02).\n";
            continue;
        }

        if(slotIndexMap.find(normalizeKey(slotId)) != slotIndexMap.end())
        {
            cout << "Slot ID already exists. Enter a different ID.\n";
            continue;
        }

        return slotId;
    }
}

string readValidZone()
{
    while(true)
    {
        string zone = readNonEmptyLine("Enter Zone: ");

        if(!isValidZone(zone))
        {
            cout << "Invalid zone. Enter a name up to 30 characters.\n";
            continue;
        }

        return zone;
    }
}

VehicleType readVehicleType(string prompt)
{
    cout << prompt;
    cout << "1. Motorcycle\n";
    cout << "2. Car\n";
    cout << "3. Truck\n";

    int choice = readIntInRange("Select vehicle type (1-3): ", 1, 3);
    return static_cast<VehicleType>(choice);
}

bool isValidTime(int hour, int minute)
{
    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

int timeToMinutes(int hour, int minute)
{
    return hour * 60 + minute;
}

string formatTime(int hour, int minute)
{
    ostringstream oss;
    oss << setfill('0') << setw(2) << hour << ":"
        << setfill('0') << setw(2) << minute;
    return oss.str();
}

/*
 * Exam rule: partial hours = full hours (ceil).
 *   15 min  -> 1 hour
 *   1h 20m  -> 2 hours
 * Returns -1 if exit is before entry (invalid business case).
 */
int calculateBillableHours(int entryMinutes, int exitMinutes)
{
    int duration = exitMinutes - entryMinutes;

    if(duration <= 0)
        return -1;

    return static_cast<int>(ceil(duration / 60.0));
}

// Reads exit time and ensures it is strictly after entry time
bool readValidExitTime(int entryHour, int entryMinute, int &exitHour, int &exitMinute)
{
    int entryTotal = timeToMinutes(entryHour, entryMinute);

    while(true)
    {
        exitHour = readIntInRange("Enter exit hour (0-23): ", 0, 23);
        exitMinute = readIntInRange("Enter exit minute (0-59): ", 0, 59);

        if(!isValidTime(exitHour, exitMinute))
        {
            cout << "Invalid exit time.\n";
            continue;
        }

        int exitTotal = timeToMinutes(exitHour, exitMinute);

        if(exitTotal <= entryTotal)
        {
            cout << "Exit time must be AFTER entry time ("
                 << formatTime(entryHour, entryMinute) << "). Try again.\n";
            continue;
        }

        return true;
    }
}

// Menu choice: loop until valid (handles strings, floats, out-of-range)
int readMenuChoice(int minChoice, int maxChoice)
{
    while(true)
    {
        cout << "Enter your choice: ";

        int choice;

        if(!(cin >> choice))
        {
            cout << "Invalid input. Enter a whole number between "
                 << minChoice << " and " << maxChoice << ".\n";
            clearInputBuffer();
            continue;
        }

        clearInputBuffer();

        if(choice >= minChoice && choice <= maxChoice)
            return choice;

        cout << "Choice must be between " << minChoice
             << " and " << maxChoice << ". Try again.\n";
    }
}

// ============================================================
// OOP CLASSES
// ============================================================

// Abstract base class - polymorphism for display operations
class Printable
{
public:
    virtual void display() const = 0;
    virtual ~Printable() {}
};

// TASK 1 entity: one parking slot
class ParkingSlot : public Printable
{
private:
    string slotId;
    VehicleType supportedType;
    string zone;
    SlotStatus status;

public:
    ParkingSlot(string id, VehicleType type, string zoneName, SlotStatus slotStatus = AVAILABLE)
        : slotId(id), supportedType(type), zone(zoneName), status(slotStatus) {}

    string getSlotId() const { return slotId; }
    VehicleType getSupportedType() const { return supportedType; }
    string getZone() const { return zone; }
    SlotStatus getStatus() const { return status; }

    void setStatus(SlotStatus newStatus) { status = newStatus; }
    bool isAvailable() const { return status == AVAILABLE; }
    bool supports(VehicleType type) const { return supportedType == type; }

    void display() const
    {
        cout << left << setw(10) << slotId
             << setw(14) << vehicleTypeToString(supportedType)
             << setw(12) << zone
             << slotStatusToString(status) << endl;
    }
};

// Active session while vehicle is still inside the parking
class ActiveParking
{
private:
    string plateNumber;
    VehicleType vehicleType;
    int entryHour;
    int entryMinute;
    string slotId;

public:
    ActiveParking(string plate, VehicleType type, int hour, int minute, string slot)
        : plateNumber(plate), vehicleType(type), entryHour(hour),
          entryMinute(minute), slotId(slot) {}

    string getPlateNumber() const { return plateNumber; }
    VehicleType getVehicleType() const { return vehicleType; }
    int getEntryHour() const { return entryHour; }
    int getEntryMinute() const { return entryMinute; }
    string getSlotId() const { return slotId; }

    void display() const
    {
        cout << left << setw(12) << plateNumber
             << setw(14) << vehicleTypeToString(vehicleType)
             << setw(10) << formatTime(entryHour, entryMinute)
             << slotId << endl;
    }
};

// readValidPlate implementation (needs full ActiveParking definition)
string readValidPlate(string prompt, bool checkDuplicate,
                      const unordered_map<string, ActiveParking> &activeVehicles)
{
    while(true)
    {
        string plate = readNonEmptyLine(prompt);

        if(!isValidPlate(plate))
        {
            cout << "Invalid plate. Use 3-15 letters/numbers only and match the format RAB123C (example: RAB123A).\n";
            continue;
        }

        if(checkDuplicate)
        {
            string key = normalizeKey(plate);

            if(activeVehicles.find(key) != activeVehicles.end())
            {
                cout << "This vehicle is already parked. One active session per plate.\n";
                continue;
            }
        }

        return plate;
    }
}

// Completed transaction - rate is frozen here so later price updates do not change history
class ParkingTransaction : public Printable
{
private:
    string plateNumber;
    VehicleType vehicleType;
    string slotId;
    string zone;
    int entryHour, entryMinute;
    int exitHour, exitMinute;
    int billableHours;
    double rateApplied;
    double totalFee;

public:
    ParkingTransaction(string plate, VehicleType type, string slot, string zoneName,
                       int eHour, int eMinute, int xHour, int xMinute,
                       int hours, double rate, double fee)
        : plateNumber(plate), vehicleType(type), slotId(slot), zone(zoneName),
          entryHour(eHour), entryMinute(eMinute), exitHour(xHour), exitMinute(xMinute),
          billableHours(hours), rateApplied(rate), totalFee(fee) {}

    string getPlateNumber() const { return plateNumber; }
    VehicleType getVehicleType() const { return vehicleType; }
    double getTotalFee() const { return totalFee; }

    void display() const
    {
        cout << left << setw(12) << plateNumber
             << setw(14) << vehicleTypeToString(vehicleType)
             << setw(10) << slotId
             << setw(10) << zone
             << setw(10) << formatTime(entryHour, entryMinute)
             << setw(10) << formatTime(exitHour, exitMinute)
             << setw(8) << billableHours
             << fixed << setprecision(0) << setw(10) << rateApplied
             << setw(12) << totalFee << " RWF" << endl;
    }
};

// Encapsulates pricing - Task 3
class TariffManager
{
private:
    unordered_map<VehicleType, double> hourlyRates;

public:
    TariffManager()
    {
        hourlyRates[MOTORCYCLE] = 500.0;
        hourlyRates[CAR] = 1000.0;
        hourlyRates[TRUCK] = 2000.0;
    }

    double getRate(VehicleType type) const
    {
        return hourlyRates.at(type);
    }

    void updateRate(VehicleType type, double newRate)
    {
        hourlyRates[type] = newRate;
    }

    void displayRates() const
    {
        cout << "\nCurrent Parking Tariffs (RWF per hour):\n";
        cout << "  Motorcycle: " << fixed << setprecision(0) << hourlyRates.at(MOTORCYCLE) << "\n";
        cout << "  Car:        " << hourlyRates.at(CAR) << "\n";
        cout << "  Truck:      " << hourlyRates.at(TRUCK) << "\n";
    }
};

// ============================================================
// MAIN SYSTEM CLASS - all tasks + reports
// ============================================================

class SmartParkingSystem
{
private:
    vector<ParkingSlot> slots;
    unordered_map<string, int> slotIndexMap;           // normalized Slot ID -> vector index
    unordered_map<string, ActiveParking> activeVehicles; // normalized plate -> session
    vector<ParkingTransaction> history;
    TariffManager tariffs;

    static const int MAX_SLOTS = 100;

    int findSlotIndex(string slotId) const
    {
        auto it = slotIndexMap.find(normalizeKey(slotId));
        if(it == slotIndexMap.end())
            return -1;
        return it->second;
    }

    // First-fit search: first available slot matching vehicle type
    int findAvailableSlot(VehicleType type) const
    {
        for(size_t i = 0; i < slots.size(); i++)
        {
            if(slots[i].isAvailable() && slots[i].supports(type))
                return static_cast<int>(i);
        }
        return -1;
    }

    int countAvailableSlotsFor(VehicleType type) const
    {
        int count = 0;

        for(size_t i = 0; i < slots.size(); i++)
        {
            if(slots[i].isAvailable() && slots[i].supports(type))
                count++;
        }

        return count;
    }

public:
    // -------------------- TASK 1 --------------------
    void addParkingSlots()
    {
        if(slots.size() >= MAX_SLOTS)
        {
            cout << "Maximum slot limit (" << MAX_SLOTS << ") reached.\n";
            return;
        }

        int remaining = static_cast<int>(MAX_SLOTS - slots.size());
        int n = readIntInRange("Enter number of slots to add (max " + to_string(remaining) + "): ", 1, remaining);

        for(int i = 0; i < n; i++)
        {
            cout << "\n--- Slot " << (i + 1) << " ---\n";

            string slotId = readValidSlotId(slotIndexMap);
            VehicleType type = readVehicleType("");
            string zone = readValidZone();

            slots.push_back(ParkingSlot(slotId, type, zone, AVAILABLE));
            slotIndexMap[normalizeKey(slotId)] = static_cast<int>(slots.size()) - 1;

            cout << "Slot " << slotId << " added successfully.\n";
        }
    }

    void displayAllSlots() const
    {
        if(slots.empty())
        {
            cout << "No parking slots configured.\n";
            return;
        }

        cout << "\n"
             << left << setw(10) << "Slot ID"
             << setw(14) << "Vehicle Type"
             << setw(12) << "Zone"
             << "Status\n";
        cout << string(46, '-') << "\n";

        for(size_t i = 0; i < slots.size(); i++)
            slots[i].display();
    }

    // -------------------- TASK 2 --------------------
    void registerVehicleEntry()
    {
        if(slots.empty())
        {
            cout << "No parking slots configured. Use menu option 1 first.\n";
            return;
        }

        string plate = readValidPlate("Enter vehicle plate number: ", true, activeVehicles);
        VehicleType type = readVehicleType("");

        int availableCount = countAvailableSlotsFor(type);

        if(availableCount == 0)
        {
            cout << "No suitable parking slot available for "
                 << vehicleTypeToString(type) << ".\n";
            cout << "Tip: Add a slot for this vehicle type or wait for an exit.\n";
            return;
        }

        int slotIndex = findAvailableSlot(type);

        // Safety check - should never fail if availableCount > 0
        if(slotIndex == -1)
        {
            cout << "System error: slot allocation failed unexpectedly.\n";
            return;
        }

        int hour = readIntInRange("Enter entry hour (0-23): ", 0, 23);
        int minute = readIntInRange("Enter entry minute (0-59): ", 0, 59);

        if(!isValidTime(hour, minute))
        {
            cout << "Invalid entry time.\n";
            return;
        }

        string slotId = slots[slotIndex].getSlotId();

        // Double-check slot is still free before occupying (prevents logic mismatch)
        if(!slots[slotIndex].isAvailable())
        {
            cout << "Selected slot is no longer available. Registration cancelled.\n";
            return;
        }

        slots[slotIndex].setStatus(OCCUPIED);
        activeVehicles.emplace(normalizeKey(plate), ActiveParking(plate, type, hour, minute, slotId));

        cout << "\nVehicle registered successfully.\n";
        cout << "Allocated Slot: " << slotId
             << " (Zone: " << slots[slotIndex].getZone() << ")\n";
        cout << "Entry Time: " << formatTime(hour, minute) << "\n";
        cout << "Available " << vehicleTypeToString(type)
             << " slots remaining: " << (availableCount - 1) << "\n";
    }

    // -------------------- TASK 3 --------------------
    void updateParkingPrices()
    {
        VehicleType type = readVehicleType("Select vehicle type to update:\n");
        double oldRate = tariffs.getRate(type);
        double newRate = readPositiveDouble("Enter new hourly rate (RWF): ");

        if(newRate == oldRate)
        {
            cout << "Rate unchanged. Still " << fixed << setprecision(0)
                 << oldRate << " RWF/hour for " << vehicleTypeToString(type) << ".\n";
            return;
        }

        tariffs.updateRate(type, newRate);

        cout << vehicleTypeToString(type) << " rate updated from "
             << fixed << setprecision(0) << oldRate << " to "
             << newRate << " RWF/hour.\n";
        cout << "Note: Completed history records keep their original rate.\n";
    }

    void displayTariffs() const
    {
        tariffs.displayRates();
    }

    // -------------------- TASK 4 --------------------
    void processVehicleExit()
    {
        if(activeVehicles.empty())
        {
            cout << "No vehicles currently parked.\n";
            return;
        }

        string plate = readNonEmptyLine("Enter vehicle plate number: ");

        if(!isValidPlate(plate))
        {
            cout << "Invalid plate format.\n";
            return;
        }

        auto it = activeVehicles.find(normalizeKey(plate));

        if(it == activeVehicles.end())
        {
            cout << "Vehicle not found in active parking records.\n";
            cout << "Check the plate number or use option 6 to view parked vehicles.\n";
            return;
        }

        ActiveParking session = it->second;

        int exitHour, exitMinute;
        readValidExitTime(session.getEntryHour(), session.getEntryMinute(), exitHour, exitMinute);

        int entryMinutes = timeToMinutes(session.getEntryHour(), session.getEntryMinute());
        int exitMinutes = timeToMinutes(exitHour, exitMinute);
        int billableHours = calculateBillableHours(entryMinutes, exitMinutes);

        if(billableHours == -1)
        {
            cout << "Cannot calculate fee: exit time is not after entry time.\n";
            return;
        }

        double rate = tariffs.getRate(session.getVehicleType());
        double totalFee = billableHours * rate;

        int slotIndex = findSlotIndex(session.getSlotId());

        if(slotIndex == -1)
        {
            cout << "Warning: slot record missing, but exit will still be processed.\n";
        }
        else
        {
            if(slots[slotIndex].getStatus() != OCCUPIED)
            {
                cout << "Warning: slot status mismatch corrected.\n";
            }
            slots[slotIndex].setStatus(AVAILABLE);
        }

        string zone = (slotIndex != -1) ? slots[slotIndex].getZone() : "Unknown";

        // Store transaction with rate at exit time (immune to future price changes)
        history.push_back(ParkingTransaction(
            session.getPlateNumber(), session.getVehicleType(), session.getSlotId(), zone,
            session.getEntryHour(), session.getEntryMinute(),
            exitHour, exitMinute, billableHours, rate, totalFee));

        activeVehicles.erase(it);

        cout << "\n--- Exit Receipt ---\n";
        cout << "Plate:          " << session.getPlateNumber() << "\n";
        cout << "Vehicle Type:   " << vehicleTypeToString(session.getVehicleType()) << "\n";
        cout << "Slot:           " << session.getSlotId() << " (" << zone << ")\n";
        cout << "Entry Time:     " << formatTime(session.getEntryHour(), session.getEntryMinute()) << "\n";
        cout << "Exit Time:      " << formatTime(exitHour, exitMinute) << "\n";
        cout << "Billable Hours: " << billableHours << "\n";
        cout << "Rate Applied:   " << fixed << setprecision(0) << rate << " RWF/hour\n";
        cout << "Total Fee:      " << totalFee << " RWF\n";
        cout << "Slot released successfully.\n";
    }

    // -------------------- REPORTS --------------------
    void displayAvailableSlots() const
    {
        if(slots.empty())
        {
            cout << "No parking slots configured.\n";
            return;
        }

        bool found = false;

        cout << "\nAvailable Slots:\n";
        cout << left << setw(10) << "Slot ID"
             << setw(14) << "Vehicle Type"
             << "Zone\n";
        cout << string(36, '-') << "\n";

        for(size_t i = 0; i < slots.size(); i++)
        {
            if(slots[i].isAvailable())
            {
                cout << left << setw(10) << slots[i].getSlotId()
                     << setw(14) << vehicleTypeToString(slots[i].getSupportedType())
                     << slots[i].getZone() << "\n";
                found = true;
            }
        }

        if(!found)
            cout << "No available slots. All slots are occupied.\n";
    }

    void displayParkedVehicles() const
    {
        if(activeVehicles.empty())
        {
            cout << "No vehicles currently parked.\n";
            return;
        }

        cout << "\nParked Vehicles:\n";
        cout << left << setw(12) << "Plate"
             << setw(14) << "Vehicle Type"
             << setw(10) << "Entry"
             << "Slot ID\n";
        cout << string(46, '-') << "\n";

        for(auto it = activeVehicles.begin(); it != activeVehicles.end(); ++it)
            it->second.display();
    }

    void displayParkingHistory() const
    {
        if(history.empty())
        {
            cout << "No completed parking transactions.\n";
            return;
        }

        cout << "\nParking History:\n";
        cout << left << setw(12) << "Plate"
             << setw(14) << "Type"
             << setw(10) << "Slot"
             << setw(10) << "Zone"
             << setw(10) << "Entry"
             << setw(10) << "Exit"
             << setw(8) << "Hours"
             << setw(10) << "Rate"
             << "Fee\n";
        cout << string(94, '-') << "\n";

        for(size_t i = 0; i < history.size(); i++)
            history[i].display();
    }

    void displayDailyRevenue() const
    {
        if(history.empty())
        {
            cout << "No revenue recorded yet.\n";
            return;
        }

        double total = 0.0;

        cout << "\nRevenue Report (all completed transactions):\n";
        cout << left << setw(12) << "Plate"
             << setw(14) << "Type"
             << "Fee (RWF)\n";
        cout << string(36, '-') << "\n";

        for(size_t i = 0; i < history.size(); i++)
        {
            cout << left << setw(12) << history[i].getPlateNumber()
                 << setw(14) << vehicleTypeToString(history[i].getVehicleType())
                 << fixed << setprecision(0) << history[i].getTotalFee() << "\n";
            total += history[i].getTotalFee();
        }

        cout << string(36, '-') << "\n";
        cout << "Total Revenue: " << total << " RWF\n";
        cout << "Total Transactions: " << history.size() << "\n";
    }

    void displayAllData() const
    {
        displayAllSlots();
        displayTariffs();
        displayParkedVehicles();
        displayParkingHistory();
    }
};

// ============================================================
// MAIN - menu-driven interface (required by exam)
// ============================================================

int main()
{
    SmartParkingSystem system;
    int choice;

    do
    {
        cout << "\n========== KIGALI SMART PARKING SYSTEM ==========\n";
        cout << "1.  Configure parking slot(s)          \n";
        cout << "2.  Register vehicle entry             \n";
        cout << "3.  Update parking prices              \n";
        cout << "4.  Process vehicle exit               \n";
        cout << "5.  View available slots               \n";
        cout << "6.  View parked vehicles               \n";
        cout << "7.  View parking history               \n";
        cout << "8.  View daily revenue                 \n";
        cout << "9.  Display all slots\n";
        cout << "10. Display current tariffs\n";
        cout << "11. Display all system data\n";
        cout << "12. Exit\n";

        choice = readMenuChoice(1, 12);

        switch(choice)
        {
            case 1:  system.addParkingSlots();       break;
            case 2:  system.registerVehicleEntry();  break;
            case 3:  system.updateParkingPrices();   break;
            case 4:  system.processVehicleExit();    break;
            case 5:  system.displayAvailableSlots(); break;
            case 6:  system.displayParkedVehicles(); break;
            case 7:  system.displayParkingHistory(); break;
            case 8:  system.displayDailyRevenue();   break;
            case 9:  system.displayAllSlots();       break;
            case 10: system.displayTariffs();        break;
            case 11: system.displayAllData();          break;
            case 12: cout << "Program exited successfully.\n"; break;
            default: cout << "Invalid choice.\n";
        }

    } while(choice != 12);

    return 0;
}
