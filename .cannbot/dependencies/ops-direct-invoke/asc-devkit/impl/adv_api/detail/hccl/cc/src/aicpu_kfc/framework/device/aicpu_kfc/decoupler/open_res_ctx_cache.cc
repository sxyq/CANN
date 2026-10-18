/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "open_res_ctx_cache.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "log.h"

namespace {
constexpr size_t OPEN_RES_CTX_CACHE_CAPACITY = 256U;

struct CacheKey {
    const void* hcclComm{nullptr};
    std::string commName;
    std::string algTag;

    bool operator<(const CacheKey& other) const
    {
        if (hcclComm != other.hcclComm) {
            return std::less<const void*>{}(hcclComm, other.hcclComm);
        }
        if (commName != other.commName) {
            return commName < other.commName;
        }
        return algTag < other.algTag;
    }
};

struct CacheEntry {
    mc2_open::OpenResCtxHolder holder;
    const void* sourceResCtx{nullptr};
    u64 ctxSize{0U};
    uint64_t lastAccess{0U};
    uint64_t publishSerial{0U};
    bool publishLogged{false};
    bool firstHitLogged{false};
};

struct CacheStats {
    std::atomic<uint64_t> hits{0U};
    std::atomic<uint64_t> misses{0U};
    std::atomic<uint64_t> stale{0U};
    std::atomic<uint64_t> bypasses{0U};
    std::atomic<uint64_t> evictions{0U};
    std::atomic<uint64_t> superseded{0U};
    std::atomic<uint64_t> buildFailures{0U};
};

std::mutex g_cacheMutex;
std::map<CacheKey, CacheEntry> g_cache;
std::set<CacheKey> g_missLoggedKeys;
uint64_t g_accessSerial{0U};
uint64_t g_publishSerial{0U};
CacheStats g_cacheStats;

template <size_t N>
bool GetBoundedString(const char (&buffer)[N], std::string& value)
{
    const char* end = std::find(buffer, buffer + N, '\0');
    if (end == buffer || end == buffer + N) {
        return false;
    }
    value.assign(buffer, end);
    return true;
}

bool IsCacheableOp(HcclCMDType opType)
{
    switch (opType) {
        case HCCL_CMD_ALLGATHER:
        case HCCL_CMD_ALLREDUCE:
        case HCCL_CMD_REDUCE_SCATTER:
        case HCCL_CMD_ALLTOALL:
        case HCCL_CMD_ALLTOALLV:
            return true;
        default:
            return false;
    }
}

HcclResult ValidateParam(const mc2_ops_hccl::OpParam& param)
{
    if (param.hcclComm == nullptr || param.resCtx == nullptr || param.ctxSize == 0U ||
        param.ctxSize > std::numeric_limits<size_t>::max()) {
        HCCL_ERROR(
            "Invalid open resource ctx parameter, hcclComm[%p], resCtx[%p], ctxSize[%llu].", param.hcclComm,
            param.resCtx, static_cast<unsigned long long>(param.ctxSize));
        return HCCL_E_PARA;
    }

    return HCCL_SUCCESS;
}

HcclResult BuildCacheKey(const mc2_ops_hccl::OpParam& param, CacheKey& key)
{
    if (!GetBoundedString(param.commName, key.commName) || !GetBoundedString(param.algTag, key.algTag)) {
        HCCL_ERROR("Invalid open resource ctx cache key, commName or algTag is empty or unterminated.");
        return HCCL_E_PARA;
    }
    key.hcclComm = param.hcclComm;
    return HCCL_SUCCESS;
}

HcclResult BuildHolder(const mc2_ops_hccl::OpParam& param, mc2_open::OpenResCtxHolder& holder)
{
    const auto* ctx = static_cast<const char*>(param.resCtx);
    const size_t ctxSize = static_cast<size_t>(param.ctxSize);
    std::vector<char> sequence(ctx, ctx + ctxSize);
    auto mutableHolder = std::make_shared<mc2_ops_hccl::AlgResourceCtxSerializable>();
    mutableHolder->DeSerialize(sequence);

    if (mutableHolder->commInfoPtr == nullptr || mutableHolder->commInfoPtr != param.hcclComm) {
        HCCL_ERROR(
            "Invalid open resource ctx comm token, serializedComm[%p], currentComm[%p].", mutableHolder->commInfoPtr,
            param.hcclComm);
        return HCCL_E_PARA;
    }
    if (mutableHolder->threads.empty() || mutableHolder->threads[0] == 0U) {
        HCCL_ERROR("Invalid open resource ctx threads, threadNum[%zu].", mutableHolder->threads.size());
        return HCCL_E_PARA;
    }
    if (mutableHolder->topoInfo.userRankSize == 0U ||
        mutableHolder->topoInfo.userRank >= mutableHolder->topoInfo.userRankSize) {
        HCCL_ERROR(
            "Invalid open resource ctx topology, userRank[%u], userRankSize[%u].", mutableHolder->topoInfo.userRank,
            mutableHolder->topoInfo.userRankSize);
        return HCCL_E_PARA;
    }

    holder = std::move(mutableHolder);
    return HCCL_SUCCESS;
}

bool IsEntryForParam(const CacheEntry& entry, const mc2_ops_hccl::OpParam& param)
{
    return entry.holder != nullptr && entry.holder->commInfoPtr == param.hcclComm &&
           entry.sourceResCtx == param.resCtx && entry.ctxSize == param.ctxSize;
}

bool IsReusable(const CacheEntry& entry, const mc2_ops_hccl::OpParam& param)
{
    return param.cacheValid && IsEntryForParam(entry, param);
}

void LogCacheEvent(const char* event, uint64_t opParamKey, const mc2_ops_hccl::OpParam& param)
{
    HCCL_DEBUG(
        "[MC2_RES_CTX_CACHE][%s] opParamKey[%#llx], hcclComm[%p], commName[%s], algTag[%s], opType[%u], "
        "sourceResCtx[%p], ctxSize[%llu], cacheValid[%d].",
        event, static_cast<unsigned long long>(opParamKey), param.hcclComm, param.commName, param.algTag,
        static_cast<u32>(param.opType), param.resCtx, static_cast<unsigned long long>(param.ctxSize),
        static_cast<int>(param.cacheValid));
}

void EvictOldestIfNeeded()
{
    if (g_cache.size() < OPEN_RES_CTX_CACHE_CAPACITY) {
        return;
    }
    auto oldest = std::min_element(g_cache.begin(), g_cache.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second.lastAccess < rhs.second.lastAccess;
    });
    if (oldest != g_cache.end()) {
        g_missLoggedKeys.erase(oldest->first);
        g_cache.erase(oldest);
        g_cacheStats.evictions.fetch_add(1U, std::memory_order_relaxed);
    }
}

void MarkDeviceCacheValid(uint64_t opParamKey)
{
    auto* deviceParam = reinterpret_cast<mc2_ops_hccl::OpParam*>(opParamKey);
    __atomic_store_n(&deviceParam->cacheValid, true, __ATOMIC_RELEASE);
}
} // namespace

namespace mc2_open {
HcclResult AcquireOpenResCtx(uint64_t opParamKey, const mc2_ops_hccl::OpParam& param, OpenResCtxHolder& holder)
{
    holder.reset();
    if (opParamKey == 0U) {
        HCCL_ERROR(
            "Invalid opParamKey %#llx for open resource ctx cache.", static_cast<unsigned long long>(opParamKey));
        return HCCL_E_PARA;
    }

    HcclResult ret = ValidateParam(param);
    if (ret != HCCL_SUCCESS) {
        return ret;
    }

    const bool cacheable = IsCacheableOp(param.opType);
    CacheKey key;
    uint64_t publishSerial = 0U;
    bool cacheReadyForParam = false;
    bool logMiss = false;
    bool logPublish = false;
    bool logFirstHit = false;
    if (cacheable) {
        ret = BuildCacheKey(param, key);
        if (ret != HCCL_SUCCESS) {
            return ret;
        }
        std::lock_guard<std::mutex> lock(g_cacheMutex);
        auto iter = g_cache.find(key);
        if (iter != g_cache.end() && IsReusable(iter->second, param)) {
            iter->second.lastAccess = ++g_accessSerial;
            holder = iter->second.holder;
            cacheReadyForParam = true;
            g_cacheStats.hits.fetch_add(1U, std::memory_order_relaxed);
            if (!iter->second.firstHitLogged) {
                iter->second.firstHitLogged = true;
                logFirstHit = true;
            }
        } else if (iter == g_cache.end()) {
            g_cacheStats.misses.fetch_add(1U, std::memory_order_relaxed);
            publishSerial = ++g_publishSerial;
            logMiss = g_missLoggedKeys.insert(key).second;
        } else {
            g_cacheStats.stale.fetch_add(1U, std::memory_order_relaxed);
            publishSerial = ++g_publishSerial;
        }
    } else {
        g_cacheStats.bypasses.fetch_add(1U, std::memory_order_relaxed);
    }

    if (logMiss) {
        LogCacheEvent("MISS", opParamKey, param);
    }
    if (logFirstHit) {
        LogCacheEvent("FIRST_HIT", opParamKey, param);
    }

    if (holder == nullptr) {
        OpenResCtxHolder newHolder;
        ret = BuildHolder(param, newHolder);
        if (ret != HCCL_SUCCESS) {
            g_cacheStats.buildFailures.fetch_add(1U, std::memory_order_relaxed);
            return ret;
        }

        if (cacheable) {
            std::lock_guard<std::mutex> lock(g_cacheMutex);
            auto iter = g_cache.find(key);
            // A miss is built without holding the cache mutex. Keep the entry produced by the latest request when
            // concurrent builders finish out of order, while their local holders remain valid for the current run.
            const bool shouldPublish = iter == g_cache.end() || publishSerial >= iter->second.publishSerial;
            if (iter == g_cache.end()) {
                EvictOldestIfNeeded();
                CacheEntry entry;
                entry.holder = newHolder;
                entry.sourceResCtx = param.resCtx;
                entry.ctxSize = param.ctxSize;
                entry.lastAccess = ++g_accessSerial;
                entry.publishSerial = publishSerial;
                entry.publishLogged = true;
                g_cache.emplace(std::move(key), std::move(entry));
                cacheReadyForParam = true;
                logPublish = true;
            } else if (shouldPublish) {
                const bool sourceChanged = !IsEntryForParam(iter->second, param);
                iter->second.holder = newHolder;
                iter->second.sourceResCtx = param.resCtx;
                iter->second.ctxSize = param.ctxSize;
                iter->second.lastAccess = ++g_accessSerial;
                iter->second.publishSerial = publishSerial;
                if (sourceChanged) {
                    iter->second.publishLogged = false;
                    iter->second.firstHitLogged = false;
                }
                if (!iter->second.publishLogged) {
                    iter->second.publishLogged = true;
                    logPublish = true;
                }
                cacheReadyForParam = true;
            } else {
                cacheReadyForParam = IsEntryForParam(iter->second, param);
                g_cacheStats.superseded.fetch_add(1U, std::memory_order_relaxed);
            }
        }
        holder = std::move(newHolder);
    }

    if (logPublish) {
        LogCacheEvent("PUBLISH", opParamKey, param);
    }

    // The open path keeps one device OpParam across launches. Promote the first successful cache publication so
    // subsequent server instances can take the same cache-hit path as native HCCL without another Host rebuild.
    if (cacheReadyForParam && !param.cacheValid) {
        MarkDeviceCacheValid(opParamKey);
    }
    return HCCL_SUCCESS;
}
} // namespace mc2_open
