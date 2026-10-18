<!--
  Copyright (c) 2026 Huawei Technologies Co., Ltd.
  This program is free software, you can redistribute it and/or modify it under the terms and conditions of
  CANN Open Software License Agreement Version 2.0 (the "License").
  Please refer to the License for details. You may not use this file except in compliance with the License.
  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
  See LICENSE in the root of the software repository for the full text of the License.
-->

<script setup>
import { computed } from 'vue'
import { useData } from 'vitepress'

const { page } = useData()
const editUrl = computed(() => {
  const path = page.value.relativePath || ''
  if (path.startsWith('en/')) {
    return `https://gitcode.com/cann/asc-devkit/blob/master/docs/en/${path.slice(3)}`
  }
  return `https://gitcode.com/cann/asc-devkit/blob/master/docs/zh/${path}`
})
</script>

<template>
  <div v-if="page.relativePath" class="edit-on-gitcode">
    <a class="edit-link" :href="editUrl" target="_blank" rel="noopener noreferrer">
      <svg class="edit-icon" viewBox="0 0 24 24" width="16" height="16" fill="currentColor">
        <path d="M18 13v6a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h6M15 3h6v6M10 14 20.2 3.8"/>
      </svg>
      在 Gitcode 上打开此页
    </a>
  </div>
</template>

<style scoped>
.edit-on-gitcode {
  margin-bottom: 24px;
}

.edit-link {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  font-size: 13px;
  font-weight: 500;
  color: var(--vp-c-text-2);
  text-decoration: none;
  transition: color 0.25s;
}

.edit-link:hover {
  color: var(--vp-c-brand-1);
}

.edit-icon {
  flex-shrink: 0;
  opacity: 0.7;
}

.edit-link:hover .edit-icon {
  opacity: 1;
}
</style>
