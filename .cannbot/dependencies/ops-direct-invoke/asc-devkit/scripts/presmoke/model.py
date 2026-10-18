#!/usr/bin/python3
# coding=utf-8

# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional


@dataclass(frozen=True)
class Command:
    raw: str
    kind: str
    env: Dict[str, str] = field(default_factory=dict)


@dataclass
class Suggestion:
    example: str
    category: str
    severity: str
    message: str
    hint: str = ""


@dataclass
class ExampleSpec:
    path: Path
    rel_path: str
    commands: List[Command]
    archs: List[str]
    modes: List[str]
    source: str
    suggestions: List[Suggestion] = field(default_factory=list)


@dataclass
class Cell:
    example: ExampleSpec
    arch: str
    mode: str
    commands: List[Command]
    build_dir: Path

    @property
    def key(self) -> str:
        return f"{self.example.rel_path}|{self.arch}|{self.mode}"


@dataclass
class StepResult:
    command: str
    kind: str
    rc: Optional[int]
    duration_s: float
    wait_s: float = 0.0
    npu_slot: bool = False
    timeout: bool = False


@dataclass
class RunResult:
    example: str
    arch: str
    mode: str
    status: str
    reason: str = ""
    failing_step: Optional[str] = None
    rc: Optional[int] = None
    duration_s: float = 0.0
    log_file: str = ""
    stage_log_files: List[str] = field(default_factory=list)
    source: str = ""
    steps: List[StepResult] = field(default_factory=list)


@dataclass
class NpuStats:
    slots: int
    queue_model: str
    idle_s: float = 0.0
    busy_s: float = 0.0
    max_slot_idle_s: float = 0.0
    max_slot_busy_s: float = 0.0

    @property
    def utilization(self) -> float:
        total = self.idle_s + self.busy_s
        return self.busy_s / total if total else 0.0


@dataclass
class RunCellsResult:
    results: List[RunResult]
    npu_stats: NpuStats


@dataclass
class RunReport:
    host_arch: str
    modes: List[str]
    started_at: str
    finished_at: str
    results: List[RunResult]
    suggestions: List[Suggestion]
    npu_stats: Optional[NpuStats] = None
    parallel_config: Dict[str, int] = field(default_factory=dict)
