#include "qualification_validation.hpp"

#include <cstdio>
#include <string>

static dtype_special_x::QualificationRecord MakeRecord()
{
    dtype_special_x::QualificationRecord record;
    record.route = "DTYPE-SPECIAL-X";
    record.revision = "V001";
    record.mode = "noise-floor";
    record.implementation = "parent";
    record.qualification = "PASS";
    record.shapeIndex = "0";
    record.device = "4";
    record.owner = "MAIN-1";
    record.leaseId = "lease-B";
    record.preflightUtc = "2026-09-25T12:00:00Z";
    record.runnerSha = "runner-sha";
    record.parentSourceSha = "parent-sha";
    record.candidateSourceSha = "candidate-sha";
    return record;
}

static dtype_special_x::QualificationExpectation MakeExpectation()
{
    return {0, 4, "MAIN-1", "lease-B", "2026-09-25T12:00:00Z",
            "runner-sha", "parent-sha", "candidate-sha"};
}

static bool ExpectResult(const char* name,
                         const dtype_special_x::QualificationRecord& record,
                         const dtype_special_x::QualificationExpectation& expected,
                         const std::string& expectedMessage)
{
    const std::string actual = dtype_special_x::ValidateQualificationRecord(record, expected);
    if (actual != expectedMessage) {
        std::fprintf(stderr, "FAIL %s expected='%s' actual='%s'\n",
                     name, expectedMessage.c_str(), actual.c_str());
        return false;
    }
    std::printf("PASS %s result=%s\n", name,
                actual.empty() ? "ACCEPT" : actual.c_str());
    return true;
}

int main()
{
    const auto expected = MakeExpectation();
    auto record = MakeRecord();
    int failures = 0;
    int cases = 0;

    const auto run = [&](const char* name,
                         const dtype_special_x::QualificationRecord& candidate,
                         const std::string& message) {
        ++cases;
        if (!ExpectResult(name, candidate, expected, message)) ++failures;
    };

    run("matching_identity", record, "");
    record.device = "5";
    run("device_mismatch", record, "paired qualification mismatch: device");
    record = MakeRecord();
    record.leaseId = "lease-A";
    run("lease_id_mismatch", record, "paired qualification mismatch: lease_id");
    record = MakeRecord();
    record.owner = "MAIN-2";
    run("owner_mismatch", record, "paired qualification mismatch: owner");
    record = MakeRecord();
    record.runnerSha = "other-runner";
    run("runner_sha_mismatch", record, "paired qualification mismatch: runner_sha256");
    record = MakeRecord();
    record.parentSourceSha = "other-parent";
    run("parent_source_sha_mismatch", record,
        "paired qualification mismatch: parent_source_sha256");
    record = MakeRecord();
    record.candidateSourceSha = "other-candidate";
    run("candidate_source_sha_mismatch", record,
        "paired qualification mismatch: candidate_source_sha256");
    record = MakeRecord();
    record.preflightUtc = "2026-09-25T11:55:00Z";
    run("preflight_exactly_300_seconds_old", record, "");
    record = MakeRecord();
    record.preflightUtc = "2026-09-25T11:54:59Z";
    run("preflight_expired", record,
        "paired qualification mismatch: preflight_utc is older than current preflight by more than 300 seconds");
    record = MakeRecord();
    record.preflightUtc = "2026-09-25T12:00:01Z";
    run("preflight_from_future", record,
        "paired qualification mismatch: preflight_utc is newer than current preflight");

    std::printf("qualification_negative_validation=%s cases=%d failures=%d\n",
                failures == 0 ? "PASS" : "FAIL", cases, failures);
    return failures == 0 ? 0 : 1;
}
