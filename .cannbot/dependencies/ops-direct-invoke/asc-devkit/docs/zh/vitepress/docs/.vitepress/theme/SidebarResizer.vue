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
import { useSidebar } from 'vitepress/theme'

const MIN_WIDTH = 240
const MAX_WIDTH = 600
const DEFAULT_WIDTH = 320
const STORAGE_KEY = 'vp-sidebar-width'

const { isSidebarEnabled } = useSidebar()

const dragging = ref(false)
const hovering = ref(false)

let startX = 0
let startWidth = DEFAULT_WIDTH
let currentWidth = DEFAULT_WIDTH

function getCurrentWidth() {
  const raw = getComputedStyle(document.documentElement)
    .getPropertyValue('--vp-sidebar-width')
    .trim()
  const px = Number(String(raw).replace(/px$/, ''))
  return Number.isFinite(px) ? px : DEFAULT_WIDTH
}

function applyWidth(px) {
  currentWidth = px
  document.documentElement.style.setProperty('--vp-sidebar-width', px + 'px')
}

function saveWidth() {
  const w = Math.min(MAX_WIDTH, Math.max(MIN_WIDTH, currentWidth))
  localStorage.setItem(STORAGE_KEY, String(w))
}

function readStoredWidth() {
  const raw = localStorage.getItem(STORAGE_KEY)
  if (raw === null || raw === '') return DEFAULT_WIDTH
  const v = Number(raw)
  if (Number.isFinite(v)) return Math.min(MAX_WIDTH, Math.max(MIN_WIDTH, v))
  return DEFAULT_WIDTH
}

function endDrag(save = true) {
  if (!dragging.value) return
  dragging.value = false
  document.body.style.cursor = ''
  document.body.style.userSelect = ''
  if (save) saveWidth()
}

function onPointerDown(e) {
  if (!isSidebarEnabled.value) return
  e.preventDefault()
  dragging.value = true
  startX = e.clientX
  startWidth = getCurrentWidth()
  currentWidth = startWidth
  document.body.style.cursor = 'col-resize'
  document.body.style.userSelect = 'none'
  e.currentTarget.setPointerCapture(e.pointerId)
}

function onPointerMove(e) {
  if (!dragging.value) return
  const delta = e.clientX - startX
  const next = Math.min(MAX_WIDTH, Math.max(MIN_WIDTH, startWidth + delta))
  applyWidth(next)
}

function onPointerUp() {
  endDrag(true)
}

function onPointerCancel() {
  endDrag(false)
}

function onDoubleClick() {
  if (!isSidebarEnabled.value) return
  applyWidth(DEFAULT_WIDTH)
  saveWidth()
}

function onVisibilityChange() {
  if (document.hidden) endDrag(false)
}

function onBlur() {
  endDrag(false)
}

onMounted(() => {
  applyWidth(readStoredWidth())
  window.addEventListener('visibilitychange', onVisibilityChange)
  window.addEventListener('blur', onBlur)
})

onUnmounted(() => {
  endDrag(false)
  window.removeEventListener('visibilitychange', onVisibilityChange)
  window.removeEventListener('blur', onBlur)
})
</script>

<template>
  <div
    v-if="isSidebarEnabled"
    class="sidebar-resizer"
    :class="{ active: dragging, hover: hovering }"
    @pointerdown="onPointerDown"
    @pointermove="onPointerMove"
    @pointerup="onPointerUp"
    @pointercancel="onPointerCancel"
    @dblclick="onDoubleClick"
    @mouseenter="hovering = true"
    @mouseleave="hovering = false"
  ></div>
</template>

<style scoped>
.sidebar-resizer {
  position: fixed;
  top: var(--vp-nav-height);
  bottom: 0;
  width: 6px;
  margin-left: -3px;
  z-index: 40;
  cursor: col-resize;
  touch-action: none;
}

@media (min-width: 960px) {
  .sidebar-resizer {
    left: var(--vp-sidebar-width);
  }
}

@media (min-width: 1440px) {
  .sidebar-resizer {
    left: calc((100vw - var(--vp-layout-max-width)) / 2 + var(--vp-sidebar-width));
  }
}

.sidebar-resizer::after {
  content: '';
  position: absolute;
  top: 0;
  bottom: 0;
  left: 50%;
  width: 2px;
  transform: translateX(-50%);
  background-color: transparent;
  transition: background-color 0.2s;
}

.sidebar-resizer.hover::after,
.sidebar-resizer.active::after {
  background-color: var(--vp-c-brand-1);
}

@media (max-width: 959px) {
  .sidebar-resizer {
    display: none;
  }
}
</style>
