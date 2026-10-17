#!/usr/bin/env python3
"""Self-test for the route-owned w5ctl orchestration contract."""

import json
import os
import stat
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

sys.dont_write_bytecode = True

ROOT = Path(__file__).resolve().parents[3]
CLI = Path(__file__).resolve().parents[1] / "w5ctl"
ROUTE = "W5-R02-REGISTER-SPILL-ELIMINATION-X"
BRANCH = "research/w5-r02-register-spill"


def scope_args(repo_root=ROOT, route_root=None, route=ROUTE, branch=BRANCH):
    return [
        "--repo-root",
        str(repo_root),
        "--expected-branch",
        branch,
        "--route",
        route,
        "--route-root",
        str(route_root or CLI.parent),
    ]


def run_cli(*args):
    return run_cli_in(ROOT, *args)


def run_cli_in(cwd, *args):
    return run_cli_with_scope(cwd, scope_args(), *args)


def run_cli_with_scope(cwd, scope, *args):
    command = [sys.executable, "-B", str(CLI), args[0], *scope, *args[1:]]
    return subprocess.run(
        command,
        cwd=str(cwd),
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )


def write_executable(path, source):
    path.write_text(textwrap.dedent(source).lstrip())
    path.chmod(path.stat().st_mode | stat.S_IXUSR)


def write_raw(path, candidate=9.0, quality="CLEAN", pairs=4):
    rows = ["pair\tvariant\tlatency_us\tquality\tcontaminated"]
    for pair in range(1, pairs + 1):
        rows.append(f"{pair}\tparent\t10.0\t{quality}\tfalse")
        rows.append(f"{pair}\tcandidate\t{candidate}\t{quality}\tfalse")
    path.write_text("\n".join(rows) + "\n")


class W5CtlTest(unittest.TestCase):
    def test_invocation_binding_is_explicit_and_route_configurable(self):
        result = run_cli_with_scope(
            ROOT,
            scope_args(repo_root=ROOT, route_root=ROOT, route="W5-ALT-ROUTE"),
            "doctor",
            "--dry-run",
            "--source",
            str(ROOT / "线上结果/R31B/V011/submission.asc"),
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("ROUTE=W5-ALT-ROUTE", result.stdout)
        self.assertIn("REPO_ROOT_CHECK=PASS", result.stdout)

    def test_invocation_binding_is_required(self):
        result = subprocess.run(
            [
                sys.executable,
                "-B",
                str(CLI),
                "doctor",
                "--dry-run",
                "--source",
                str(ROOT / "线上结果/R31B/V011/submission.asc"),
            ],
            cwd=str(ROOT),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        self.assertEqual(result.returncode, 78, result.stdout + result.stderr)
        self.assertIn("EXPECTED_BRANCH_REQUIRED", result.stdout)

    def test_doctor_is_read_only_and_reports_unconfigured_backends(self):
        result = run_cli(
            "doctor",
            "--dry-run",
            "--source",
            str(ROOT / "线上结果/R31B/V011/submission.asc"),
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("DOCTOR_STATUS=PASS", result.stdout)
        self.assertIn("EXECUTION_READINESS=BLOCKED", result.stdout)
        self.assertIn("NO_SECOND_RUNNER=YES", result.stdout)
        self.assertIn("NO_SECOND_SCORER=YES", result.stdout)

    def test_backend_return_codes_and_sha_guards(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-test-", dir=str(CLI.parent)) as temp:
            root = Path(temp)
            source = root / "source.asc"
            artifact = root / "artifact.o"
            source.write_text("champion-source\n")
            fail = root / "fail.py"
            write_executable(
                fail,
                """
                #!/usr/bin/env python3
                import sys
                sys.exit(7)
                """,
            )
            result = run_cli(
                "build",
                "--backend",
                str(fail),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
            )
            self.assertEqual(result.returncode, 7, result.stdout + result.stderr)
            self.assertIn("BACKEND_RC=7", result.stdout)
            self.assertIn("STATUS=FAILED", result.stdout)
            self.assertNotIn("STATUS=PASS", result.stdout)

            mutate = root / "mutate.py"
            write_executable(
                mutate,
                """
                #!/usr/bin/env python3
                import os
                from pathlib import Path
                Path(os.environ["W5CTL_ARTIFACT"]).write_text("artifact")
                Path(os.environ["W5CTL_SOURCE"]).write_text("mutated")
                """,
            )
            result = run_cli(
                "build",
                "--backend",
                str(mutate),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
            )
            self.assertEqual(result.returncode, 65, result.stdout + result.stderr)
            self.assertIn("IDENTITY_STATUS=FAIL", result.stdout)

            source.write_text("champion-source\n")
            result = run_cli(
                "correctness",
                "--backend",
                str(fail),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
            )
            self.assertEqual(result.returncode, 7, result.stdout + result.stderr)
            self.assertIn("BACKEND_RC=7", result.stdout)

            raw = root / "raw.tsv"
            write_raw(raw)
            result = run_cli(
                "local",
                "--backend",
                str(fail),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
                "--raw",
                str(raw),
            )
            self.assertEqual(result.returncode, 7, result.stdout + result.stderr)
            self.assertIn("MEASUREMENT_QUALITY=BACKEND_FAILED", result.stdout)

            result = run_cli(
                "record",
                "--evidence",
                str(source),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
                "--raw",
                str(raw),
                "--manifest",
                str(root / "record-failed.json"),
                "--backend",
                str(fail),
            )
            self.assertEqual(result.returncode, 7, result.stdout + result.stderr)
            self.assertIn("EVIDENCE_SHA256=", result.stdout)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_WRITTEN", result.stdout)
            self.assertIn("STATUS=FAILED", result.stdout)

            result = run_cli(
                "build",
                "--backend",
                str(fail),
                "--source",
                "/tmp/w5ctl-outside-source.asc",
                "--artifact",
                str(artifact),
            )
            self.assertEqual(result.returncode, 65, result.stdout + result.stderr)
            self.assertIn("OUT_OF_SCOPE:source", result.stdout)

            result = run_cli_in(
                "/tmp",
                "build",
                "--backend",
                str(fail),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
            )
            self.assertEqual(result.returncode, 78, result.stdout + result.stderr)
            self.assertIn("WORKTREE_ISOLATION", result.stdout)

    def test_route_local_record_is_idempotent_sha_bound_and_finalizable(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-test-", dir=str(CLI.parent)) as temp:
            root = Path(temp)
            source = root / "source.asc"
            artifact = root / "artifact.o"
            raw = root / "raw.tsv"
            manifest = root / "record.json"
            counter = root / "record-backend-count"
            source.write_text("route-local-source\n")
            artifact.write_text("route-local-artifact\n")
            write_raw(raw, pairs=2)
            counter.write_text("0")

            common = [
                "record",
                "--source", str(source),
                "--artifact", str(artifact),
                "--raw", str(raw),
                "--manifest", str(manifest),
                "--evidence", str(source),
                "--backend", "",
            ]
            first = run_cli(*common)
            self.assertEqual(first.returncode, 78, first.stdout + first.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_WRITTEN", first.stdout)
            self.assertIn("INTEGRATION_STATUS=INTEGRATION_INCOMPLETE", first.stdout)
            saved_bytes = manifest.read_bytes()
            saved = json.loads(saved_bytes)
            self.assertEqual(saved["source"]["sha256"], __import__("hashlib").sha256(source.read_bytes()).hexdigest())
            self.assertEqual(saved["artifact"]["sha256"], __import__("hashlib").sha256(artifact.read_bytes()).hexdigest())
            self.assertEqual(saved["raw"]["sha256"], __import__("hashlib").sha256(raw.read_bytes()).hexdigest())
            self.assertEqual(saved["local_result"]["status"], "INSUFFICIENT_DATA")
            self.assertEqual(saved["local_result"]["pair_count"], 2)
            self.assertEqual(saved["local_result"]["full_local_score"], "NOT_COMPUTABLE")
            self.assertFalse(saved["local_result"]["promotion_eligible"])

            repeated = run_cli(*common)
            self.assertEqual(repeated.returncode, 78, repeated.stdout + repeated.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_REUSED", repeated.stdout)
            self.assertIn("IDEMPOTENT_NOOP=YES", repeated.stdout)
            self.assertEqual(manifest.read_bytes(), saved_bytes)

            bad_sha = run_cli(
                *common,
                "--expected-source-sha",
                "0" * 64,
            )
            self.assertEqual(bad_sha.returncode, 65, bad_sha.stdout + bad_sha.stderr)
            self.assertIn("SOURCE_SHA_MISMATCH", bad_sha.stdout)
            self.assertEqual(manifest.read_bytes(), saved_bytes)

            record_backend = root / "record-backend.py"
            write_executable(
                record_backend,
                """
                #!/usr/bin/env python3
                import os
                from pathlib import Path
                path = Path(os.environ["TEST_RECORD_BACKEND_COUNT"])
                path.write_text(str(int(path.read_text()) + 1))
                """,
            )
            env = os.environ.copy()
            env["TEST_RECORD_BACKEND_COUNT"] = str(counter)
            finalize = subprocess.run(
                [sys.executable, "-B", str(CLI), "record", *scope_args(), *common[1:-2], "--backend", str(record_backend)],
                cwd=str(ROOT), env=env, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
            )
            self.assertEqual(finalize.returncode, 0, finalize.stdout + finalize.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_FINALIZED", finalize.stdout)
            self.assertEqual(counter.read_text(), "1")
            finalized_bytes = manifest.read_bytes()
            finalized = json.loads(finalized_bytes)
            self.assertEqual(finalized["record_status"], "PASS")
            self.assertEqual(finalized["backend_rc"], 0)

            replay = subprocess.run(
                [sys.executable, "-B", str(CLI), "record", *scope_args(), *common[1:-2], "--backend", str(record_backend)],
                cwd=str(ROOT), env=env, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
            )
            self.assertEqual(replay.returncode, 0, replay.stdout + replay.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_REUSED", replay.stdout)
            self.assertIn("IDEMPOTENT_NOOP=YES", replay.stdout)
            self.assertEqual(counter.read_text(), "1")
            self.assertEqual(manifest.read_bytes(), finalized_bytes)

            raw.write_text(raw.read_text().replace("9.0", "8.0"))
            changed_identity = run_cli(
                *common[:-2],
                "--backend",
                str(record_backend),
            )
            self.assertEqual(changed_identity.returncode, 78, changed_identity.stdout + changed_identity.stderr)
            self.assertIn("RECORD_MANIFEST_IDENTITY_CONFLICT", changed_identity.stdout)
            self.assertEqual(counter.read_text(), "1")
            self.assertEqual(manifest.read_bytes(), finalized_bytes)

    def test_non_executable_shell_backend_uses_bash_and_propagates_status(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-test-", dir=str(CLI.parent)) as temp:
            root = Path(temp)
            source = root / "source.asc"
            artifact = root / "artifact.so"
            backend = root / "historical-build.sh"
            source.write_text("public-parent-candidate-input\n")
            backend.write_text(
                "#!/bin/bash\n"
                "if [[ \"$W5CTL_STAGE\" == build ]]; then\n"
                "  printf 'artifact-from-shell-backend\\n' > \"$W5CTL_ARTIFACT\"\n"
                "  echo HISTORICAL_SHELL_BACKEND=BUILD\n"
                "elif [[ \"$W5CTL_STAGE\" == correctness ]]; then\n"
                "  echo HISTORICAL_SHELL_BACKEND=CORRECTNESS\n"
                "  exit 17\n"
                "fi\n"
            )
            backend.chmod(0o644)

            build = run_cli(
                "build",
                "--backend",
                str(backend),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
            )
            self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
            self.assertIn("BACKEND_COMMAND=/bin/bash", build.stdout)
            self.assertIn("HISTORICAL_SHELL_BACKEND=BUILD", build.stdout)
            self.assertIn("BACKEND_RC=0", build.stdout)
            self.assertIn("IDENTITY_STATUS=PASS", build.stdout)
            self.assertEqual(artifact.read_text(), "artifact-from-shell-backend\n")

            correctness = run_cli(
                "correctness",
                "--backend",
                str(backend),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
            )
            self.assertEqual(correctness.returncode, 17, correctness.stdout + correctness.stderr)
            self.assertIn("BACKEND_COMMAND=/bin/bash", correctness.stdout)
            self.assertIn("HISTORICAL_SHELL_BACKEND=CORRECTNESS", correctness.stdout)
            self.assertIn("BACKEND_RC=17", correctness.stdout)
            self.assertIn("STATUS=FAILED", correctness.stdout)

    def test_local_statistics_accept_insufficient_and_contamination(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-test-", dir=str(CLI.parent)) as temp:
            root = Path(temp)
            clean = root / "clean.tsv"
            write_raw(clean)
            result = run_cli("local", "--stats-only", "--raw", str(clean))
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("MEDIAN_PARENT_US=10.000000", result.stdout)
            self.assertIn("MEDIAN_CANDIDATE_US=9.000000", result.stdout)
            self.assertIn("MEDIAN_RATIO_DELTA=-0.100000", result.stdout)
            self.assertIn("PAIRED_MEDIAN_DELTA=-1.000000", result.stdout)
            self.assertIn("PAIRED_MEDIAN_PERCENT_DELTA=-0.100000", result.stdout)
            self.assertIn("RAW_SAMPLE_COUNT=8", result.stdout)
            self.assertIn("MEASUREMENT_QUALITY=ACCEPTED", result.stdout)
            self.assertIn("NOISE_STATUS=STABLE", result.stdout)

            insufficient = root / "insufficient.tsv"
            write_raw(insufficient, pairs=2)
            result = run_cli("local", "--stats-only", "--raw", str(insufficient))
            self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
            self.assertIn("MEASUREMENT_QUALITY=INSUFFICIENT_DATA", result.stdout)
            self.assertIn("NOISE_STATUS=INSUFFICIENT_DATA", result.stdout)
            self.assertIn("MEDIAN_PARENT_US=10.000000", result.stdout)

            contaminated = root / "contaminated.tsv"
            write_raw(contaminated, quality="LOAD_CONTAMINATED")
            result = run_cli("local", "--stats-only", "--raw", str(contaminated))
            self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
            self.assertIn("MEASUREMENT_QUALITY=REJECTED_CONTAMINATION", result.stdout)
            self.assertIn("NOISE_STATUS=CONTAMINATED", result.stdout)
            self.assertIn("MEDIAN_PARENT_US=NONE", result.stdout)

            missing_quality = root / "missing-quality.tsv"
            missing_quality.write_text(
                "pair\tvariant\tlatency_us\n"
                "1\tparent\t10\n"
                "1\tcandidate\t9\n"
            )
            result = run_cli("local", "--stats-only", "--raw", str(missing_quality))
            self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
            self.assertIn("MEASUREMENT_QUALITY=RAW_PARSE_ERROR", result.stdout)
            self.assertIn("RAW_QUALITY_MISSING", result.stdout)

            unpaired = root / "unpaired.tsv"
            write_raw(unpaired, pairs=3)
            with unpaired.open("a") as stream:
                stream.write("4\tparent\t10\tCLEAN\tfalse\n")
            result = run_cli("local", "--stats-only", "--raw", str(unpaired))
            self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
            self.assertIn("MEASUREMENT_QUALITY=UNPAIRED_DATA", result.stdout)
            self.assertIn("NOISE_STATUS=INCOMPLETE_PAIRS", result.stdout)

            nonpositive_parent = root / "nonpositive-parent.tsv"
            nonpositive_parent.write_text(
                "pair\tvariant\tlatency_us\tquality\tcontaminated\n"
                "1\tparent\t0\tCLEAN\tfalse\n"
                "1\tcandidate\t9\tCLEAN\tfalse\n"
            )
            result = run_cli("local", "--stats-only", "--raw", str(nonpositive_parent))
            self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
            self.assertIn("RAW_PARENT_LATENCY_NONPOSITIVE", result.stdout)

            crossrow = root / "crossrow-runner.tsv"
            crossrow_lines = [
                "# LOCAL_PROXY_ONLY parent=public-parent candidate=public-candidate warmups=45 paired_samples=21 device_load=EXTERNAL_NOT_CAPTURED",
                "sample\torder\tparent_device_us\tcandidate_device_us\tdelta_device_us\tparent_wall_us\tcandidate_wall_us\tdelta_wall_us",
            ]
            for sample in range(1, 22):
                order = "PC" if sample % 2 else "CP"
                crossrow_lines.append(f"{sample}\t{order}\t20.0\t18.0\t-2.0\t30.0\t28.0\t-2.0")
            crossrow.write_text("\n".join(crossrow_lines) + "\n")
            crossrow_before = crossrow.read_bytes()
            result = run_cli("local", "--stats-only", "--raw", str(crossrow))
            self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
            self.assertIn("MEDIAN_PARENT_US=20.000000", result.stdout)
            self.assertIn("MEDIAN_CANDIDATE_US=18.000000", result.stdout)
            self.assertIn("MEDIAN_RATIO_DELTA=-0.100000", result.stdout)
            self.assertIn("PAIRED_MEDIAN_DELTA=-2.000000", result.stdout)
            self.assertIn("PAIRED_MEDIAN_PERCENT_DELTA=-0.100000", result.stdout)
            self.assertIn("RAW_SAMPLE_COUNT=42", result.stdout)
            self.assertIn("RAW_PAIR_COUNT=21", result.stdout)
            self.assertIn("MEASUREMENT_QUALITY=UNCLASSIFIED", result.stdout)
            self.assertIn("QUALITY_CLASSIFICATION=UNCLASSIFIED", result.stdout)
            self.assertIn("NOISE_STATUS=STABLE_NUMERICALLY_ONLY", result.stdout)
            self.assertIn("PROMOTION_ELIGIBLE=NO", result.stdout)
            self.assertNotIn("STATUS=PASS", result.stdout)
            self.assertNotIn("MEASUREMENT_QUALITY=ACCEPTED", result.stdout)
            self.assertEqual(crossrow.read_bytes(), crossrow_before)

    def test_dedup_same_and_different_patches(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-test-", dir=str(CLI.parent)) as temp:
            root = Path(temp)
            common = root / "common.patch"
            common.write_text("#include <abi_scaffold.h>\n")
            same_text = """\
                diff --git a/kernel.asc b/kernel.asc
                --- a/kernel.asc
                +++ b/kernel.asc
                @@
                +#include <abi_scaffold.h>
                +for (int tile = 0; tile < n; ++tile) {
                +  acc = ReduceSum(tile);
                +}
            """
            parent = root / "parent.patch"
            candidate = root / "candidate.patch"
            parent.write_text(textwrap.dedent(same_text))
            candidate.write_text(textwrap.dedent(same_text))
            result = run_cli(
                "dedup",
                "--parent-patch",
                str(parent),
                "--candidate-patch",
                str(candidate),
                "--common",
                str(common),
            )
            self.assertEqual(result.returncode, 78, result.stdout + result.stderr)
            self.assertIn("PATCH_SIMILARITY=1.000000", result.stdout)
            self.assertIn("PATCH_SIMILARITY_GATE=0.600000", result.stdout)
            self.assertIn("PATCH_SIMILARITY_GATE_STATUS=BLOCKED", result.stdout)
            self.assertIn("DEDUP_STATUS=BLOCKED_PATCH_SIMILARITY", result.stdout)
            self.assertIn("COMMON_EXCLUDED_LINE_COUNT=1", result.stdout)

            different = root / "different.patch"
            different.write_text(
                textwrap.dedent(
                    """\
                    diff --git a/kernel.asc b/kernel.asc
                    --- a/kernel.asc
                    +++ b/kernel.asc
                    @@
                    +#include <abi_scaffold.h>
                    +for (int row = 0; row < rows; ++row) {
                    +  DataCopy(dst, src, row);
                    +}
                    """
                )
            )
            result = run_cli(
                "dedup",
                "--parent-patch",
                str(parent),
                "--candidate-patch",
                str(different),
                "--common",
                str(common),
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("PATCH_SIMILARITY_GATE_STATUS=PASS", result.stdout)
            self.assertIn("MECHANISM_ORTHOGONALITY=UNPROVEN", result.stdout)
            self.assertIn("MECHANISM_REVIEW=ZERO_TOKEN_OVERLAP_ONLY", result.stdout)
            self.assertIn("DEDUP_STATUS=DISTINCT", result.stdout)

            review_parent = root / "review-parent.patch"
            review_candidate = root / "review-candidate.patch"
            review_parent.write_text(
                "+register pressure live range scratch vector spill reload alloc parent_only\n"
            )
            review_candidate.write_text(
                "+register pressure live range scratch vector spill reload alloc candidate_a candidate_b candidate_c\n"
            )
            result = run_cli(
                "dedup",
                "--parent-patch",
                str(review_parent),
                "--candidate-patch",
                str(review_candidate),
            )
            self.assertEqual(result.returncode, 78, result.stdout + result.stderr)
            self.assertIn("PATCH_SIMILARITY=0.000000", result.stdout)
            self.assertIn("MECHANISM_OVERLAP=0.692308", result.stdout)
            self.assertIn("MECHANISM_OVERLAP_GATE_STATUS=PASS", result.stdout)
            self.assertIn("MECHANISM_ORTHOGONALITY=UNPROVEN", result.stdout)
            self.assertIn("MECHANISM_REVIEW=REQUIRED_OWNER_REVIEW", result.stdout)
            self.assertIn("DEDUP_STATUS=MANUAL_REVIEW_MECHANISM_OVERLAP", result.stdout)
            self.assertNotIn("DEDUP_STATUS=DISTINCT", result.stdout)

            gate_parent = root / "gate-parent.patch"
            gate_parent.write_text(
                "\n".join(
                    ["+for int"] * 6
                    + [f"+parent_unique_{index}" for index in range(1, 4)]
                )
                + "\n"
            )
            gate_blocked = root / "gate-blocked.patch"
            gate_blocked.write_text(
                "\n".join(
                    ["+for int"] * 6
                    + [f"+blocked_{index}" for index in range(10, 14)]
                )
                + "\n"
            )
            result = run_cli(
                "dedup",
                "--parent-patch",
                str(gate_parent),
                "--candidate-patch",
                str(gate_blocked),
            )
            self.assertEqual(result.returncode, 78, result.stdout + result.stderr)
            self.assertIn("PATCH_SIMILARITY=0.631579", result.stdout)
            self.assertIn("PATCH_SIMILARITY_GATE_STATUS=BLOCKED", result.stdout)
            self.assertIn("DEDUP_STATUS=BLOCKED_PATCH_SIMILARITY", result.stdout)

            gate_allowed = root / "gate-allowed.patch"
            gate_allowed.write_text(
                "\n".join(
                    ["+for int"] * 6
                    + [f"+allowed_{index}" for index in range(10, 15)]
                )
                + "\n"
            )
            result = run_cli(
                "dedup",
                "--parent-patch",
                str(gate_parent),
                "--candidate-patch",
                str(gate_allowed),
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("PATCH_SIMILARITY=0.600000", result.stdout)
            self.assertIn("PATCH_SIMILARITY_GATE_STATUS=PASS", result.stdout)
            self.assertIn("MECHANISM_OVERLAP=0.000000", result.stdout)
            self.assertIn("MECHANISM_REVIEW=ZERO_TOKEN_OVERLAP_ONLY", result.stdout)
            self.assertIn("DEDUP_STATUS=DISTINCT", result.stdout)

            overlap_parent = root / "overlap-parent.patch"
            overlap_candidate = root / "overlap-candidate.patch"
            overlap_parent.write_text(
                "\n".join(
                    "+register pressure live range scratch vector parent_only"
                    for _ in range(4)
                )
                + "\n"
            )
            overlap_candidate.write_text(
                "\n".join(
                    "+register pressure live range scratch vector candidate_only"
                    for _ in range(4)
                )
                + "\n"
            )
            result = run_cli(
                "dedup",
                "--parent-patch",
                str(overlap_parent),
                "--candidate-patch",
                str(overlap_candidate),
            )
            self.assertEqual(result.returncode, 78, result.stdout + result.stderr)
            self.assertIn("PATCH_SIMILARITY=0.000000", result.stdout)
            self.assertIn("MECHANISM_OVERLAP=0.750000", result.stdout)
            self.assertIn("MECHANISM_OVERLAP_GATE_STATUS=BLOCKED", result.stdout)
            self.assertIn("DEDUP_STATUS=BLOCKED_MECHANISM_OVERLAP", result.stdout)

    def test_cycle_resume_and_repeat_are_checkpoint_idempotent(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-test-", dir=str(CLI.parent)) as temp:
            root = Path(temp)
            source = root / "source.asc"
            artifact = root / "artifact.o"
            raw = root / "raw.tsv"
            checkpoint = root / "checkpoint.json"
            counter = root / "counter"
            source.write_text("champion-source\n")
            counter.write_text("0")
            backend = root / "backend.py"
            write_executable(
                backend,
                """
                #!/usr/bin/env python3
                import os
                from pathlib import Path
                counter = Path(os.environ["TEST_COUNTER"])
                value = int(counter.read_text()) + 1
                counter.write_text(str(value))
                stage = os.environ["W5CTL_STAGE"]
                if stage == "build":
                    Path(os.environ["W5CTL_ARTIFACT"]).write_text("artifact")
                if stage == "local":
                    Path(os.environ["W5CTL_RAW_PATH"]).write_text(
                        "pair\\tvariant\\tlatency_us\\tquality\\tcontaminated\\n"
                        "1\\tparent\\t10\\tCLEAN\\tfalse\\n"
                        "1\\tcandidate\\t9\\tCLEAN\\tfalse\\n"
                        "2\\tparent\\t10\\tCLEAN\\tfalse\\n"
                        "2\\tcandidate\\t9\\tCLEAN\\tfalse\\n"
                        "3\\tparent\\t10\\tCLEAN\\tfalse\\n"
                        "3\\tcandidate\\t9\\tCLEAN\\tfalse\\n"
                    )
                """,
            )
            failing = root / "failing.py"
            write_executable(
                failing,
                """
                #!/usr/bin/env python3
                import sys
                sys.exit(13)
                """,
            )
            env = os.environ.copy()
            env["TEST_COUNTER"] = str(counter)
            first = subprocess.run(
                [
                    sys.executable,
                    "-B",
                    str(CLI),
                    "cycle",
                    *scope_args(),
                    "--checkpoint",
                    str(checkpoint),
                    "--source",
                    str(source),
                    "--artifact",
                    str(artifact),
                    "--raw",
                    str(raw),
                    "--build-backend",
                    str(failing),
                    "--correctness-backend",
                    str(backend),
                    "--local-backend",
                    str(backend),
                ],
                cwd=str(ROOT),
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(first.returncode, 13, first.stdout + first.stderr)
            self.assertEqual(counter.read_text(), "0")
            self.assertTrue(checkpoint.is_file())

            resumed = subprocess.run(
                [
                    sys.executable,
                    "-B",
                    str(CLI),
                    "resume",
                    *scope_args(),
                    "--checkpoint",
                    str(checkpoint),
                    "--source",
                    str(source),
                    "--artifact",
                    str(artifact),
                    "--raw",
                    str(raw),
                    "--from-stage",
                    "build",
                    "--build-backend",
                    str(backend),
                    "--correctness-backend",
                    str(backend),
                    "--local-backend",
                    str(backend),
                ],
                cwd=str(ROOT),
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(resumed.returncode, 0, resumed.stdout + resumed.stderr)
            self.assertIn("CYCLE_STATUS=PASS", resumed.stdout)
            self.assertEqual(counter.read_text(), "3")

            repeated = subprocess.run(
                [
                    sys.executable,
                    "-B",
                    str(CLI),
                    "cycle",
                    *scope_args(),
                    "--checkpoint",
                    str(checkpoint),
                    "--source",
                    str(source),
                    "--artifact",
                    str(artifact),
                    "--raw",
                    str(raw),
                    "--build-backend",
                    str(backend),
                    "--correctness-backend",
                    str(backend),
                    "--local-backend",
                    str(backend),
                ],
                cwd=str(ROOT),
                env=env,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(repeated.returncode, 0, repeated.stdout + repeated.stderr)
            self.assertIn("IDEMPOTENT_NOOP=YES", repeated.stdout)
            self.assertEqual(counter.read_text(), "3")
            report = run_cli("report", "--checkpoint", str(checkpoint))
            self.assertEqual(report.returncode, 0, report.stdout + report.stderr)
            self.assertIn("BUILD_STATUS=PASS", report.stdout)
            self.assertIn("ONLINE=NOT_SUBMITTED", report.stdout)
            payload = json.loads(checkpoint.read_text())
            self.assertFalse(payload["revision_created"])
            self.assertEqual(payload["online"], "NOT_SUBMITTED")

    def test_manifest_checkpoint_report_unclassified_and_resume_idempotence(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-test-", dir=str(CLI.parent)) as temp:
            root = Path(temp)
            source = root / "source.asc"
            artifact = root / "artifact.so"
            raw = root / "crossrow.tsv"
            checkpoint = root / "checkpoint.json"
            manifest = root / "record.json"
            counter = root / "counter"
            source.write_text("public-source\n")
            counter.write_text("0")
            backend = root / "backend.py"
            write_executable(
                backend,
                """
                #!/usr/bin/env python3
                import os
                from pathlib import Path
                counter = Path(os.environ["TEST_COUNTER"])
                counter.write_text(str(int(counter.read_text()) + 1))
                stage = os.environ["W5CTL_STAGE"]
                if stage == "build":
                    Path(os.environ["W5CTL_ARTIFACT"]).write_text("artifact")
                elif stage == "local":
                    raw = Path(os.environ["W5CTL_RAW_PATH"])
                    rows = [
                        "# LOCAL_PROXY_ONLY parent=p candidate=c warmups=45 paired_samples=3",
                        "sample\torder\tparent_device_us\tcandidate_device_us\tdelta_device_us",
                    ]
                    for sample in range(1, 4):
                        rows.append(f"{sample}\tPC\t10\t9\t-1")
                    raw.write_text("\\n".join(rows) + "\\n")
                """,
            )
            env = os.environ.copy()
            env["TEST_COUNTER"] = str(counter)
            common = [
                *scope_args(),
                "--checkpoint",
                str(checkpoint),
                "--source",
                str(source),
                "--artifact",
                str(artifact),
                "--raw",
                str(raw),
                "--stages",
                "build,correctness,local,record",
                "--build-backend",
                str(backend),
                "--correctness-backend",
                str(backend),
                "--local-backend",
                str(backend),
                "--record-manifest",
                str(manifest),
                "--record-backend",
                "",
                "--record-evidence",
                str(source),
            ]
            first = subprocess.run(
                [sys.executable, "-B", str(CLI), "cycle", *common],
                cwd=str(ROOT), env=env, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
            )
            self.assertEqual(first.returncode, 78, first.stdout + first.stderr)
            self.assertIn("MEASUREMENT_QUALITY=UNCLASSIFIED", first.stdout)
            self.assertIn("PROMOTION_ELIGIBLE=NO", first.stdout)
            self.assertIn("RECORD_STATUS=LOCAL_MANIFEST_WRITTEN", first.stdout)
            self.assertIn("INTEGRATION_STATUS=INTEGRATION_INCOMPLETE", first.stdout)
            self.assertTrue(checkpoint.is_file())
            self.assertTrue(manifest.is_file())
            payload = json.loads(checkpoint.read_text())
            record = json.loads(manifest.read_text())
            self.assertEqual(payload["stages"]["local"]["status"], "UNCLASSIFIED")
            self.assertEqual(
                payload["stages"]["local"]["result"]["shape_local_verdict"],
                "SHAPE_UNCLASSIFIED_NO_PROMOTION",
            )
            self.assertFalse(payload["stages"]["local"]["result"]["promotion_eligible"])
            self.assertEqual(record["source"]["sha256"], __import__("hashlib").sha256(source.read_bytes()).hexdigest())
            self.assertEqual(record["artifact"]["sha256"], __import__("hashlib").sha256(artifact.read_bytes()).hexdigest())
            self.assertEqual(record["raw"]["sha256"], __import__("hashlib").sha256(raw.read_bytes()).hexdigest())
            self.assertEqual(record["evidence"][0]["path"], str(source.resolve()))
            self.assertEqual(record["evidence"][0]["sha256"], __import__("hashlib").sha256(source.read_bytes()).hexdigest())
            self.assertEqual(record["local_result"]["shape_local_verdict"], "SHAPE_UNCLASSIFIED_NO_PROMOTION")
            self.assertEqual(record["integration_status"], "INTEGRATION_INCOMPLETE")
            manifest_bytes_after_record = manifest.read_bytes()

            report = run_cli("report", "--checkpoint", str(checkpoint))
            self.assertEqual(report.returncode, 0, report.stdout + report.stderr)
            self.assertIn("LOCAL_STATUS=UNCLASSIFIED", report.stdout)
            self.assertIn("LOCAL_MEDIAN_PARENT_US=10.000000", report.stdout)
            self.assertIn("LOCAL_MEDIAN_CANDIDATE_US=9.000000", report.stdout)
            self.assertIn("LOCAL_RAW_PAIR_COUNT=3", report.stdout)
            self.assertIn("LOCAL_NOISE_STATUS=STABLE_NUMERICALLY_ONLY", report.stdout)
            self.assertIn("SHAPE_LOCAL_VERDICT=SHAPE_UNCLASSIFIED_NO_PROMOTION", report.stdout)
            self.assertIn("LOCAL_PROMOTION_ELIGIBLE=NO", report.stdout)
            self.assertIn("FULL_LOCAL_SCORE=NOT_COMPUTABLE", report.stdout)
            self.assertIn("RECORD_INTEGRATION_STATUS=INTEGRATION_INCOMPLETE", report.stdout)
            self.assertIn("REPORT_STATUS=INCOMPLETE", report.stdout)

            manifest_report = run_cli("report", "--manifest", str(manifest))
            self.assertEqual(manifest_report.returncode, 0, manifest_report.stdout + manifest_report.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=IDENTITY_VERIFIED", manifest_report.stdout)
            self.assertIn("IDENTITY_STATUS=PASS", manifest_report.stdout)
            self.assertIn("SOURCE_SHA256=" + record["source"]["sha256"], manifest_report.stdout)
            self.assertIn("ARTIFACT_SHA256=" + record["artifact"]["sha256"], manifest_report.stdout)
            self.assertIn("RAW_SHA256=" + record["raw"]["sha256"], manifest_report.stdout)
            self.assertIn("SHAPE_LOCAL_VERDICT=SHAPE_UNCLASSIFIED_NO_PROMOTION", manifest_report.stdout)
            self.assertIn("FULL_LOCAL_SCORE=NOT_COMPUTABLE", manifest_report.stdout)
            self.assertIn("PROMOTION_ELIGIBLE=NO", manifest_report.stdout)
            self.assertIn("RECORD_INTEGRATION_STATUS=INTEGRATION_INCOMPLETE", manifest_report.stdout)
            self.assertIn("REPORT_STATUS=INCOMPLETE", manifest_report.stdout)

            outside_report = run_cli(
                "report", "--manifest", str(ROOT / "outside-route-manifest.json")
            )
            self.assertEqual(outside_report.returncode, 78, outside_report.stdout + outside_report.stderr)
            self.assertIn("OUT_OF_SCOPE:manifest", outside_report.stdout)

            tampered_manifest = root / "tampered-record.json"
            tampered_record = dict(record)
            tampered_record["artifact"] = dict(record["artifact"])
            tampered_record["artifact"]["sha256"] = "0" * 64
            tampered_manifest.write_text(json.dumps(tampered_record))
            tampered_report = run_cli("report", "--manifest", str(tampered_manifest))
            self.assertEqual(tampered_report.returncode, 65, tampered_report.stdout + tampered_report.stderr)
            self.assertIn("ARTIFACT_SHA_MISMATCH", tampered_report.stdout)

            before = counter.read_text()
            interrupted_checkpoint = json.loads(checkpoint.read_text())
            interrupted_checkpoint["stages"].pop("record")
            checkpoint.write_text(json.dumps(interrupted_checkpoint))
            resumed = subprocess.run(
                [sys.executable, "-B", str(CLI), "resume", *common, "--from-stage", "record"],
                cwd=str(ROOT), env=env, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
            )
            self.assertEqual(resumed.returncode, 78, resumed.stdout + resumed.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_REUSED", resumed.stdout)
            self.assertEqual(counter.read_text(), before)
            self.assertEqual(manifest.read_bytes(), manifest_bytes_after_record)

            resumed_again = subprocess.run(
                [sys.executable, "-B", str(CLI), "resume", *common],
                cwd=str(ROOT), env=env, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
            )
            self.assertEqual(resumed_again.returncode, 78, resumed_again.stdout + resumed_again.stderr)
            self.assertIn("IDEMPOTENT_NOOP=YES", resumed_again.stdout)
            self.assertIn("REVISION_CREATED=NO", resumed_again.stdout)
            self.assertIn("RUNNER_CREATED=NO", resumed_again.stdout)
            self.assertEqual(counter.read_text(), before)
            self.assertEqual(manifest.read_bytes(), manifest_bytes_after_record)


if __name__ == "__main__":
    unittest.main(verbosity=2)
