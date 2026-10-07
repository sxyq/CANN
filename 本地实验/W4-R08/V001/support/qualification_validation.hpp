#ifndef W4R08_QUALIFICATION_VALIDATION_HPP
#define W4R08_QUALIFICATION_VALIDATION_HPP

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace w4r08 {

struct QualificationRecord {
    std::string route;
    std::string revision;
    std::string mode;
    std::string implementation;
    std::string qualification;
    std::string shape;
    std::string dtype;
    std::string device;
    std::string owner;
    std::string leaseId;
    std::string preflightUtc;
    std::string runnerSha;
    std::string parentSourceSha;
    std::string candidateSourceSha;
};

struct QualificationExpectation {
    std::string shape;
    std::string dtype;
    int device;
    std::string owner;
    std::string leaseId;
    std::string preflightUtc;
    std::string runnerSha;
    std::string parentSourceSha;
    std::string candidateSourceSha;
};

inline bool ParseQualificationUtc(const std::string& text, std::time_t* value)
{
    std::tm parsed = {};
    std::istringstream input(text);
    input >> std::get_time(&parsed, "%Y-%m-%dT%H:%M:%SZ");
    if (input.fail() || input.peek() != std::char_traits<char>::eof()) {
        return false;
    }
    *value = timegm(&parsed);
    return *value != static_cast<std::time_t>(-1);
}

inline std::string ValidateQualificationRecord(
    const QualificationRecord& record,
    const QualificationExpectation& expected)
{
    if (record.route != "W4-R08") return "paired qualification mismatch: route";
    if (record.revision != "V001") return "paired qualification mismatch: revision";
    if (record.mode != "noise-floor") return "paired qualification mismatch: mode";
    if (record.implementation != "parent") return "paired qualification mismatch: implementation";
    if (record.qualification != "PASS") return "paired qualification mismatch: verdict";
    if (record.shape != expected.shape) return "paired qualification mismatch: shape";
    if (record.dtype != expected.dtype) return "paired qualification mismatch: dtype";
    if (record.device != std::to_string(expected.device)) {
        return "paired qualification mismatch: device";
    }
    if (record.owner != expected.owner) return "paired qualification mismatch: owner";
    if (record.leaseId != expected.leaseId) return "paired qualification mismatch: lease_id";
    if (record.runnerSha != expected.runnerSha) {
        return "paired qualification mismatch: runner_sha256";
    }
    if (record.parentSourceSha != expected.parentSourceSha) {
        return "paired qualification mismatch: parent_source_sha256";
    }
    if (record.candidateSourceSha != expected.candidateSourceSha) {
        return "paired qualification mismatch: candidate_source_sha256";
    }
    if (record.preflightUtc != expected.preflightUtc) {
        return "paired qualification mismatch: preflight_utc";
    }
    return "";
}

}  // namespace w4r08

#endif
