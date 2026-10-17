#!/usr/bin/env python3
"""Self-test for the route-owned w5ctl orchestration contract."""

import json
import csv
import hashlib
import io
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
CANONICAL_COMMIT = "6d3ad0b30b21d2462b0fd8716c884f0ff6bdaabb"
PUBLIC_R1_CROSSROW_COMMIT = "d2412ab5cb2d091679947c65d90d45912541b1a5"
ANCHOR_ROOT = "本地实验/MAIN2-CANONICAL-V1/versions/R31B__V011/"
CANONICAL_CASES = ("C12", "C13", "C14", "C16")


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


def run_cli_with_route_root(route_root, *args):
    return run_cli_with_scope(ROOT, scope_args(route_root=route_root), *args)


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


def git_object_at(commit, object_path):
    completed = subprocess.run(
        ["git", "show", f"{commit}:{object_path}"],
        cwd=str(ROOT),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if completed.returncode != 0:
        raise AssertionError(completed.stderr.decode("utf-8", "replace"))
    return completed.stdout


def git_object(object_path):
    return git_object_at(CANONICAL_COMMIT, object_path)


def create_public_canonical_fixture(route_root, as_git_objects=True):
    public_version = "本地实验/MAIN2-CANONICAL-V1/versions/VECTOR-MATH-X__V003"
    public_build = "本地实验/MAIN2-CANONICAL-V1/builds/VECTOR-MATH-X__V003"
    source = route_root / "public-vector-math-submission.asc"
    parent = route_root / "public-vector-math-parent.asc"
    receipt = route_root / "public-vector-math-build.json"
    correctness = route_root / "public-vector-math-correctness.json"
    measurements = route_root / "public-vector-math-measurements.tsv"
    source.write_bytes(git_object(public_build + "/submission.asc"))
    parent.write_bytes(git_object(public_build + "/parent.asc"))
    receipt.write_bytes(git_object(public_version + "/build.json"))
    correctness.write_bytes(git_object(public_version + "/correctness.json"))
    measurement_text = git_object(public_version + "/measurements.tsv").decode()
    if as_git_objects:
        measurement_text = measurement_text.replace(
            "/home/data4t2/lelinfeng/cann-w2-main2-control/",
            f"gitobj://{CANONICAL_COMMIT}/",
        )
    measurements.write_text(measurement_text)
    return {
        "source": source,
        "parent": parent,
        "receipt": receipt,
        "correctness": correctness,
        "measurements": measurements,
        "source_sha": hashlib.sha256(source.read_bytes()).hexdigest(),
        "parent_sha": hashlib.sha256(parent.read_bytes()).hexdigest(),
        "artifact_sha": json.loads(receipt.read_text())["EXE_SHA"],
    }


def create_canonical_fixture(route_root, revision, candidate_scale):
    source = route_root / f"{revision}.asc"
    artifact = route_root / f"{revision}.o"
    measurements = route_root / f"{revision}-measurements.tsv"
    manifest = route_root / f"{revision}-record.json"
    source.write_text(f"route kernel source {revision}\n")
    artifact.write_bytes(f"route kernel artifact {revision}\n".encode())
    source_sha = hashlib.sha256(source.read_bytes()).hexdigest()
    artifact_sha = hashlib.sha256(artifact.read_bytes()).hexdigest()
    index = list(csv.DictReader(
        io.StringIO(git_object(ANCHOR_ROOT + "measurements.tsv").decode()),
        delimiter="\t",
    ))
    anchor = {
        (row["CASE_ID"], int(row["RUN"])): row
        for row in index
        if row.get("CASE_ID") in CANONICAL_CASES and row.get("SIDE") == "anchor"
    }
    output_rows = []
    for case_id in CANONICAL_CASES:
        stable_parent_us = float(anchor[(case_id, 1)]["MEDIAN_US"])
        for run in (1, 2, 3):
            anchor_row = anchor[(case_id, run)]
            suffix = anchor_row["RAW_PATH"].split(ANCHOR_ROOT, 1)[-1]
            if suffix == anchor_row["RAW_PATH"]:
                raise AssertionError("published anchor raw path does not map to its public commit")
            raw_object = ANCHOR_ROOT + suffix
            stats_object = anchor_row["STATS_PATH"].split(ANCHOR_ROOT, 1)[-1]
            if stats_object == anchor_row["STATS_PATH"]:
                raise AssertionError("published anchor stats path does not map to its public commit")
            raw_bytes = git_object(raw_object)
            stats_bytes = git_object(ANCHOR_ROOT + stats_object)
            stats_path = route_root / f"{revision}-{case_id}-run{run}-stats.txt"
            stats_path.write_bytes(stats_bytes)
            for side in (("parent", "candidate") if run in (1, 3) else ("candidate", "parent")):
                side_scale = 1.0 if side == "parent" else candidate_scale
                raw_path = route_root / f"{revision}-{case_id}-run{run}-{side}-raw.tsv"
                raw_rows = list(csv.DictReader(io.StringIO(raw_bytes.decode()), delimiter="\t"))
                for sample in raw_rows:
                    sample["device_us"] = f"{stable_parent_us * side_scale:.9g}"
                with raw_path.open("w", newline="") as stream:
                    writer = csv.DictWriter(stream, fieldnames=raw_rows[0].keys(), delimiter="\t", lineterminator="\n")
                    writer.writeheader()
                    writer.writerows(raw_rows)
                row = dict(anchor_row)
                row.update({
                    "ROUTE": ROUTE,
                    "REVISION": revision,
                    "SOURCE_SHA": source_sha if side == "candidate" else anchor_row["SOURCE_SHA"],
                    "SIDE": side,
                    "RUN": str(run),
                    "ATTEMPT": str(run),
                    "EXE_SHA": artifact_sha if side == "candidate" else anchor_row["EXE_SHA"],
                    "RAW_PATH": str(raw_path.resolve()),
                    "STATS_PATH": str(stats_path.resolve()),
                })
                output_rows.append(row)
    with measurements.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=index[0].keys(), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(output_rows)
    return {
        "source": source,
        "artifact": artifact,
        "measurements": measurements,
        "manifest": manifest,
        "source_sha": source_sha,
        "artifact_sha": artifact_sha,
    }


def run_canonical_record(route_root, fixture, revision, extra_args=()):
    return run_cli_with_route_root(
        route_root,
        "record",
        "--source", str(fixture["source"]),
        "--artifact", str(fixture["artifact"]),
        "--raw", str(fixture["measurements"]),
        "--canonical-measurements", str(fixture["measurements"]),
        "--canonical-purpose", "route-kernel",
        "--canonical-revision", revision,
        "--manifest", str(fixture["manifest"]),
        "--evidence", str(fixture["source"]),
        "--backend", "",
        *extra_args,
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
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_FINALIZED", result.stdout)
            self.assertIn("RECORD_STATUS=PASS", result.stdout)
            self.assertIn("BACKEND_STATUS=FAILED", result.stdout)
            self.assertIn("BACKEND_RC=7", result.stdout)
            self.assertIn("STATUS=PASS", result.stdout)
            failed_manifest = root / "record-failed.json"
            failed_payload = json.loads(failed_manifest.read_text())
            self.assertEqual(failed_payload["record_status"], "PASS")
            self.assertEqual(failed_payload["backend_status"], "FAILED")
            self.assertEqual(failed_payload["backend_rc"], 7)
            report = run_cli("report", "--manifest", str(failed_manifest))
            self.assertEqual(report.returncode, 0, report.stdout + report.stderr)
            self.assertIn("REPORT_STATUS=PASS", report.stdout)
            self.assertIn("BACKEND_STATUS=FAILED", report.stdout)

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
            self.assertEqual(first.returncode, 0, first.stdout + first.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_WRITTEN", first.stdout)
            self.assertIn("RECORD_STATUS=PASS", first.stdout)
            self.assertIn("BACKEND_STATUS=NOT_REQUESTED", first.stdout)
            self.assertIn("BACKEND_RC=NONE", first.stdout)
            saved_bytes = manifest.read_bytes()
            saved = json.loads(saved_bytes)
            self.assertEqual(saved["record_status"], "PASS")
            self.assertEqual(saved["backend_status"], "NOT_REQUESTED")
            self.assertEqual(saved["source"]["sha256"], __import__("hashlib").sha256(source.read_bytes()).hexdigest())
            self.assertEqual(saved["artifact"]["sha256"], __import__("hashlib").sha256(artifact.read_bytes()).hexdigest())
            self.assertEqual(saved["raw"]["sha256"], __import__("hashlib").sha256(raw.read_bytes()).hexdigest())
            self.assertEqual(saved["local_result"]["status"], "INSUFFICIENT_DATA")
            self.assertEqual(saved["local_result"]["pair_count"], 2)
            self.assertEqual(saved["local_result"]["full_local_score"], "NOT_COMPUTABLE")
            self.assertFalse(saved["local_result"]["promotion_eligible"])

            repeated = run_cli(*common)
            self.assertEqual(repeated.returncode, 0, repeated.stdout + repeated.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_REUSED", repeated.stdout)
            self.assertIn("IDEMPOTENT_NOOP=YES", repeated.stdout)
            self.assertEqual(manifest.read_bytes(), saved_bytes)
            local_report = run_cli("report", "--manifest", str(manifest))
            self.assertEqual(local_report.returncode, 0, local_report.stdout + local_report.stderr)
            self.assertIn("RECORD_STATUS=PASS", local_report.stdout)
            self.assertIn("BACKEND_STATUS=NOT_REQUESTED", local_report.stdout)
            self.assertIn("LOCAL_SCORE_COMPLETENESS=INCOMPLETE", local_report.stdout)
            self.assertIn("REPORT_STATUS=PASS", local_report.stdout)

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
            self.assertEqual(finalized["backend_status"], "PASS")
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
            self.assertIn("MECHANISM_REVIEW=NO_TOKEN_OVERLAP_SIGNAL", result.stdout)
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
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("PATCH_SIMILARITY=0.000000", result.stdout)
            self.assertIn("MECHANISM_OVERLAP=0.692308", result.stdout)
            self.assertIn("MECHANISM_OVERLAP_GATE_STATUS=SIGNAL_ONLY_NOT_A_BLOCKER", result.stdout)
            self.assertIn("MECHANISM_ORTHOGONALITY=UNPROVEN", result.stdout)
            self.assertIn("MECHANISM_REVIEW=MAIN_READ_ONLY_REVIEW_SIGNAL", result.stdout)
            self.assertIn("DEDUP_STATUS=PATCH_GATE_PASS_MECHANISM_REVIEW_SIGNAL", result.stdout)
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
            self.assertIn("MECHANISM_REVIEW=NO_TOKEN_OVERLAP_SIGNAL", result.stdout)
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
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("PATCH_SIMILARITY=0.000000", result.stdout)
            self.assertIn("MECHANISM_OVERLAP=0.750000", result.stdout)
            self.assertIn("MECHANISM_OVERLAP_GATE_STATUS=SIGNAL_ONLY_NOT_A_BLOCKER", result.stdout)
            self.assertIn("MECHANISM_REVIEW_SIGNAL=NONZERO_TOKEN_OVERLAP", result.stdout)
            self.assertIn("DEDUP_STATUS=PATCH_GATE_PASS_MECHANISM_REVIEW_SIGNAL", result.stdout)

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
            self.assertIn("E2E_STATUS=INCOMPLETE", resumed.stdout)
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
            self.assertIn("E2E_STATUS=INCOMPLETE", report.stdout)
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
            self.assertIn("RECORD_STATUS=PASS", first.stdout)
            self.assertIn("BACKEND_STATUS=NOT_REQUESTED", first.stdout)
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
            self.assertEqual(record["record_status"], "PASS")
            self.assertEqual(record["backend_status"], "NOT_REQUESTED")
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
            self.assertIn("RECORD_STATUS=PASS", report.stdout)
            self.assertIn("BACKEND_STATUS=NOT_REQUESTED", report.stdout)
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
            self.assertIn("BACKEND_STATUS=NOT_REQUESTED", manifest_report.stdout)
            self.assertIn("LOCAL_SCORE_COMPLETENESS=INCOMPLETE", manifest_report.stdout)
            self.assertIn("REPORT_STATUS=PASS", manifest_report.stdout)

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

    def test_record_manifest_request_defaults_cycle_to_four_stages_without_e2e_claim(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-cycle-contract-", dir=str(CLI.parent)) as temp:
            root = Path(temp)
            checkpoint = root / "checkpoint.json"
            manifest = root / "record.json"
            source = CLI.parent / "public-crossrow-v001" / "Candidate.asc"
            artifact = CLI.parent / "public-crossrow-v001" / "build" / "crossrow_v001_paired_runner"
            raw = CLI.parent / "crossrow-public-paired-local-device3.tsv"
            result = run_cli(
                "cycle",
                "--checkpoint", str(checkpoint),
                "--source", str(source),
                "--artifact", str(artifact),
                "--raw", str(raw),
                "--record-manifest", str(manifest),
                "--record-evidence", str(source),
                "--record-backend", "",
            )
            self.assertEqual(result.returncode, 78, result.stdout + result.stderr)
            self.assertTrue(checkpoint.is_file())
            payload = json.loads(checkpoint.read_text())
            self.assertEqual(payload["stage_order"], ["build", "correctness", "local", "record"])
            self.assertNotEqual(payload.get("e2e_status"), "PASS")
            self.assertIn("CURRENT_STAGE=build", result.stdout)
            self.assertNotIn("E2E_STATUS=PASS", result.stdout)
            self.assertFalse(manifest.exists())

    def test_route_summary_reports_full_scores_quality_and_reliable_best(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-summary-", dir=str(CLI.parent)) as temp:
            route_root = Path(temp)
            proxy_source = route_root / "shape-proxy.asc"
            proxy_artifact = route_root / "shape-proxy.o"
            proxy_raw = route_root / "shape-proxy.tsv"
            proxy_manifest = route_root / "shape-proxy-record.json"
            proxy_source.write_text("shape proxy, not a kernel revision\n")
            proxy_artifact.write_text("shape proxy artifact\n")
            write_raw(proxy_raw, pairs=4)
            proxy = run_cli_with_route_root(
                route_root,
                "record",
                "--source", str(proxy_source),
                "--artifact", str(proxy_artifact),
                "--raw", str(proxy_raw),
                "--manifest", str(proxy_manifest),
                "--evidence", str(proxy_source),
                "--backend", "",
            )
            self.assertEqual(proxy.returncode, 0, proxy.stdout + proxy.stderr)
            empty_summary = run_cli_with_route_root(route_root, "report", "--route-summary")
            self.assertEqual(empty_summary.returncode, 0, empty_summary.stdout + empty_summary.stderr)
            self.assertIn("KERNEL_REVISION_COUNT=0", empty_summary.stdout)
            self.assertIn("KERNEL_REVISIONS=NONE", empty_summary.stdout)
            self.assertIn("LATEST_REVISION=NONE", empty_summary.stdout)
            self.assertIn("BEST_LOCAL_REVISION=NONE", empty_summary.stdout)
            self.assertIn("SUMMARY_SCOPE=ROUTE_ROOT_ONLY", empty_summary.stdout)
            self.assertIn("SCOREBOARD_WRITTEN=NO", empty_summary.stdout)

            fixtures = {}
            for revision, scale in (("V001", 0.7), ("V002", 0.5), ("V003", 1.2)):
                fixture = create_canonical_fixture(route_root, revision, scale)
                recorded = run_canonical_record(route_root, fixture, revision)
                self.assertEqual(recorded.returncode, 0, recorded.stdout + recorded.stderr)
                self.assertIn("FULL_LOCAL_SCORE=", recorded.stdout)
                self.assertIn("RECORD_STATUS=PASS", recorded.stdout)
                self.assertIn("BACKEND_STATUS=NOT_REQUESTED", recorded.stdout)
                payload = json.loads(fixture["manifest"].read_text())
                payload["recorded_utc"] = {
                    "V001": "2026-10-11T10:00:00Z",
                    "V002": "2026-10-11T11:00:00Z",
                    "V003": "2026-10-11T12:00:00Z",
                }[revision]
                fixture["manifest"].write_text(json.dumps(payload, indent=2) + "\n")
                fixtures[revision] = fixture

            slow_report = run_cli_with_route_root(
                route_root, "report", "--manifest", str(fixtures["V003"]["manifest"])
            )
            self.assertEqual(slow_report.returncode, 0, slow_report.stdout + slow_report.stderr)
            slow_record = json.loads(fixtures["V003"]["manifest"].read_text())
            self.assertLess(float(slow_record["local_result"]["canonical"]["reported_score"]), 100.0)
            self.assertIn("LOCAL_SCORE_COMPLETENESS=COMPLETE", slow_report.stdout)
            self.assertIn("REPORT_STATUS=PASS", slow_report.stdout)
            self.assertIn("PROMOTION_ELIGIBLE=NO", slow_report.stdout)

            summary = run_cli_with_route_root(route_root, "report", "--route-summary")
            self.assertEqual(summary.returncode, 0, summary.stdout + summary.stderr)
            self.assertIn("KERNEL_REVISION_COUNT=3", summary.stdout)
            self.assertIn("KERNEL_REVISIONS=V001,V002,V003", summary.stdout)
            self.assertIn("LATEST_REVISION=V003", summary.stdout)
            self.assertIn("LATEST_SCORE=", summary.stdout)
            self.assertIn("LATEST_QUALITY=", summary.stdout)
            self.assertIn("BEST_LOCAL_REVISION=NONE", summary.stdout)
            self.assertIn("BEST_LOCAL_QUALITY=NONE", summary.stdout)
            self.assertIn("REPORT_STATUS=PASS", summary.stdout)
            self.assertEqual(summary.stdout.count("KERNEL_REVISION=V00"), 3)
            self.assertIn("REVISION_SCORE=", summary.stdout)
            self.assertIn("REVISION_QUALITY=", summary.stdout)
            self.assertEqual(summary.stdout.count("REVISION_RELIABLE=NO"), 3)

    def test_public_canonical_receipt_and_pinned_paths_are_identity_bound(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-public-canonical-", dir=str(CLI.parent)) as temp:
            route_root = Path(temp)
            fixture = create_public_canonical_fixture(route_root, as_git_objects=True)
            local = run_cli_with_route_root(
                route_root,
                "local",
                "--stats-only",
                "--source", str(fixture["source"]),
                "--artifact-receipt", str(fixture["receipt"]),
                "--canonical-measurements", str(fixture["measurements"]),
                "--canonical-purpose", "historical-tool-validation",
                "--canonical-revision", "V003",
                "--expected-parent-artifact-sha", "eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71",
            )
            self.assertEqual(local.returncode, 0, local.stdout + local.stderr)
            self.assertIn("CANONICAL_CORE_CASES_COMPLETE=4/4", local.stdout)
            self.assertIn("CANONICAL_LOCAL_SCORE=60.477249", local.stdout)
            self.assertIn("CANONICAL_SCORE_SCOPE=historical-tool-validation", local.stdout)
            self.assertIn("PROMOTION_ELIGIBLE=NO", local.stdout)

            manifest = route_root / "public-canonical-record.json"
            recorded = run_cli_with_route_root(
                route_root,
                "record",
                "--source", str(fixture["source"]),
                "--artifact-receipt", str(fixture["receipt"]),
                "--raw", str(fixture["measurements"]),
                "--canonical-measurements", str(fixture["measurements"]),
                "--canonical-purpose", "historical-tool-validation",
                "--canonical-revision", "V003",
                "--expected-parent-artifact-sha", "eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71",
                "--build-receipt", str(fixture["receipt"]),
                "--correctness-receipt", str(fixture["correctness"]),
                "--manifest", str(manifest),
                "--evidence", str(fixture["source"]),
                "--evidence", str(fixture["parent"]),
                "--evidence", str(fixture["receipt"]),
                "--evidence", str(fixture["correctness"]),
                "--backend", "",
            )
            self.assertEqual(recorded.returncode, 0, recorded.stdout + recorded.stderr)
            payload = json.loads(manifest.read_text())
            self.assertEqual(payload["artifact"]["identity_mode"], "PUBLIC_BUILD_RECEIPT")
            self.assertEqual(payload["artifact"]["sha256"], fixture["artifact_sha"])
            self.assertEqual(payload["local_result"]["canonical"]["reported_score"], "60.477249")
            self.assertEqual(payload["local_result"]["canonical"]["score_scope"], "historical-tool-validation")

            report = run_cli_with_route_root(route_root, "report", "--manifest", str(manifest))
            self.assertEqual(report.returncode, 0, report.stdout + report.stderr)
            self.assertIn("REPORT_STATUS=PASS", report.stdout)
            self.assertIn("BUILD_STATUS=PASS", report.stdout)
            self.assertIn("CORRECTNESS_STATUS=PASS", report.stdout)
            self.assertIn("LOCAL_SCORE_COMPLETENESS=COMPLETE", report.stdout)
            self.assertIn("CANONICAL_LOCAL_SCORE=60.477249", report.stdout)
            self.assertIn("FULL_LOCAL_SCORE=NOT_COMPUTABLE", report.stdout)
            self.assertIn("PROMOTION_ELIGIBLE=NO", report.stdout)

            repeated = run_cli_with_route_root(
                route_root,
                "record",
                "--source", str(fixture["source"]),
                "--artifact-receipt", str(fixture["receipt"]),
                "--raw", str(fixture["measurements"]),
                "--canonical-measurements", str(fixture["measurements"]),
                "--canonical-purpose", "historical-tool-validation",
                "--canonical-revision", "V003",
                "--expected-parent-artifact-sha", "eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71",
                "--build-receipt", str(fixture["receipt"]),
                "--correctness-receipt", str(fixture["correctness"]),
                "--manifest", str(manifest),
                "--evidence", str(fixture["source"]),
                "--evidence", str(fixture["parent"]),
                "--evidence", str(fixture["receipt"]),
                "--evidence", str(fixture["correctness"]),
                "--backend", "",
            )
            self.assertEqual(repeated.returncode, 0, repeated.stdout + repeated.stderr)
            self.assertIn("RECORD_MANIFEST_STATUS=RECORD_MANIFEST_REUSED", repeated.stdout)

            bad_receipt = route_root / "bad-build.json"
            bad_payload = json.loads(fixture["receipt"].read_text())
            bad_payload["EXE_SHA"] = "0" * 64
            bad_receipt.write_text(json.dumps(bad_payload))
            bad = run_cli_with_route_root(
                route_root,
                "record",
                "--source", str(fixture["source"]),
                "--artifact-receipt", str(bad_receipt),
                "--expected-artifact-sha", fixture["artifact_sha"],
                "--raw", str(fixture["measurements"]),
                "--canonical-measurements", str(fixture["measurements"]),
                "--canonical-purpose", "historical-tool-validation",
                "--canonical-revision", "V003",
                "--expected-parent-artifact-sha", "eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71",
                "--manifest", str(route_root / "bad-record.json"),
                "--evidence", str(fixture["source"]),
                "--backend", "",
            )
            self.assertEqual(bad.returncode, 65, bad.stdout + bad.stderr)
            self.assertIn("ARTIFACT_SHA_MISMATCH", bad.stdout)

            invalid_prefix = route_root / "invalid-prefix.tsv"
            invalid_prefix.write_text(
                fixture["measurements"].read_text().replace(
                    f"gitobj://{CANONICAL_COMMIT}/本地实验/MAIN2-CANONICAL-V1/",
                    f"gitobj://{CANONICAL_COMMIT}/归档/not-allowed/",
                    1,
                )
            )
            rejected_prefix = run_cli_with_route_root(
                route_root,
                "local",
                "--stats-only",
                "--source", str(fixture["source"]),
                "--artifact-receipt", str(fixture["receipt"]),
                "--canonical-measurements", str(invalid_prefix),
                "--canonical-purpose", "historical-tool-validation",
                "--canonical-revision", "V003",
                "--expected-parent-artifact-sha", "eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71",
            )
            self.assertEqual(rejected_prefix.returncode, 66, rejected_prefix.stdout + rejected_prefix.stderr)
            self.assertIn("CANONICAL_RAW_GIT_OBJECT_OUT_OF_SCOPE", rejected_prefix.stdout)

            external_path = route_root / "external-path.tsv"
            external_path.write_text(
                fixture["measurements"].read_text().replace(
                    f"gitobj://{CANONICAL_COMMIT}/本地实验/MAIN2-CANONICAL-V1/",
                    "/tmp/arbitrary-file/本地实验/MAIN2-CANONICAL-V1/",
                    1,
                )
            )
            rejected_external = run_cli_with_route_root(
                route_root,
                "local",
                "--stats-only",
                "--source", str(fixture["source"]),
                "--artifact-receipt", str(fixture["receipt"]),
                "--canonical-measurements", str(external_path),
                "--canonical-purpose", "historical-tool-validation",
                "--canonical-revision", "V003",
                "--expected-parent-artifact-sha", "eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71",
            )
            self.assertEqual(rejected_external.returncode, 66, rejected_external.stdout + rejected_external.stderr)
            self.assertIn("CANONICAL_PUBLIC_EXTERNAL_FILE_REJECTED", rejected_external.stdout)

    def test_public_crossrow_files_match_pinned_ref_and_canonical_host_raw_is_per_side(self):
        public_root = "本地实验/CROSSROW-EPI-PROLOGUE-X/V001"
        route_root = CLI.parent / "public-crossrow-v001"
        for relative in (
            "CMakeLists.txt",
            "Candidate.asc",
            "Parent.asc",
            "compile_adapter.asc",
            "support/CMakeLists.txt",
            "support/candidate_adapter.asc",
            "support/parent_adapter.asc",
            "support/runner_abi.h",
            "support/runner_main.cpp",
        ):
            with self.subTest(relative=relative):
                self.assertEqual(
                    (route_root / relative).read_bytes(),
                    git_object_at(PUBLIC_R1_CROSSROW_COMMIT, public_root + "/" + relative),
                )

        crossrow_runner = git_object_at(
            PUBLIC_R1_CROSSROW_COMMIT, public_root + "/support/runner_main.cpp"
        ).decode()
        self.assertIn("--mode correctness|local --device ID --rows M --width D --blocks N --dtype fp16", crossrow_runner)
        self.assertIn("--output FILE", crossrow_runner)
        self.assertIn("sample\\torder\\tparent_device_us\\tcandidate_device_us\\tdelta_device_us", crossrow_runner)
        self.assertIn("parent=W3-CROSSROW-FULL-PIPELINE-V012", crossrow_runner)
        self.assertNotIn("aclrtResetDevice", crossrow_runner)

        canonical_benchmark = git_object_at(
            CANONICAL_COMMIT, "工具/main2-canonical-benchmark.mjs"
        ).decode()
        canonical_runner = git_object_at(
            CANONICAL_COMMIT, "工具/main2-canonical-harness/runner_ref.inc"
        ).decode()
        self.assertIn("const argv=[String(device),c.ROWS,c.WIDTH,c.DTYPE_CODE,prefix,String(warmup),String(samples),'1','0','1']", canonical_benchmark)
        self.assertIn("block\\trep\\tdevice_us\\thost_wall_us", canonical_runner)
        self.assertIn("for(const c of frozenSuite())", canonical_benchmark)

        with tempfile.TemporaryDirectory(prefix=".w5ctl-canonical-one-side-", dir=str(CLI.parent)) as temp:
            raw = Path(temp) / "one-side-raw.tsv"
            raw.write_text("block\trep\tdevice_us\thost_wall_us\n0\t0\t10.0\t12.0\n")
            result = run_cli("local", "--stats-only", "--raw", str(raw))
            self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
            self.assertIn("MEASUREMENT_QUALITY=RAW_PARSE_ERROR", result.stdout)
            self.assertIn("LOCAL_STATISTICS_REASON=RAW_ROW_INVALID:1", result.stdout)
            self.assertIn("FULL_LOCAL_SCORE=NOT_COMPUTABLE", result.stdout)

    def test_crossrow_backend_args_bind_raw_path_and_preserve_shape_only_verdict(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-crossrow-backend-", dir=str(CLI.parent)) as temp:
            route_root = Path(temp)
            source = route_root / "Candidate.asc"
            artifact = route_root / "paired_runner"
            raw = route_root / "paired.tsv"
            backend = route_root / "crossrow_backend.py"
            source.write_text("published Crossrow V001 source identity\n")
            artifact.write_text("runner identity placeholder for backend contract unit test\n")
            write_executable(
                backend,
                """
                #!/usr/bin/env python3
                import os
                import sys
                from pathlib import Path
                expected = [
                    "--mode", "local", "--device", "3", "--rows", "16",
                    "--width", "6144", "--blocks", "8", "--dtype", "fp16",
                    "--warmups", "45", "--repeats", "21", "--output",
                    os.environ["W5CTL_RAW_PATH"],
                ]
                if sys.argv[1:] != expected:
                    print("BACKEND_ARG_CONTRACT_MISMATCH=" + repr(sys.argv[1:]))
                    raise SystemExit(97)
                print("BACKEND_ARG_CONTRACT=PASS")
                rows = [
                    "# LOCAL_PROXY_ONLY parent=W3-CROSSROW-FULL-PIPELINE-V012 candidate=W5-R01-CROSSROW-EPI-PROLOGUE-X-V001 warmups=45 paired_samples=3",
                    "sample\\torder\\tparent_device_us\\tcandidate_device_us\\tdelta_device_us\\tparent_wall_us\\tcandidate_wall_us\\tdelta_wall_us",
                    "1\\tPC\\t10\\t9\\t-1\\t12\\t11\\t-1",
                    "2\\tCP\\t10\\t9\\t-1\\t12\\t11\\t-1",
                    "3\\tPC\\t10\\t9\\t-1\\t12\\t11\\t-1",
                ]
                Path(os.environ["W5CTL_RAW_PATH"]).write_text("\\n".join(rows) + "\\n")
                """,
            )
            result = run_cli_with_route_root(
                route_root,
                "local",
                "--source", str(source),
                "--artifact", str(artifact),
                "--raw", str(raw),
                "--backend", str(backend),
                "--backend-arg=--mode",
                "--backend-arg=local",
                "--backend-arg=--device",
                "--backend-arg=3",
                "--backend-arg=--rows",
                "--backend-arg=16",
                "--backend-arg=--width",
                "--backend-arg=6144",
                "--backend-arg=--blocks",
                "--backend-arg=8",
                "--backend-arg=--dtype",
                "--backend-arg=fp16",
                "--backend-arg=--warmups",
                "--backend-arg=45",
                "--backend-arg=--repeats",
                "--backend-arg=21",
                "--backend-arg=--output",
                "--backend-arg=" + str(raw),
            )
            self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
            self.assertIn("BACKEND_ARG_CONTRACT=PASS", result.stdout)
            self.assertIn("BACKEND_RC=0", result.stdout)
            self.assertIn("RAW_PAIR_COUNT=3", result.stdout)
            self.assertIn("MEASUREMENT_QUALITY=UNCLASSIFIED", result.stdout)
            self.assertIn("SHAPE_LOCAL_VERDICT=SHAPE_UNCLASSIFIED_NO_PROMOTION", result.stdout)
            self.assertIn("FULL_LOCAL_SCORE=NOT_COMPUTABLE", result.stdout)
            self.assertIn("PROMOTION_ELIGIBLE=NO", result.stdout)

    def test_canonical_aggregation_rejects_missing_core_parent_identity_and_order(self):
        with tempfile.TemporaryDirectory(prefix=".w5ctl-canonical-reject-", dir=str(CLI.parent)) as temp:
            route_root = Path(temp)
            fixture = create_canonical_fixture(route_root, "V004", 0.7)
            with fixture["measurements"].open() as stream:
                base_rows = list(csv.DictReader(stream, delimiter="\t"))
            headers = list(base_rows[0].keys())

            cases = []
            missing_core = [row for row in base_rows if row["CASE_ID"] != "C16"]
            cases.append(("missing-core", missing_core, (), "CANONICAL_CORE_INCOMPLETE"))
            missing_parent = [dict(row) for row in base_rows]
            next(row for row in missing_parent if row["CASE_ID"] == "C12" and row["SIDE"] == "parent")["EXE_SHA"] = ""
            cases.append(("missing-parent-artifact", missing_parent, (), "CANONICAL_PARENT_ARTIFACT_SHA_MISSING"))
            mixed_parent = [dict(row) for row in base_rows]
            parents = [row for row in mixed_parent if row["CASE_ID"] == "C12" and row["SIDE"] == "parent"]
            parents[1]["EXE_SHA"] = "f" * 64
            cases.append(("mixed-parent-artifact", mixed_parent, (), "CANONICAL_PARENT_ARTIFACT_SHA_MIXED"))
            unpinned_parent = [dict(row) for row in base_rows]
            for row in unpinned_parent:
                if row["SIDE"] == "parent":
                    row["EXE_SHA"] = "f" * 64
            cases.append(("unpinned-parent-artifact", unpinned_parent, (), "CANONICAL_PARENT_ARTIFACT_SHA_PIN_MISMATCH"))
            invalid_order = [dict(row) for row in base_rows]
            invalid_order[0], invalid_order[1] = invalid_order[1], invalid_order[0]
            cases.append(("invalid-pc-order", invalid_order, (), "CANONICAL_PC_CP_ORDER_INVALID"))
            cases.append((
                "expected-parent-artifact-mismatch",
                [dict(row) for row in base_rows],
                ("--expected-parent-artifact-sha", "0" * 64),
                "CANONICAL_PARENT_ARTIFACT_SHA_MISMATCH",
            ))

            for name, rows, extra_args, expected_error in cases:
                with self.subTest(name=name):
                    measurements = route_root / f"{name}.tsv"
                    manifest = route_root / f"{name}-record.json"
                    with measurements.open("w", newline="") as stream:
                        writer = csv.DictWriter(stream, fieldnames=headers, delimiter="\t", lineterminator="\n")
                        writer.writeheader()
                        writer.writerows(rows)
                    variant = dict(fixture, measurements=measurements, manifest=manifest)
                    result = run_canonical_record(route_root, variant, "V004", extra_args)
                    self.assertEqual(result.returncode, 66, result.stdout + result.stderr)
                    self.assertIn(expected_error, result.stdout)
                    self.assertFalse(manifest.exists())


if __name__ == "__main__":
    unittest.main(verbosity=2)
