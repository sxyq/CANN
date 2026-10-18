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

import os
from typing import Mapping


WERROR_CMAKE_ARGS = ("-DCMAKE_ASC_FLAGS=-Werror",)


def truthy(value: str | None) -> bool:
    return value in {"1", "true", "TRUE", "yes", "YES", "on", "ON"}


def enabled_from_env(env: Mapping[str, str] | None = None) -> bool:
    values = os.environ if env is None else env
    return truthy(values.get("PRESMOKE_WERROR"))


def cmake_args(enabled: bool) -> list[str]:
    return list(WERROR_CMAKE_ARGS) if enabled else []


def command_env(env: Mapping[str, str], enabled: bool) -> dict[str, str]:
    result = dict(env)
    if not enabled:
        return result
    result["CFLAGS"] = append_werror(result.get("CFLAGS", ""))
    result["CXXFLAGS"] = append_werror(result.get("CXXFLAGS", ""))
    return result


def append_werror(value: str) -> str:
    flags = value.split()
    if "-Werror" not in flags:
        flags.append("-Werror")
    return " ".join(flags)
