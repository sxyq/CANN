# N04 V021

STATUS=NO_CANDIDATE
PARENT=R31B-V011
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_PATH=NONE
CANDIDATE_SHA256=NONE

FUNCTION/STREAM=AddRmsNormBiasKernel<T>::Load; gammaGm_ and biasGm_ are read from GM into local tensors through DataCopyPad.
HINT_BEFORE=NONE_EXPOSED_BY_CHAMPION_LOAD
HINT_AFTER=NONE
API_BASIS=Load uses DataCopyExtParams and DataCopyPadExtParams; its source has no cache-hint argument or separate hint operation. No distinct supported stream/hint change can be established from the permitted source.
TARGET=ProcessWideLowPrecision, rowWidth > 8192, FP16/BF16. Exact workload shapes are unavailable in the permitted source.
VECTOR_ADDRESS_TILING_DMA_ORDER_EVENTS=UNCHANGED; no candidate was created.
