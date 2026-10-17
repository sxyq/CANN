#!/usr/bin/env python3
"""Emit the FULL-R022 direct-invoke candidate from a small schedule spec."""

import argparse
import json
from pathlib import Path


TOKENS = {
    "ROUTE_LABEL": "route_label",
    "TILE_ELEMS": "tile_elems",
    "VECTOR_WIDTH": "vector_width",
    "BUILD_PROFILE": "build_profile",
    "CLASS_NAME": "class_name",
    "KERNEL_SYMBOL": "kernel_symbol",
}


def render(template: str, spec: dict) -> str:
    missing = [key for key in TOKENS.values() if key not in spec]
    if missing:
        raise ValueError("missing spec fields: " + ", ".join(missing))

    output = template
    for token, field in TOKENS.items():
        marker = "@@" + token + "@@"
        output = output.replace(marker, str(spec[field]))

    unresolved = sorted({part.split("@@", 1)[0] for part in output.split("@@")[1:]})
    if "@@" in output:
        raise ValueError("unresolved template marker: " + ", ".join(unresolved))
    required = [
        "run_kernel",
        "__global__ __vector__",
        "GeneratedSchedule::kTileElems",
        "CANN9",
    ]
    for marker in required:
        if marker not in output:
            raise ValueError("generated source missing required marker: " + marker)
    return output


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--spec", type=Path, required=True)
    parser.add_argument("--template", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    spec = json.loads(args.spec.read_text())
    source = render(args.template.read_text(), spec)
    args.output.write_text(source)
    print(
        "generated",
        args.output,
        "bytes=",
        len(source.encode()),
        "tile_elems=",
        spec["tile_elems"],
    )


if __name__ == "__main__":
    main()
