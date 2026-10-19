#include "runner_validation.h"

#include <cassert>
#include <sstream>

namespace {

const char* kHeader = "device\towner\troute\tlease_id\tstatus\tstart_time\tend_time\tnote\n";

std::string LeaseRow(const char* device, const char* owner, const char* route,
                     const char* leaseId, const char* status)
{
    return std::string(device) + '\t' + owner + '\t' + route + '\t' + leaseId + '\t' +
           status + "\t2026-09-25T00:00:00Z\t-\ttest\n";
}

bool Validate(const std::string& rows, int device = 4,
              const std::string& owner = "MAIN-1",
              const std::string& leaseId = "MIX-A-TEST-01")
{
    std::istringstream input(std::string(kHeader) + rows);
    mix_a_runner::LeaseIdentity identity;
    std::string error;
    const bool valid = mix_a_runner::ValidateLeaseTable(
        input, device, owner, leaseId, &identity, &error);
    if (valid) {
        assert(identity.device == device);
        assert(identity.owner == owner);
        assert(identity.route == "MIX-A");
        assert(identity.leaseId == leaseId);
        assert(identity.status == "LEASED");
    } else {
        assert(!error.empty());
    }
    return valid;
}

}  // namespace

int main()
{
    int device = -1;
    assert(mix_a_runner::ParseServerDevice("0", &device) && device == 0);
    assert(mix_a_runner::ParseServerDevice("6", &device) && device == 6);
    assert(mix_a_runner::ParseServerDevice("7", &device) && device == 7);
    assert(!mix_a_runner::IsTimingDeviceAllowed(7));
    assert(!mix_a_runner::ParseServerDevice("-1", &device));
    assert(!mix_a_runner::ParseServerDevice("8", &device));
    assert(!mix_a_runner::ParseServerDevice("2147483647", &device));
    assert(!mix_a_runner::ParseServerDevice("2147483648", &device));
    assert(!mix_a_runner::ParseServerDevice("9223372036854775808", &device));
    assert(!mix_a_runner::ParseServerDevice("4x", &device));
    assert(!mix_a_runner::ParseServerDevice("", &device));

    const std::string active = LeaseRow("4", "MAIN-1", "MIX-A", "MIX-A-TEST-01", "LEASED");
    const std::string releasedPriorLease =
        LeaseRow("4", "MAIN-2", "OTHER-ROUTE", "OTHER-D4-01", "LEASED") +
        LeaseRow("4", "MAIN-2", "OTHER-ROUTE", "OTHER-D4-01", "RELEASED");
    assert(Validate(active));
    assert(Validate(releasedPriorLease + active));
    assert(!Validate(active, 5));
    assert(!Validate(active, 4, "MAIN-2"));
    assert(!Validate(active, 4, "MAIN-1", "STALE-LEASE"));
    assert(!Validate(LeaseRow("4", "MAIN-1", "MIX-A", "MIX-A-TEST-01", "RELEASED")));
    assert(!Validate(active + LeaseRow("4", "MAIN-2", "OTHER-ROUTE", "OTHER-01", "LEASED")));
    assert(!Validate(releasedPriorLease + active +
                     LeaseRow("4", "MAIN-2", "OTHER-ROUTE", "OTHER-D4-02", "LEASED")));
    assert(!Validate(active + LeaseRow("5", "MAIN-1", "MIX-A", "MIX-A-TEST-02", "LEASED")));
    assert(!Validate(LeaseRow("7", "MAIN-1", "MIX-A", "MIX-A-TEST-01", "LEASED"), 7));

    std::istringstream badHeader("device\towner\tstatus\n");
    mix_a_runner::LeaseIdentity identity;
    std::string error;
    assert(!mix_a_runner::ValidateLeaseTable(
        badHeader, 4, "MAIN-1", "MIX-A-TEST-01", &identity, &error));
    assert(!error.empty());
    return 0;
}
