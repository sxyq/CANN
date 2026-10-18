/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

const { execSync } = require('node:child_process')
const { existsSync } = require('node:fs')
const { resolve } = require('node:path')

const root = resolve(__dirname, '..')

function hasPythonModule(name) {
  try {
    execSync(`python3 -c "import ${name}"`, { stdio: 'pipe', timeout: 5000 })
    return true
  } catch {
    try {
      execSync(`python -c "import ${name}"`, { stdio: 'pipe', timeout: 5000 })
      return true
    } catch {
      return false
    }
  }
}

function findPip() {
  try { execSync('pip3 --version', { stdio: 'pipe', timeout: 5000 }); return 'pip3' } catch { /* ignore */ }
  try { execSync('pip --version', { stdio: 'pipe', timeout: 5000 }); return 'pip' } catch { /* ignore */ }
  return null
}

// ── Node dependencies ──────────────────────────────────────────
if (!existsSync(resolve(root, 'node_modules'))) {
  console.log('[install-deps] Installing Node dependencies...')
  execSync('npm install --legacy-peer-deps', { cwd: root, stdio: 'inherit' })
  console.log('[install-deps] Node dependencies installed')
} else {
  console.log('[install-deps] Node dependencies: OK')
}

// ── Python dependencies ────────────────────────────────────────
const missing = []
if (!hasPythonModule('cmarkgfm')) missing.push('cmarkgfm')
if (!hasPythonModule('pygments')) missing.push('pygments')

if (missing.length > 0) {
  const pip = findPip()
  if (!pip) {
    console.error('[install-deps] ERROR: pip not found. Install Python dependencies manually:')
    console.error('  pip install -r requirements.txt')
    process.exit(1)
  }
  console.log(`[install-deps] Installing Python dependencies: ${missing.join(', ')}...`)
  execSync(`"${pip}" install -r requirements.txt`, { cwd: root, stdio: 'inherit' })
  console.log('[install-deps] Python dependencies installed')
} else {
  console.log('[install-deps] Python dependencies: OK')
}
