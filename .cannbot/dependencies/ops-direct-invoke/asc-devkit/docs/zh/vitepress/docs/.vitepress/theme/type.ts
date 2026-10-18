/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

export interface PagefindResult {
  url: string
  content: string
  word_count: number
  filters: Filters
  meta: Meta
  anchors: Anchor[]
  weighted_locations: WeightedLocation[]
  locations: number[]
  raw_content: string
  raw_url: string
  excerpt: string
  sub_results: SubResult[]
}

interface SubResult {
  title: string
  url: string
  anchor: Anchor
  weighted_locations: WeightedLocation[]
  locations: number[]
  excerpt: string
}

interface WeightedLocation {
  weight: number
  balanced_score: number
  location: number
}

interface Anchor {
  element: string
  id: string
  text: string
  location: number
}

interface Meta {
  image_alt: string
  title: string
  image: string
  base64: string
}

interface Filters {
}

export interface PagefindOption {
  excludeSelector?: string[]
  forceLanguage?: string
  indexingCommand?: string
}

export interface SearchItem {
  route: string
  meta: Record<string, any>
  result: PagefindResult
}
export interface SearchConfig {
  btnPlaceholder?: string
  placeholder?: string
  emptyText?: string
  heading?: string
  toSelect?: string
  toNavigate?: string
  toClose?: string
  searchBy?: string
  langReload?: boolean
  customSearchQuery?: (input: string) => string
  resultOptimization?: boolean
  filter?: (searchItem: SearchItem, idx: number, array: SearchItem[]) => boolean
  sort?: (a: SearchItem, b: SearchItem) => number
  showDate?: boolean | ((date: number, lang: string) => string)
  timeZone?: number
  locales?: Record<string, Omit<SearchConfig, 'locales'>>
  ignorePublish?: boolean
  manual?: boolean
  delay?: number
  pageResultCount?: number
  mpaDefaultUI?: boolean
}

export type PagefindConfig = PagefindOption & SearchConfig
