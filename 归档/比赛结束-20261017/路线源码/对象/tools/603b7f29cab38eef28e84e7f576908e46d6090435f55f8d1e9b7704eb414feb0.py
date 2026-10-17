"""Compare device computation separately from launch and conversion utilities.

Line metric: ordered SequenceMatcher ratio on substantive normalized lines.
Token metric: multiset Dice, retaining identifier and numeric frequencies.
Shingle metric: Jaccard of consecutive eight-token sequences.
Long blocks: ordered matches of at least 12 substantive lines and 80 tokens.
These are text signals, not proof of agent access or semantic equivalence.
"""

import argparse
from collections import Counter
from difflib import SequenceMatcher
import itertools
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FILES = {
    "R031": ROOT / "phase4/workspaces/R31A/R31A-V000-original.asc",
    "R31A": ROOT / "phase4/workspaces/R31A/R31A-V002-submission.asc",
    "R31B": ROOT / "phase4/workspaces/R31B/R31B-V002-WIDE-FULL-Y-CACHE_kernel.asc",
    "A001": ROOT / "phase4/workspaces/A001/submission_v008.asc",
    "G001": ROOT / "phase4/workspaces/G001/src/add_rms_norm_bias.cpp",
    "H001": ROOT / "phase4/workspaces/H001/src/add_rms_norm_bias_kernel.cpp",
    "I001": ROOT / "phase4/workspaces/I001/submission.asc",
}

API_NAMES = {
    "DataCopy", "DataCopyPad", "DataCopyPadParams", "LocalTensor", "GlobalTensor",
    "TQue", "TPipe", "AllocTensor", "FreeTensor", "EnQue", "DeQue", "SetFlag",
    "WaitFlag", "ReduceSum", "Cast", "Adds", "Muls", "Add", "Mul", "Sub", "Div",
    "SyncAll", "SetGlobalBuffer", "GetBlockIdx", "GetBlockNum", "TBuf",
    "InitBuffer", "Get", "GetValue", "SetValue", "PipeBarrier", "Duplicate",
    "Sqrt", "Rsqrt", "Reciprocal", "IsSameType", "DataCopyExtParams",
    "DataCopyParams", "DataCopyPadExtParams", "LocalMemAllocator",
    "Sum", "WholeReduceSum", "BlockReduceSum", "Brcb", "Alloc",
}

UTILITY_NAMES = {
    "ToFloat", "FromFloat", "CopyIn", "CopyOut", "CopyRow", "CopyRows",
    "StoreRows", "AlignUp", "AlignBytes", "RoundUp", "CeilDiv",
}
TOKEN_RE = re.compile(
    r"0[xX][0-9A-Fa-f]+[uUlL]*|(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?[fFuUlL]*"
    r"|[A-Za-z_]\w*|::|==|!=|<=|>=|&&|\|\||\+\+|--|->|<<|>>|\+=|-=|\*=|/=|[^\s]"
)
NONCODE_RE = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
DEVICE_START_RE = re.compile(r"\b(?:__aicore__|__global__)\b[^;{}]*\{")


def blank(match):
    return re.sub(r"[^\n]", " ", match.group(0))


def code_only(text):
    text = NONCODE_RE.sub(blank, text)
    return re.sub(r"^[ \t]*#(?:[^\n]*\\\n)*[^\n]*", blank, text, flags=re.M)


def device_functions(text):
    """Balanced-brace extraction for the annotated functions in these sources."""
    clean = code_only(text)
    end = 0
    for match in DEVICE_START_RE.finditer(clean):
        if match.start() < end:
            continue
        signature = clean[match.start():match.end() - 1]
        name_match = re.search(r"\b([A-Za-z_]\w*)(?:\s*<[^(){}]+>)?\s*\(", signature)
        if not name_match:
            continue
        depth, end = 1, match.end()
        while end < len(clean) and depth:
            depth += (clean[end] == "{") - (clean[end] == "}")
            end += 1
        if depth:
            raise ValueError(f"Unbalanced function {name_match.group(1)}")
        yield name_match.group(1), match.start(), end, "__global__" in signature


def normalize(text):
    clean = code_only(text).replace("AscendC::", "")
    lines = []
    for raw in clean.splitlines():
        tokens = [token for token in TOKEN_RE.findall(raw) if token not in API_NAMES]
        if any(re.match(r"[A-Za-z_0-9]", token) for token in tokens):
            lines.append(" ".join(tokens))
    return {"lines": lines, "tokens": TOKEN_RE.findall("\n".join(lines))}


def split_source(text):
    clean = code_only(text)
    compute = []
    remainder = list(clean)
    names = set()
    comments = []
    for name, start, end, global_entry in device_functions(text):
        if global_entry or name in UTILITY_NAMES:
            continue
        compute.append(clean[start:end])
        names.add(name)
        for match in re.finditer(r"//[^\n]*|/\*.*?\*/", text[start:end], re.S):
            words = re.findall(r"[A-Za-z_0-9]+", match.group(0))
            if len(words) >= 12:
                comments.append(tuple(words))
        remainder[start:end] = " " * (end - start)

    # Preserve distinctive storage and compile-time choices outside functions.
    remainder_text = "".join(remainder)
    declarations = re.compile(
        r"^[ \t]*(?:(?:static\s+)?constexpr\b|(?:AscendC::)?(?:TBuf|TQue|GlobalTensor)\s*<)[^;{}]*;",
        re.M,
    )
    for match in declarations.finditer(remainder_text):
        compute.append(match.group(0))
        remainder[match.start():match.end()] = " " * (match.end() - match.start())

    return {
        "compute": normalize("\n".join(compute)),
        "infra": normalize("".join(remainder)),
        "names": names,
        "comments": set(comments),
    }


def body(path):
    if not path.exists():
        return None
    result = split_source(path.read_text(errors="replace"))
    if not result["compute"]["tokens"]:
        raise ValueError(f"No device computation extracted from {path}")
    return result


def shingles(tokens, width=8):
    return {tuple(tokens[i:i + width]) for i in range(max(0, len(tokens) - width + 1))}


def line_similarity(left, right):
    return SequenceMatcher(None, left, right, autojunk=False).ratio()


def token_similarity(left, right):
    a, b = Counter(left), Counter(right)
    total = len(left) + len(right)
    return 1.0 if not total else 2.0 * sum((a & b).values()) / total


def shingle_similarity(left, right):
    a, b = shingles(left), shingles(right)
    return 1.0 if not a and not b else (len(a & b) / max(1, len(a | b)))


def long_blocks(left, right, minimum=12):
    return sum(
        match.size >= minimum
        and sum(len(line.split()) for line in left[match.a:match.a + match.size]) >= 80
        for match in SequenceMatcher(None, left, right, autojunk=False).get_matching_blocks()
    )


def metrics(a, b):
    return (
        line_similarity(a["lines"], b["lines"]),
        token_similarity(a["tokens"], b["tokens"]),
        shingle_similarity(a["tokens"], b["tokens"]),
        long_blocks(a["lines"], b["lines"]),
    )


def self_test():
    source = '''
#include "kernel_operator.h"
struct TensorInfo { int dtype; };
template<typename T> __aicore__ inline float ToFloat(T x) { return (float)x; }
__aicore__ inline void ComputeSpecial() {
    float sum = 0;
    for (int c = 0; c < 32; ++c) { sum += c * c; }
    sink(sum);
}
__global__ __vector__ void Launch() { ComputeSpecial(); }
template<typename Info> void run_kernel(Info x) { host_only(x); }
'''
    a = split_source(source)
    assert "host_only" not in a["compute"]["tokens"]
    assert "ToFloat" not in a["compute"]["tokens"]
    assert "Launch" not in a["compute"]["tokens"]
    assert "ComputeSpecial" in a["compute"]["tokens"]
    changed_host = split_source(source.replace("host_only", "other_host"))
    assert a["compute"] == changed_host["compute"]
    changed_comment = split_source(source.replace("float sum = 0;", "float sum = 0; /* braces { } */"))
    assert a["compute"] == changed_comment["compute"]
    assert line_similarity(["a = 1", "b = 2"], ["b = 2", "a = 1"]) < 1
    assert token_similarity(["x", "x", "y"], ["x", "y", "y"]) < 1
    assert shingle_similarity(list("abcdefgh"), list("abcdefgh")) == 1
    assert metrics(a["compute"], a["compute"])[:3] == (1, 1, 1)
    print("self-test: 10 assertions passed")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
        return
    normalized = {name: body(path) for name, path in FILES.items()}
    rows = ["left\tright\tline_similarity\ttoken_similarity\tshingle_similarity\tidentical_long_blocks\tinterpretation"]
    for left, right in itertools.combinations(FILES, 2):
        a, b = normalized[left], normalized[right]
        if a is None or b is None:
            missing = ",".join(name for name, value in ((left, a), (right, b)) if value is None)
            rows.append(f"{left}\t{right}\t\t\t\t\tNOT_AVAILABLE:{missing}; COMPUTE_REUSE=unmeasured; INFRA_REUSE=unmeasured")
            continue
        ls, ts, ss, blocks = metrics(a["compute"], b["compute"])
        infra = metrics(a["infra"], b["infra"])
        shared_names = sorted(name for name in a["names"] & b["names"] if len(name) >= 22)
        shared_comments = len(a["comments"] & b["comments"])
        if {left, right} <= {"R031", "R31A", "R31B"}:
            signal = "EXPECTED_R031_EVOLUTION"
        elif blocks or shared_names or shared_comments or ss >= 0.18:
            signal = "POSSIBLE_CONTEXT_LEAK"
        else:
            signal = "LOW_TEXT_SIMILARITY; ACCESS_HISTORY_NOT_VERIFIED"
        interpretation = (
            f"COMPUTE_REUSE={signal}; INFRA_REUSE:line={infra[0]:.6f},token={infra[1]:.6f},"
            f"shingle={infra[2]:.6f},blocks={infra[3]}; shared_special_functions={','.join(shared_names) or 'none'};"
            f" identical_long_comments={shared_comments}"
        )
        rows.append(f"{left}\t{right}\t{ls:.6f}\t{ts:.6f}\t{ss:.6f}\t{blocks}\t{interpretation}")
    (ROOT / "phase4/control/code-similarity.tsv").write_text("\n".join(rows) + "\n")
    for name, value in normalized.items():
        if value is not None:
            print(f"{name}: {len(value['compute']['lines'])} compute lines, {len(value['compute']['tokens'])} compute tokens; {FILES[name].relative_to(ROOT)}")
    print("wrote phase4/control/code-similarity.tsv")


if __name__ == "__main__":
    main()
