/**
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/*!
 * \file op_aclnn_generator.h
 * \brief
 */

#ifndef ACLNN_GENERATOR_H
#define ACLNN_GENERATOR_H

#include <fstream>
#include <cstring>
#include <map>
#include <unordered_set>
#include "op_generator.h"
#include "op_generator_factory.h"
#include "register/op_def.h"
#include "register/op_def_factory.h"
#include "op_build_error_codes.h"

namespace ops {
using namespace std;

constexpr const int32_t OP_ACLNN_ATTR_TYPE_STR = 0;
constexpr const int32_t OP_ACLNN_ATTR_TYPE_BOOL = 1;
constexpr const int32_t OP_ACLNN_ATTR_TYPE_FLOAT = 2;
constexpr const int32_t OP_ACLNN_ATTR_TYPE_INT = 3;
constexpr const int32_t OP_ACLNN_ATTR_TYPE_LISTBOOL = 4;
constexpr const int32_t OP_ACLNN_ATTR_TYPE_LISTFLOAT = 5;
constexpr const int32_t OP_ACLNN_ATTR_TYPE_LISTINT = 6;

const std::map<std::string, int32_t> ACLNN_OP_ATTR_TYPE_MAP = {
    {"str", OP_ACLNN_ATTR_TYPE_STR},           {"bool", OP_ACLNN_ATTR_TYPE_BOOL},
    {"float", OP_ACLNN_ATTR_TYPE_FLOAT},       {"int", OP_ACLNN_ATTR_TYPE_INT},
    {"listBool", OP_ACLNN_ATTR_TYPE_LISTBOOL}, {"listFloat", OP_ACLNN_ATTR_TYPE_LISTFLOAT},
    {"listInt", OP_ACLNN_ATTR_TYPE_LISTINT},
};

struct OpDefName {
    std::vector<std::string> originInputName; // op_host配置的输入名称，scalar输入跟随指定参数做类型转换
    std::vector<std::string> inputsName;      // 转换后的输入名称
    std::vector<std::string> outputsName;      // 转换后的输出名称
    std::vector<std::string> attrsName;        // 转换后的属性名称
    std::vector<std::string> defaultAttrsName; // 配置版本号时默认属性名称
    std::string prefixName;                    // 前缀名
    std::string maxVersionName;                // 最大版本号前缀名
    std::string opName;                        // 算子名
    std::string decName;                       // include的头文件名称
    std::string macroNmae;                     // 宏名称
    std::string fileName;                      // 头文件名称
    std::string maxDecName;                    // 最大版本号宏名称
    bool hasOutputShapeDepend;
};

// 辅助结构：存储每个输入在不同 Soc 上的 Contiguous 配置
enum class ContiguousType : int32_t {
    Default = 0,          // 默认(无特殊配置)
    IgnoreContiguous = 1, // 忽略连续性检查
    AutoContiguous = 2    // 自动转换为连续tensor
};

struct InputContiguousConfig {
    std::string inputName;
    int32_t inputIndex = -1;
    std::map<std::string, ContiguousType> socContiguousType;
};

struct OpDefIoDesc {
    OpParamDef input;
    std::string inputName;
    size_t index = 0U;
    std::string opType;
};

struct OpCodeGenConfig {
    bool valueDependApi = false;
    bool needSocCheck = false;
    bool useBaseConfig = false;
    std::string indent;
};

class AclnnOpGenerator : public Generator {
public:
    explicit AclnnOpGenerator(std::vector<std::string>& ops);
    void AclnnSetErrorMessage(std::string& str, const std::string opType) const;
    void AclnnOpGenHeaderFileDel(std::string& name, std::ofstream& outfile, bool isStart) const;
    std::ofstream AclnnOpGenHeaderFileStart(std::string& fileName, std::string& macroNmae, uint32_t version) const;
    bool AclnnOpGenFunProtoValueDependParam(
        OpDef& opDef, const OpDefName& opdefName, size_t paramIndex, ofstream& outfile,
        const std::string& opType) const;
    std::vector<std::vector<ge::DataType>> AclnnGetInputAndOutputDataTypeList(
        std::vector<OpParamDef>& inputs, std::vector<OpParamDef>& outputs) const;
    std::vector<std::string> AclnnGetInputAndOutputNames(
        const std::vector<OpParamDef>& inputs, const std::vector<OpParamDef>& outputs) const;
    std::vector<size_t> AclnnGetValueDependIntTypeIndex(std::vector<OpParamDef>& inputs) const;
    std::string AclnnBuildValueDependDataTypeErrorMessage(
        const std::vector<std::string>& paramOriginNames, const std::vector<ge::DataType>& originDataTypes,
        const std::vector<ge::DataType>& requiredDataTypes, int valueDependIndex, const string& opType) const;
    bool AclnnCheckForInt64CombinationWithValueDepend(OpDef& opDef, size_t paramIndex, const std::string& opType) const;
    bool AclnnIsValueDependDataTypeSupport(std::vector<OpParamDef>& inputs, const std::string& opType) const;
    bool AclnnIsRefParam(const std::string& inputName) const;
    void AclnnOpGenFunProtoParam(
        const OpParamDef& param, const std::string& paramName, int32_t paramType, bool hasOutputShapeDepend,
        ofstream& outfile) const;
    void AclnnOpGenFunProtoInputParams(
        OpDef& opDef, OpDefName& opdefName, std::ofstream& outfile, const uint32_t version,
        const bool valDependApi) const;
    void AclnnOpGenFunProtoOutputParams(
        OpDef& opDef, OpDefName& opdefName, std::ofstream& outfile, const uint32_t version,
        const bool valDependApi) const;
    void AclnnOpGenFunProtoAttrParamsImpl(
        OpAttrDef& attr, std::ofstream& outfile, std::string& name, const std::string opType) const;
    void AclnnOpGenFunProtoAttrParams(
        OpDef& opDef, std::vector<std::string>& paramNames, std::ofstream& outfile, uint32_t version) const;
    void AclnnOpGenValueDependInput(
        OpParamDef& input, std::string& name, size_t index, std::ofstream& outfile,
        const std::string& indent = "") const;
    bool AclOpGenScalarInputWithIndent(
        OpDefIoDesc& opDefIoDesc, OpDefName& opdefName, std::ofstream& outfile, std::string funcName,
        const std::string& indent) const;
    void AclnnOpGenCodeAddInputTensors(
        OpDef& opDef, OpDefName& opdefName, std::ofstream& outfile, bool valueDependApi, bool needSocCheck) const;
    void AclnnOpGenCodeAddOutputShapeDependTensors(
        std::vector<OpParamDef>& outputs, std::vector<std::string>& name, std::ofstream& outfile) const;
    void AclnnOpGenCodeAddOutputTensors(
        std::vector<OpParamDef>& outputs, std::vector<std::string>& name, bool hasOutputShapeDepend,
        std::ofstream& outfile) const;
    void AclnnoOpGenCodeAttrValue(OpAttrDef& attr, size_t* len, std::ofstream& outfile) const;
    void AclnnOpGenCodeOptionalStrAttr(OpAttrDef& attr, std::string& name, size_t index, std::ofstream& outfile) const;
    void AclnnOpGenCodeOptionalBoolAttr(OpAttrDef& attr, std::string& name, size_t index, std::ofstream& outfile) const;
    void AclnnOpGenCodeOptionalFloatAttr(
        OpAttrDef& attr, std::string& name, size_t index, std::ofstream& outfile) const;
    void AclnnOpGenCodeOptionalIntAttr(OpAttrDef& attr, std::string& name, size_t index, std::ofstream& outfile) const;
    void AclnnOpGenCodeAttrParams(
        OpDef& opDef, std::vector<std::string>& name, std::ofstream& outfile, std::vector<int32_t>& attrTypes) const;
    void AclnnOpGenCodeFunIoTypeCommentImpl(const int32_t type, std::string& name, std::ofstream& outfile) const;
    void AclnnOpGenCodeFunInputComment(
        std::vector<OpParamDef>& inputs, std::vector<OpParamDef>& outputs, OpDefName& opdefName, std::ofstream& outfile,
        uint32_t version) const;
    void AclnnOpGenCodeFunOutputComment(
        std::vector<OpParamDef>& inputs, std::vector<OpParamDef>& outputs, std::vector<std::string>& paramName,
        bool* hasOutputShapeDepend, std::ofstream& outfile) const;
    void AclnnOpGenCodeFunAttrComment(
        std::vector<OpAttrDef>& attrs, std::vector<std::string>& paramName, std::ofstream& outfile,
        uint32_t version) const;
    void AclnnOpGenCodeRunForWSFunComment(
        OpDef& opDef, OpDefName& opdefName, std::ofstream& outfile, uint32_t version) const;
    void AclnnOpGenCodeRunWithWSFunComment(std::string& prefixName, std::ofstream& outfile) const;
    void AclnnOpGenCodeRunForWSFunProto(
        OpDef& opDef, OpDefName& opdefName, std::ofstream& outfile, uint32_t version) const;
    void AclnnOpGenCodeTensorRunForWSFunProto(
        OpDef& opDef, OpDefName& opdefName, ofstream& outfile, uint32_t version) const;
    void AclnnOpGenCodeRunWithWSFunProto(std::string& prefixName, std::ofstream& outfile) const;
    void AclnnOpGenCodeIoParamCheck(
        std::vector<OpParamDef>& param, std::vector<std::string>& name, std::ofstream& outfile, bool isInput) const;
    void AclnnOpGenCodeParamCheck(
        std::vector<OpParamDef>& inputs, std::vector<OpParamDef>& outputs, OpDefName& opdefName,
        std::ofstream& outfile) const;
    void AclnnGenCodeCommFunDelcare(std::ofstream& outfile, bool needInvalidArgumentReport = false) const;
    void AclnnOpGenCodeWorkspaceDelcare(
        OpDef& opDef, OpDefName& opdefName, std::ofstream& outfile, uint32_t version) const;
    void AclnnOpGenCodeIoParamDesc(
        std::vector<OpParamDef>& params, const std::string& desc, std::ofstream& outfile, uint32_t version) const;
    void AclnnOpGenCodeParamDesc(OpDef& opDef, std::ofstream& outfile, uint32_t version) const;
    void AclnnOpGenCodeExecutor(OpDef& opDef, std::ofstream& outfile) const;
    struct SocEntry {
        std::string socName;
        std::string enumConstant;
        bool hasAicoreConfig = false;
        ge::AscendString originalKey;
    };
    std::vector<SocEntry> BuildUnifiedSocList(OpDef& opDef) const;
    void AclnnOpGenSocSupportList(const std::vector<SocEntry>& entries, std::ofstream& outfile) const;
    void AclnnOpGenHcclServerTypeList(OpDef& opDef, const std::vector<SocEntry>& entries, ofstream& outfile) const;
    void AclnnOpGenHcclServerType(OpDef& opDef, ofstream& outfile) const;
    void AclnnOpGenIoTensorDesc(
        size_t i, std::vector<OpParamDef>& params, std::ofstream& outfile, const std::string opType) const;
    void AclnnOpGenTensorDesc(
        size_t index, std::vector<OpParamDef>& inputs, std::vector<OpParamDef>& outputs, std::ofstream& outfile,
        const std::string opType) const;
    void AclnnOpGenOpSupportList(
        size_t index, std::vector<OpParamDef>& inputs, std::vector<OpParamDef>& outputs, std::ofstream& outfile,
        const std::string opType) const;
    void AclnnOpGenOpSupportListAll(
        OpDef& opDef, const std::vector<SocEntry>& socEntries, std::ofstream& outfile) const;
    void AclnnGenOpTypeId(OpDef& opDef, std::ofstream& outfile) const;
    void AclnnGenNameSpaceInfo(std::ofstream& outfile, OpDef& opDef) const;
    void AclnnGenCheckInfo(std::ofstream& outfile, bool needInvalidArgumentReport = false) const;
    bool IsSupportAutoContiguous(std::vector<OpParamDef>& inputs) const;
    // 获取每个输入在不同Soc上的Contiguous配置
    std::vector<InputContiguousConfig> GetInputContiguousConfigs(OpDef& opDef) const;
    // 检查是否有Soc配置了AutoContiguous
    std::map<std::string, bool> GetSocAutoContiguousMap(OpDef& opDef) const;
    // 检查是否需要SOC判断（用于Input的IgnoreContiguous差异或AutoContiguous差异）
    bool NeedSocCheckForContiguous(OpDef& opDef) const;
    // 校验同一个输入在同一个SOC上是否同时配置了AutoContiguous和IgnoreContiguous（两者冲突）
    bool ValidateInputContiguousConflict(OpDef& opDef) const;
    // 检查AutoContiguous配置并打印WARNING日志
    void CheckAutoContiguousWarning(OpDef& opDef) const;
    void AclnnGenUncontDeclaration(OpDef& opDef, std::ofstream& outfile) const;
    void AclnnGenCodeDecImpl(
        std::string& declFile, std::ofstream& outfile, bool needInvalidArgumentReport = false) const;
    void AclnnGenCodeImplStart(
        std::string& declFile, bool hasOutputShapeDepend, std::ofstream& outfile, OpDef& opDef) const;
    void AclnnGenCodeImplEnd(std::ofstream& outfile) const;
    void AclopGenDfxInfo(OpDef& opDef, std::string& opName, std::string& prefixName, std::ofstream& outfile) const;
    // ========== SOC条件判断辅助函数 ==========
    void AnalyzeSocAutoContiguousSupport(
        OpDef& opDef, bool& allSupport, bool& noneSupport, std::vector<std::string>& autoContSocs) const;
    bool HasDefaultAutoContiguous(std::vector<OpParamDef>& inputs) const;
    void GenerateSocConditionCode(
        const std::vector<std::string>& socNames, std::ofstream& outfile, bool withNullCheck,
        const std::string& indent = "    ") const;
    void GenerateCurrentSocDeclaration(std::ofstream& outfile, const std::string& indent = "    ") const;
    void GenerateViewDeclaration(std::ofstream& outfile, const std::string& indent = "    ") const;
    void GetIgnoreContSocsForInput(
        const std::vector<InputContiguousConfig>& contConfigs, size_t idx, bool& hasIgnoreCont,
        std::vector<std::string>& ignoreContSocs) const;
    void GenSingleInputCode(
        OpDefIoDesc& opDefIoDesc, OpDefName& opdefName, std::ofstream& outfile, const OpCodeGenConfig& genConfig,
        const std::vector<InputContiguousConfig>& contConfigs) const;
    void GenDynamicInputIgnoreContCode(
        OpDefIoDesc& opDefIoDesc, const std::vector<InputContiguousConfig>& contConfigs, std::ofstream& outfile,
        const OpCodeGenConfig& genConfig) const;
    void GenDynamicInputWeakSymbolCode(
        OpDefIoDesc& opDefIoDesc, std::ofstream& outfile, const std::string& indent) const;
    void GenInputIgnoreContCode(
        OpDefIoDesc& opDefIoDesc, const std::vector<InputContiguousConfig>& contConfigs, std::ofstream& outfile,
        const OpCodeGenConfig& genConfig) const;
    void AclnnOpGenCodeSetUnContInfo(OpDef& opDef, std::ofstream& outfile, bool needSocCheck) const;
    void AclopGenCodeCommon(
        OpDef& opDef, OpDefName& opdefName, std::ofstream& outfile, uint32_t version, bool valueDependApi) const;
    void AclnnOpGenIoParam(
        std::vector<OpParamDef>& params, std::vector<std::string>& paramName, uint32_t version, const bool isInput,
        std::ofstream& outfile) const;
    void AclnnOpGenAttrDefParam(
        std::vector<OpAttrDef>& attrs, std::vector<std::string>& paramName, std::ofstream& outfile) const;
    void AclnnOpGenDefaultArrayAttr(
        OpAttrDef& attr, std::string attrsName, std::vector<std::string>& defaultAttrsName, int32_t type,
        std::ofstream& outfile) const;
    void AclnnOpGenDefaultAttr(OpDef& opdef, OpDefName& opdefName, uint32_t version, std::ofstream& outfile) const;
    void AclnnOpGenCodeRunForWorkspaceVersionImpl(
        OpDef& opDef, OpDefName& opdefName, uint32_t version, uint32_t maxVersion, std::ofstream& outfile) const;
    void AclnnOpGenCodeRunForWorkspaceImpl(
        OpDef& opDef, OpDefName& opdefName, uint32_t version, std::ofstream& outfile, bool valDependApi) const;
    void AclnnOpGenCodeRunUnContWithWorkspaceImpl(OpDef& opDef, OpDefName& opDefName, std::ofstream& outfile) const;
    void AclnnOpGenCodeRunWithWorkspaceVersionImpl(OpDefName& opdefName, std::ofstream& outfile) const;
    void AclnnOpGenCodeRunWithWorkspaceImpl(OpDef& opDef, OpDefName& opDefName, std::ofstream& outfile) const;
    std::vector<std::string> AclnnOpGetEnvValue() const;
    bool GetInputConfigVerion(OpDef& opDef, std::set<uint32_t>& versions, uint32_t& maxVersion) const;
    bool GetAttrConfigVerion(OpDef& opDef, std::set<uint32_t>& versions, uint32_t& maxVersion) const;
    bool GetConfigVerion(OpDef& opDef, std::set<uint32_t>& versions, uint32_t& maxVersion) const;
    void AclopGenVersionCode(
        OpDef& opDef, OpDefName& opdefName, std::string prefixName, std::set<uint32_t> versions,
        uint32_t maxVersion) const;
    void AclnnOpGenCodeAttrParamDesc(
        std::vector<OpAttrDef>& attrs, const std::string& desc, std::ofstream& outfile, uint32_t version) const;
    std::string ToLower(std::string str) const;
    bool IsSupportProduct(OpDef& opDef) const;
    std::vector<std::string> Spilt(const std::string& str, const char delim) const;
    void AclnnGenMc2Declaration(OpDef& opDef, std::ofstream& outfile) const;
    void AclnnGenOutEmptyLaunchDeclaration(OpDef& opDef, ofstream& outfile) const;
    void AclnnOpGenCodeAttrParamsImpl(
        std::vector<OpAttrDef>& attrs, std::vector<std::string>& name, size_t index, int32_t type,
        std::ofstream& outfile) const;
    void AclnnOpGenCodeHcclGroup(
        OpDef& opDef, std::vector<std::string>& name, std::vector<int32_t> attrTypes, std::ofstream& outfile) const;
    void AclnnOpGenAddParamName(OpDef& opDef, const OpDefName& opdefName, std::ofstream& outfile) const;
    void AclnnAddDisableInputIndex(OpDef& opDef, uint32_t version, std::ofstream& outfile) const;

    bool HasRef(std::vector<std::string>& names) const;
    void AclnnOpGenCodeSetRef(
        std::vector<OpParamDef>& inputs, std::vector<OpParamDef>& outputs, std::ofstream& outfile) const;
    void AclopGenCodeRefContiguous(OpDef& opDef, OpDefName& opdefName, std::ofstream& outfile) const;
    void AclnnOpGenCodeRunRefUnContWithWorkspaceImpl(OpDef& opDef, OpDefName& opDefName, std::ofstream& outfile) const;
    opbuild::Status GenerateCode(void) override;
    void AclnnGenExternFunc(std::ofstream& outfile) const;
    void AclnnOpGenFormatMode(OpDef& opDef, std::ofstream& outfile) const;
    bool IsBaseTypeOfAttr(const char* type) const;
    bool IsOpValueDepend(OpDef& opDef) const;
    ~AclnnOpGenerator() override = default;
    void AclOpGenMatchArgsFunc(ofstream& outfile) const;
    mutable size_t socMatchCounter_ = 0;
    struct SocMatchArrayInfo {
        std::vector<std::string> names;
    };
    mutable std::vector<SocMatchArrayInfo> socMatchArrays_;
    mutable std::map<std::string, size_t> socMatchArrayIndex_;
    size_t GetOrCreateSocMatchArray(const std::vector<std::string>& socNames) const;
    void CollectSocMatchArrays(OpDef& opDef) const;
    void EmitSocMatchArrays(ofstream& outfile) const;
};
} // namespace ops

#endif
