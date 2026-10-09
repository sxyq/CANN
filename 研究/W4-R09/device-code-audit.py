"""Read existing R09 ELF objects in memory; print evidence without writing inputs."""

from datetime import datetime, timezone
from difflib import SequenceMatcher, unified_diff
import json
from pathlib import Path
import struct
import subprocess


ROOT = Path("/home/data4t2/lelinfeng/cann/server_runs/W4-R09/V001")


class Elf:
    def __init__(self, data):
        assert data[:6] == b"\x7fELF\x02\x01"
        self.data = data
        header = struct.unpack_from("<16sHHIQQQIHHHHHH", data)
        self.kind, self.machine, self.flags = header[1], header[2], header[7]
        raw = [
            struct.unpack_from("<IIQQQQIIQQ", data, header[6] + i * header[11])
            for i in range(header[12])
        ]
        names = raw[header[13]]
        strings = data[names[4] : names[4] + names[5]]
        keys = ("name_offset", "type", "flags", "address", "offset", "size",
                "link", "info", "alignment", "entry_size")
        self.sections = []
        for index, values in enumerate(raw):
            section = dict(zip(keys, values))
            section["index"] = index
            section["name"] = strings[values[0] :].split(b"\0", 1)[0].decode()
            self.sections.append(section)
        self.named = {section["name"]: section for section in self.sections}
        table = self.named[".symtab"]
        strings = self.content(self.sections[table["link"]])
        self.symbols = []
        for offset in range(table["offset"], table["offset"] + table["size"],
                            table["entry_size"]):
            name, info, other, section, value, size = struct.unpack_from(
                "<IBBHQQ", data, offset)
            self.symbols.append({
                "name": strings[name:].split(b"\0", 1)[0].decode(),
                "type": info & 15, "binding": info >> 4, "visibility": other,
                "section": section, "value": value, "size": size,
            })
        self.functions = {s["name"]: s for s in self.symbols if s["type"] == 2}

    def content(self, section):
        if isinstance(section, str):
            section = self.named[section]
        if section["type"] == 8:
            return b""
        return self.data[section["offset"] : section["offset"] + section["size"]]

    def function_bytes(self, symbol):
        section = self.sections[symbol["section"]]
        offset = symbol["value"] - section["address"]
        data = self.content(section)[offset : offset + symbol["size"]]
        assert len(data) == symbol["size"]
        return data

    def relocations(self, symbol):
        section = self.sections[symbol["section"]]
        start = symbol["value"] - section["address"]
        result = []
        for table in self.sections:
            if table["type"] != 4 or table["info"] != symbol["section"]:
                continue
            for offset in range(table["offset"], table["offset"] + table["size"],
                                table["entry_size"]):
                place, info, addend = struct.unpack_from("<QQq", self.data, offset)
                relative = place - section["address"] if self.kind == 2 else place
                if not start <= relative < start + symbol["size"]:
                    continue
                target = self.symbols[info >> 32]
                target_section = self.sections[target["section"]] if (
                    0 < target["section"] < len(self.sections)) else None
                result.append({
                    "function_offset": relative - start,
                    "type": info & 0xFFFFFFFF,
                    "target_name": target["name"],
                    "target_section": target_section["name"] if target_section else None,
                    "target_section_offset": target["value"] - target_section["address"]
                    if target_section else target["value"],
                    "addend": addend,
                })
        return result

    def summary(self):
        return {"elf_type": self.kind, "machine": self.machine, "flags": self.flags,
                "size": len(self.data), "sections": self.sections,
                "functions": self.functions}


def state(path):
    st = path.stat()
    return {"size": st.st_size, "inode": st.st_ino,
            "mtime_ns": st.st_mtime_ns, "ctime_ns": st.st_ctime_ns}


def differences(parent, candidate):
    assert len(parent) % 4 == 0 and len(candidate) % 4 == 0
    p = [parent[i:i + 4] for i in range(0, len(parent), 4)]
    c = [candidate[i:i + 4] for i in range(0, len(candidate), 4)]
    spans = []
    equal_bytes = 0
    for tag, p0, p1, c0, c1 in SequenceMatcher(None, p, c, autojunk=False).get_opcodes():
        if tag == "equal":
            equal_bytes += 4 * (p1 - p0)
            continue
        spans.append({"kind": tag,
                      "parent_offset": 4 * p0, "parent_bytes": 4 * (p1 - p0),
                      "candidate_offset": 4 * c0, "candidate_bytes": 4 * (c1 - c0),
                      "parent_hex": parent[4 * p0:4 * p1].hex(),
                      "candidate_hex": candidate[4 * c0:4 * c1].hex()})
    return {"method": "exact sequence alignment of 4-byte units; not instruction decoding",
            "equal_bytes_in_aligned_runs": equal_bytes, "changed_spans": spans}


def compare(parent, candidate):
    sections = []
    for name in sorted(parent.named.keys() | candidate.named.keys()):
        ps, cs = parent.named.get(name), candidate.named.get(name)
        sections.append({
            "name": name,
            "parent_size": ps["size"] if ps else None,
            "candidate_size": cs["size"] if cs else None,
            "parent_address": ps["address"] if ps else None,
            "candidate_address": cs["address"] if cs else None,
            "content_equal": parent.content(ps) == candidate.content(cs) if ps and cs else False,
        })
    functions = []
    assert parent.functions.keys() == candidate.functions.keys()
    for name, ps in parent.functions.items():
        cs = candidate.functions[name]
        p, c = parent.function_bytes(ps), candidate.function_bytes(cs)
        pr, cr = parent.relocations(ps), candidate.relocations(cs)
        entry = {"symbol": name, "parent": ps, "candidate": cs,
                 "code_bytes_equal": p == c,
                 "parent_relocations": pr, "candidate_relocations": cr,
                 "function_relative_relocations_equal": pr == cr}
        if p != c:
            entry["difference"] = differences(p, c)
        functions.append(entry)
    return {"whole_embedded_elf_equal": parent.data == candidate.data,
            "sections": sections, "functions": functions}


report = {"route": "W4-R09", "revision": "V001",
          "utc": datetime.now(timezone.utc).isoformat(),
          "method": "Path.read_bytes plus in-memory ELF64 parsing; no objcopy, no compilation, no NPU run",
          "elf_reader_origin": "e697ddf35d4c42e735236e597338a148bfb437f2:研究/W4-R12/extract_parent_disassembly.py",
          "files": {}, "side_comparisons": {}, "parent_candidate_comparisons": {}}
objects = {}
for side in ("parent", "candidate"):
    paths = {
        "object": ROOT / "build" / f"CMakeFiles/w4r09_v001_{side}.dir/{side}_adapter.asc.o",
        "library": ROOT / "build" / f"libw4r09_v001_{side}.so",
    }
    objects[side] = {}
    for kind, path in paths.items():
        before = state(path)
        outer = Elf(path.read_bytes())
        embedded = {name: Elf(outer.content(name))
                    for name in (".aicore_binary", "__aicore_rel_binary")}
        objects[side][kind] = embedded
        report["files"][f"{side}_{kind}"] = {
            "path": str(path), "before": before, "after": state(path),
            "outer_device_sections": {name: outer.named[name] for name in embedded},
            "device_elfs": {name: item.summary() for name, item in embedded.items()},
        }
    report["side_comparisons"][side] = {
        name: objects[side]["object"][name].data == objects[side]["library"][name].data
        for name in (".aicore_binary", "__aicore_rel_binary")
    }
for name in (".aicore_binary", "__aicore_rel_binary"):
    report["parent_candidate_comparisons"][name] = compare(
        objects["parent"]["library"][name], objects["candidate"]["library"][name])
report["remote_source_difference"] = "".join(unified_diff(
    (ROOT / "Parent.asc").read_text().splitlines(keepends=True),
    (ROOT / "Candidate.asc").read_text().splitlines(keepends=True),
    fromfile="Parent.asc", tofile="Candidate.asc"))
report["build_evidence"] = {}
for side in ("parent", "candidate"):
    directory = ROOT / "build" / "CMakeFiles" / f"w4r09_v001_{side}.dir"
    report["build_evidence"][side] = {
        name: (directory / name).read_text() for name in ("flags.make", "link.txt")}
    report["build_evidence"][side]["compile_recipe"] = [
        line.strip() for line in (directory / "build.make").read_text().splitlines()
        if line.lstrip().startswith("/usr/local/Ascend/") and " -c -x asc " in line]
processes = subprocess.check_output(["ps", "-eo", "pid,args"], text=True).splitlines()
report["r09_processes"] = [line for line in processes
                           if str(ROOT) in line or "w4r09_v001_paired_runner" in line]
report["new_performance_revisions"] = 0
report["local_score"] = "NONE"
report["running_device_operation"] = "NONE"
print(json.dumps(report, indent=2, ensure_ascii=False))
