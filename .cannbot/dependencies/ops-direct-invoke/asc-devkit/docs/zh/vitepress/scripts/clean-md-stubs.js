/**
* Copyright (c) 2026 Huawei Technologies Co., Ltd.
* This program is free software, you can redistribute it and/or modify it under the terms and conditions of
* CANN Open Software License Agreement Version 2.0 (the "License").
* Please refer to the License for details. You may not use this file except in compliance with the License.
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
* See LICENSE in the root of the software repository for the full text of the License.
*/


const { existsSync, unlinkSync, readFileSync } = require('node:fs')
const { resolve } = require('node:path')

const manifestFile = resolve(__dirname, '..', '.stubs-manifest.json')
if (!existsSync(manifestFile)) {
  console.log('No stubs manifest found, nothing to clean')
  process.exit(0)
}

const stubs = JSON.parse(readFileSync(manifestFile, 'utf-8'))
let removed = 0
for (const stub of stubs) {
  if (existsSync(stub)) {
    unlinkSync(stub)
    removed++
  }
  const headerFile = stub + '.header'
  if (existsSync(headerFile)) {
    unlinkSync(headerFile)
  }
}

unlinkSync(manifestFile)
console.log(`Cleaned up ${removed} .md stubs`)
