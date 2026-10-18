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

const searchInput = ref(null)
const query = ref('')
const results = ref([])
const loading = ref(false)
const showResults = ref(false)
const selectedIndex = ref(-1)

function getDirInfo(url) {
  const pathPart = url.split('#')[0].replace(/\.html?$/, '')
  const parts = pathPart.replace(/\/$/, '').split('/').filter(Boolean).map(decodeURIComponent)
  if (parts.length <= 1) return []
  const routePrefixes = ['api', 'guide']
  let start = routePrefixes.includes(parts[0]) ? 1 : 0
  return parts.slice(start).slice(-4)
}

let pagefindReady = false

async function ensurePagefind() {
  if (window.__pagefind__?.search) {
    pagefindReady = true
    return true
  }
  let elapsed = 0
  while (elapsed < 5000) {
    await new Promise(r => setTimeout(r, 100))
    elapsed += 100
    if (window.__pagefind__?.search) {
      pagefindReady = true
      return true
    }
  }
  return false
}

async function doSearch(q) {
  if (!q.trim()) {
    results.value = []
    return
  }
  if (!pagefindReady) {
    const ready = await ensurePagefind()
    if (!ready) return
  }
  loading.value = true
  try {
    const search = await window.__pagefind__.search(q.trim())
    const mapped = []
    for (const r of search.results.slice(0, 24)) {
      const data = await r.data()
      mapped.push({
        id: r.id,
        url: data.url,
        excerpt: data.excerpt,
        meta: data.meta || {},
        breadcrumbs: getDirInfo(data.url),
      })
    }
    results.value = mapped
    selectedIndex.value = -1
  } catch {
    results.value = []
  } finally {
    loading.value = false
  }
}

function debounce(fn, delay) {
  let timer
  return (...args) => {
    clearTimeout(timer)
    timer = setTimeout(() => fn(...args), delay)
  }
}

const debouncedSearch = debounce(doSearch, 200)

function onInput() {
  showResults.value = true
  debouncedSearch(query.value)
}

function onKeyDown(e) {
  if (!showResults.value || results.value.length === 0) return
  if (e.key === 'ArrowDown') {
    e.preventDefault()
    selectedIndex.value = (selectedIndex.value + 1) % results.value.length
  } else if (e.key === 'ArrowUp') {
    e.preventDefault()
    selectedIndex.value = selectedIndex.value <= 0 ? results.value.length - 1 : selectedIndex.value - 1
  } else if (e.key === 'Enter' && selectedIndex.value >= 0) {
    const r = results.value[selectedIndex.value]
    if (r) window.location.href = r.url
    showResults.value = false
    query.value = ''
  } else if (e.key === 'Escape') {
    showResults.value = false
    searchInput.value?.blur()
  }
}

function onClickOutside(e) {
  if (!e.target.closest('.pagefind-search')) {
    showResults.value = false
  }
}

onMounted(() => {
  document.addEventListener('click', onClickOutside)
})

onUnmounted(() => {
  document.removeEventListener('click', onClickOutside)
})
</script>

<template>
  <div class="pagefind-search">
    <input
      ref="searchInput"
      v-model="query"
      class="search-input"
      type="text"
      placeholder="搜索文档..."
      @input="onInput"
      @focus="onInput"
      @keydown="onKeyDown"
    />
    <div v-if="loading" class="search-loading">搜索中...</div>
    <ul v-if="showResults && results.length" class="search-results">
      <li
        v-for="(r, i) in results"
        :key="r.id"
        :class="{ selected: i === selectedIndex }"
      >
        <a :href="r.url" @click="showResults = false; query = ''">
          <div v-if="r.breadcrumbs.length" class="result-breadcrumbs">
            <span v-for="(seg, si) in r.breadcrumbs" :key="si" class="result-crumb">
              {{ seg }}
            </span>
          </div>
          <div class="result-title">{{ r.meta?.title || r.url }}</div>
          <div v-if="r.excerpt" class="result-excerpt" v-html="r.excerpt" />
        </a>
      </li>
    </ul>
    <div v-if="showResults && !loading && query.trim() && results.length === 0" class="search-empty">
      未找到结果
    </div>
  </div>
</template>

<style scoped>
.pagefind-search {
  position: relative;
  margin-right: 8px;
}

.search-input {
  width: 200px;
  padding: 6px 12px;
  border: 1px solid var(--vp-c-divider);
  border-radius: 8px;
  background: var(--vp-c-bg-alt);
  color: var(--vp-c-text-1);
  font-size: 14px;
  outline: none;
  transition: border-color 0.2s;
}

.search-input:focus {
  border-color: var(--vp-c-brand);
}

.search-input::placeholder {
  color: var(--vp-c-text-3);
}

.search-loading {
  position: absolute;
  top: 100%;
  left: 0;
  right: 0;
  padding: 12px;
  background: var(--vp-c-bg-soft);
  border: 1px solid var(--vp-c-divider);
  border-radius: 8px;
  margin-top: 4px;
  font-size: 13px;
  color: var(--vp-c-text-2);
  z-index: 100;
}

.search-results {
  position: absolute;
  top: 100%;
  left: 0;
  min-width: 480px;
  max-height: 480px;
  overflow-y: auto;
  background: var(--vp-c-bg-soft);
  border: 1px solid var(--vp-c-divider);
  border-radius: 8px;
  margin-top: 4px;
  padding: 8px 0;
  list-style: none;
  z-index: 100;
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.12);
}

.search-results li {
  padding: 0;
}

.search-results a {
  display: block;
  padding: 8px 16px;
  text-decoration: none;
  color: var(--vp-c-text-1);
  transition: background-color 0.15s;
}

.search-results a:hover,
.search-results li.selected a {
  background: var(--vp-c-bg-alt);
}

.result-breadcrumbs {
  display: flex;
  align-items: center;
  gap: 2px;
  margin-bottom: 2px;
  font-size: 12px;
  color: var(--vp-c-text-2);
  line-height: 1.4;
}

.result-crumb:not(:last-child)::after {
  content: ' \203A ';
  opacity: 0.5;
}

.result-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--vp-c-text-1);
  line-height: 1.4;
}

.result-excerpt {
  margin-top: 2px;
  font-size: 12px;
  color: var(--vp-c-text-2);
  line-height: 1.5;
  display: -webkit-box;
  -webkit-line-clamp: 1;
  -webkit-box-orient: vertical;
  overflow: hidden;
}

.result-excerpt :deep(mark) {
  background: var(--vp-c-brand-light);
  color: var(--vp-c-brand-dark);
  border-radius: 2px;
  padding: 0 2px;
}

.search-empty {
  position: absolute;
  top: 100%;
  left: 0;
  right: 0;
  padding: 12px;
  background: var(--vp-c-bg-soft);
  border: 1px solid var(--vp-c-divider);
  border-radius: 8px;
  margin-top: 4px;
  font-size: 13px;
  color: var(--vp-c-text-2);
  z-index: 100;
}
</style>
