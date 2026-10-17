#ifndef MIX_A_RUNNER_VALIDATION_H
#define MIX_A_RUNNER_VALIDATION_H

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <istream>
#include <string>
#include <vector>

namespace mix_a_runner {

constexpr int kServerDeviceCount = 8;
constexpr int kTimingDeviceCount = 7;

struct LeaseIdentity {
    int device = -1;
    std::string owner;
    std::string route;
    std::string leaseId;
    std::string status;
};

inline bool ParseServerDevice(const std::string& text, int* device)
{
    if (text.empty() || device == nullptr) return false;
    for (char digit : text) {
        if (digit < '0' || digit > '9') return false;
    }
    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(text.c_str(), &end, 10);
    if (errno != 0 || end == text.c_str() || *end != '\0' ||
        parsed < 0 || parsed > INT_MAX || parsed >= kServerDeviceCount) {
        return false;
    }
    *device = static_cast<int>(parsed);
    return true;
}

inline bool IsTimingDeviceAllowed(int device)
{
    return device >= 0 && device < kTimingDeviceCount;
}

inline std::vector<std::string> SplitTsv(const std::string& line)
{
    std::vector<std::string> fields;
    size_t start = 0;
    for (;;) {
        const size_t tab = line.find('\t', start);
        if (tab == std::string::npos) {
            fields.push_back(line.substr(start));
            return fields;
        }
        fields.push_back(line.substr(start, tab - start));
        start = tab + 1;
    }
}

inline bool ValidateLeaseTable(std::istream& input, int requestedDevice,
                               const std::string& requestedOwner,
                               const std::string& requestedLeaseId,
                               LeaseIdentity* identity, std::string* error)
{
    const auto fail = [error](const char* message) {
        if (error != nullptr) *error = message;
        return false;
    };
    if (identity == nullptr || requestedDevice < 0 ||
        requestedDevice >= kServerDeviceCount || !IsTimingDeviceAllowed(requestedDevice)) {
        return fail("device is outside the allowed timing domain");
    }
    if (requestedOwner.size() <= 5 || requestedOwner.compare(0, 5, "MAIN-") != 0 ||
        requestedLeaseId.empty()) {
        return fail("Main owner and lease ID are required");
    }

    std::string line;
    if (!std::getline(input, line)) return fail("lease table is empty");
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const std::vector<std::string> header = SplitTsv(line);
    size_t deviceColumn = header.size(), ownerColumn = header.size();
    size_t routeColumn = header.size(), leaseColumn = header.size(), statusColumn = header.size();
    for (size_t i = 0; i < header.size(); ++i) {
        if (header[i] == "device") deviceColumn = i;
        if (header[i] == "owner") ownerColumn = i;
        if (header[i] == "route") routeColumn = i;
        if (header[i] == "lease_id") leaseColumn = i;
        if (header[i] == "status") statusColumn = i;
    }
    if (deviceColumn == header.size() || ownerColumn == header.size() ||
        routeColumn == header.size() || leaseColumn == header.size() ||
        statusColumn == header.size()) {
        return fail("lease table header is missing required columns");
    }

    size_t activeMixALeases = 0;
    LeaseIdentity activeMixA;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        const std::vector<std::string> fields = SplitTsv(line);
        const size_t lastColumn = std::max(
            std::max(deviceColumn, ownerColumn), std::max(routeColumn,
                std::max(leaseColumn, statusColumn)));
        if (fields.size() <= lastColumn) return fail("lease table row is incomplete");
        if (fields[statusColumn] != "LEASED") continue;

        int activeDevice = -1;
        if (!ParseServerDevice(fields[deviceColumn], &activeDevice)) {
            return fail("active lease has an invalid device ID");
        }
        const std::string& owner = fields[ownerColumn];
        const std::string& route = fields[routeColumn];
        const std::string& leaseId = fields[leaseColumn];
        if (activeDevice == requestedDevice &&
            (route != "MIX-A" || owner != requestedOwner || leaseId != requestedLeaseId)) {
            return fail("requested device is leased to a different owner or route");
        }
        if (route == "MIX-A") {
            ++activeMixALeases;
            activeMixA = {activeDevice, owner, route, leaseId, fields[statusColumn]};
        }
    }
    if (activeMixALeases != 1) return fail("lease table must contain exactly one active MIX-A lease");
    if (activeMixA.device != requestedDevice || activeMixA.owner != requestedOwner ||
        activeMixA.leaseId != requestedLeaseId || activeMixA.status != "LEASED") {
        return fail("requested device, Main owner, or lease ID does not match the active MIX-A lease");
    }
    *identity = activeMixA;
    if (error != nullptr) error->clear();
    return true;
}

}  // namespace mix_a_runner

#endif
