import itertools
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FILES = {
    "R031": ROOT / "phase4/workspaces/R31A/R31A-V000-original.asc",
    "R31A": ROOT / "phase4/workspaces/R31A/R31A-V002-submission.asc",
    "R31B": ROOT / "phase4/workspaces/R31B/R31B-V001-WIDE-PANEL-RESIDENT_kernel.asc",
    "A001": ROOT / "phase4/workspaces/A001/submission_v008.asc",
    "G001": ROOT / "phase4/workspaces/G001/submission.asc",
    "H001": ROOT / "phase4/workspaces/H001/submission.asc",
    "I001": ROOT / "phase4/workspaces/I001/submission.asc",
}

API_NAMES = {
    "DataCopy", "DataCopyPad", "DataCopyPadParams", "LocalTensor", "GlobalTensor",
    "TQue", "TPipe", "AllocTensor", "FreeTensor", "EnQue", "DeQue", "SetFlag",
    "WaitFlag", "ReduceSum", "Cast", "Adds", "Muls", "Add", "Mul", "Sub", "Div",
    "SyncAll", "SetGlobalBuffer", "GetBlockIdx", "GetBlockNum", "GetBlockIdx",
}

TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z_0-9]*|0x[0-9A-Fa-f]+|[0-9]+(?:\.[0-9]+)?|==|!=|<=|>=|&&|\|\||\+\+|--|->|.\S")


def body(path):
    if not path.exists():
        return None
    text = path.read_text(errors="replace")
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    text = re.sub(r"^\s*#.*$", " ", text, flags=re.M)
    text = re.sub(r"\b(?:TensorInfo|TensorGroupInfo|aclrtStream|GM_ADDR)\b", "ABI", text)
    text = re.sub(r"extern\s+\"C\"\s+void\s+run_kernel\b.*", " ", text, flags=re.S)
    def normalize_tokens(tokens):
        out = []
        for token in tokens:
            if token.isspace():
                continue
            if token in API_NAMES:
                out.append("API")
            elif token in {"half", "bfloat16_t", "float", "uint32_t", "uint64_t", "int32_t", "int64_t"}:
                out.append("TYPE")
            else:
                out.append(token)
        return out

    lines = []
    for raw_line in text.splitlines():
        line_tokens = normalize_tokens(TOKEN_RE.findall(raw_line))
        if line_tokens:
            lines.append(" ".join(line_tokens))
    tokens = normalize_tokens(TOKEN_RE.findall("\n".join(lines)))
    return {"lines": lines, "tokens": tokens}


def shingles(tokens, width=8):
    return {tuple(tokens[i:i + width]) for i in range(max(0, len(tokens) - width + 1))}


def line_similarity(left, right):
    a = set(" ".join(left).split())
    b = set(" ".join(right).split())
    return 1.0 if not a and not b else (2.0 * len(a & b) / (len(a) + len(b)))


def token_similarity(left, right):
    a, b = set(left), set(right)
    return 1.0 if not a and not b else (2.0 * len(a & b) / (len(a) + len(b)))


def shingle_similarity(left, right):
    a, b = shingles(left), shingles(right)
    return 1.0 if not a and not b else (len(a & b) / max(1, len(a | b)))


def long_blocks(left, right, minimum=12):
    right_lines = [tuple(right[i:i + minimum]) for i in range(max(0, len(right) - minimum + 1))]
    right_set = set(right_lines)
    matches = [tuple(left[i:i + minimum]) in right_set for i in range(max(0, len(left) - minimum + 1))]
    return sum(current and (index == 0 or not matches[index - 1]) for index, current in enumerate(matches))


def main():
    normalized = {name: body(path) for name, path in FILES.items()}
    rows = ["left\tright\tline_similarity\ttoken_similarity\tshingle_similarity\tidentical_long_blocks\tinterpretation"]
    for left, right in itertools.combinations(FILES, 2):
        a, b = normalized[left], normalized[right]
        if a is None or b is None:
            continue
        ls = line_similarity(a["lines"], b["lines"])
        ts = token_similarity(a["tokens"], b["tokens"])
        ss = shingle_similarity(a["tokens"], b["tokens"])
        blocks = long_blocks(a["lines"], b["lines"])
        if {left, right} <= {"R031", "R31A", "R31B"}:
            interpretation = "INFRA_REUSE_EXPECTED_R031_TRACK"
        elif blocks >= 3 or ss >= 0.18:
            interpretation = "POSSIBLE_CONTEXT_LEAK_REVIEW"
        else:
            interpretation = "LOW_COMPUTE_REUSE_SIGNAL"
        rows.append(f"{left}\t{right}\t{ls:.6f}\t{ts:.6f}\t{ss:.6f}\t{blocks}\t{interpretation}")
    (ROOT / "phase4/control/code-similarity.tsv").write_text("\n".join(rows) + "\n")


if __name__ == "__main__":
    main()
