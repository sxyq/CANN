/**
* Copyright (c) 2026 Huawei Technologies Co., Ltd.
* This program is free software, you can redistribute it and/or modify it under the terms and conditions of
* CANN Open Software License Agreement Version 2.0 (the "License").
* Please refer to the License for details. You may not use this file except in compliance with the License.
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
* INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
* See LICENSE in the root of the software repository for the full text of the License.
*/

if (typeof document !== 'undefined') {
  document.addEventListener('click', (e) => {
    const btn = e.target.closest('.code-block .copy-btn')
    if (!btn) return
    const pre = btn.closest('.code-block').querySelector('pre')
    if (!pre) return
    navigator.clipboard.writeText(pre.textContent).then(() => {
      btn.classList.add('copied')
      setTimeout(() => btn.classList.remove('copied'), 2000)
    })
  })
}
