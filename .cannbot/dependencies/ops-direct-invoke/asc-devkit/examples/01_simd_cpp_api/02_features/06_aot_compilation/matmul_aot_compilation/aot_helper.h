/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/* !
 * \file aot_helper.h
 * \brief AOT 框架核心实现（Holder、Registry、Dispatcher）
 */

#pragma once

#include <type_traits>
#include <cstring>

namespace aot {

// ========== 1.1 AOT Holder 基类（C++17 兼容） ==========

// 使用指针指向全局 constexpr 变量
template <typename T, const uint8_t* ValuePtr>
struct AOTHolder {
    using value_type = T;
    static constexpr const uint8_t* value_ptr = ValuePtr;
    static constexpr const T value = convert_from_bytes<T>(ValuePtr);
};

// ========== 1.3 运行时 Holder（保持兼容） ==========

template <typename T>
struct RuntimeHolder {
    static constexpr T value = {0};
};

// ========== 1.4 AOT 注册表 ==========

// 注册表：只保存类型 pack，不再用 std::tuple
template <typename... Holders>
struct AOTRegistry {
    static constexpr size_t size = sizeof...(Holders);
};

// ========== 1.5 分发器 ==========

// 主模板只声明，靠下方偏特化把 Holders... 从 Registry 里解出来
template <typename T, typename Registry>
struct AOTDispatcher;

// 偏特化：直接对 Holders... pack 做折叠展开，避免 tuple_element_t 的 O(N) 递归和
// 编译期 O(N²) 实例化代价，同时让模板深度从 O(N) 降到 O(1)。
template <typename T, typename... Holders>
struct AOTDispatcher<T, AOTRegistry<Holders...>> {
    // 主入口：接收运行时值，自动分发
    template <typename Func>
    static void dispatch(const uint8_t* value, Func&& kernel_func)
    {
        // C++17 折叠表达式：(... || try_one<H_i>())，|| 短路保证首次命中即停
        bool matched = (try_one<Holders>(value, kernel_func) || ...);
        if (!matched) {
            // 所有 Holder 都不匹配：调用运行时版本
            kernel_func(RuntimeHolder<T>{}, *reinterpret_cast<const T*>(value));
        }
    }

private:
    // try_one 必须强制内联：否则 N 个 Holder 会留下 N 份函数体，导致符号表和代码体积线性膨胀
    template <typename Holder, typename Func>
    __attribute__((always_inline)) static inline bool try_one(const uint8_t* value, Func& kernel_func)
    {
        if (std::memcmp(value, Holder::value_ptr, sizeof(T)) == 0) {
            kernel_func(Holder{}, *reinterpret_cast<const T*>(value));
            return true;
        }
        return false;
    }
};

template <typename T, typename HT>
inline __aicore__ const T& GetHolderDataRef(const T& rt_data)
{
    if constexpr (std::is_same_v<HT, RuntimeHolder<T>>) {
        return rt_data;
    } else {
        return HT::value;
    }
}

} // end of namespace aot
