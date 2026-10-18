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
import { ref, onMounted, onUnmounted } from 'vue'

const visible = ref(false)

function onScroll() {
  visible.value = window.scrollY > 400
}

function scrollToTop() {
  window.scrollTo({ top: 0, behavior: 'smooth' })
}

onMounted(() => window.addEventListener('scroll', onScroll))
onUnmounted(() => window.removeEventListener('scroll', onScroll))
</script>

<template>
  <button
    v-show="visible"
    class="back-to-top"
    @click="scrollToTop"
    title="返回顶部"
  >
    <svg width="20" height="20" viewBox="0 0 20 20" fill="currentColor">
      <path d="M10 3.5L3.5 10h4v6h5v-6h4z"/>
    </svg>
  </button>
</template>

<style scoped>
.back-to-top {
  position: fixed;
  bottom: 40px;
  right: 40px;
  width: 44px;
  height: 44px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 50%;
  background: var(--vp-c-bg-soft);
  color: var(--vp-c-text-2);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 100;
  transition: all 0.3s;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

.back-to-top:hover {
  border-color: var(--vp-c-brand-1);
  color: var(--vp-c-brand-1);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
}
</style>
