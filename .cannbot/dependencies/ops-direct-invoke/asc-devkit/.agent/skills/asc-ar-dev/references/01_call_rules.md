# Call-Hierarchy And Coding Rules

This file records the house rules that override repository example variation when needed.

## Mandatory Rules

- Ordinary host or general helper functions may use C++17.
- Ordinary host or general helper functions may launch `__global__` kernels only through `<<<>>>`.
- Ordinary host or general helper functions must not call `__aicore__` directly.
- `__global__` functions may call `__aicore__` functions only.
- `__global__` functions must not call another `__global__` function.
- `__aicore__` functions may call `__aicore__` functions.
- `__aicore__` functions must not call `__global__` functions.

## Preferred Structural Pattern

- Host side owns `aclInit`, device selection, stream creation, GM memory allocation, `aclrtMemcpy`, kernel launch, stream sync, and cleanup.
- Device entry is a thin `__global__` wrapper.
- Reusable device logic sits in `__aicore__ inline` functions or `__aicore__` class methods.
- Kernel state is usually wrapped in a small device-side class with `Init`, `Process`, and focused substeps such as `CopyIn`, `Compute`, and `CopyOut`.

## Legal Call Graph

```text
Host/general C++17 function
  -> kernel<<<...>>>()
    -> __global__ entry
      -> __aicore__ helper or __aicore__ class method
        -> __aicore__ helper
```

## Illegal Patterns

```text
Host/general function -> __aicore__ helper
__global__ kernel A -> __global__ kernel B
__aicore__ helper -> __global__ kernel
```

## Recommended Skeleton

```cpp
class KernelOp {
public:
    __aicore__ inline void Init(GM_ADDR x, GM_ADDR y) { /* ... */ }
    __aicore__ inline void Process()
    {
        CopyIn();
        Compute();
        CopyOut();
    }

private:
    __aicore__ inline void CopyIn() { /* ... */ }
    __aicore__ inline void Compute() { /* ... */ }
    __aicore__ inline void CopyOut() { /* ... */ }
};

__global__ __aicore__ void kernel_entry(GM_ADDR x, GM_ADDR y)
{
    KernelOp op;
    op.Init(x, y);
    op.Process();
}

int main()
{
    kernel_entry<<<numBlocks, nullptr, stream>>>(xDevice, yDevice);
}
```

## Style Notes

- Keep device code in AscendC idioms: `GM_ADDR`, `GlobalTensor`, `LocalTensor`, `TPipe`, `TQue`, `DataCopy`, explicit tiling structs.
- Keep host-only utilities in standard C++17 if that simplifies file IO, container handling, or verification.
- Preserve repo-specific qualifiers such as `__vector__` or `__cube__` when the target codebase already depends on them, but do not violate the mandatory call rules.
