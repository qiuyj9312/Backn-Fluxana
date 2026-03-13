# RDataFrameAnalysis 类重构总结

## 重构完成时间

2026-02-11

## 重构目标

将 `RDataFrameAnalysis` 重构为基类,并创建两个派生类:

- `CrossSectionAnalysis`: 用于处理截面分析
- `NeutronFluxAnalysis`: 用于处理中子通量分析

## 实施的修改

### 1. 基类修改 (`RDataFrameAnalysis`)

#### 头文件 (`include/RDataFrameAnalysis.h`)

- ✅ 将 `RunAnalysis()` 方法改为 `virtual`
- ✅ 将所有 `private` 成员变量改为 `protected`
- ✅ 移除 `GetXSSingleBunch()` 方法声明

#### 实现文件 (`src/RDataFrameAnalysis.cxx`)

- ✅ 从 `RunAnalysis()` 中移除 `GetXSSingleBunch` 分支
- ✅ 完全删除 `GetXSSingleBunch()` 方法实现(约280行代码)

### 2. 派生类创建

#### CrossSectionAnalysis

**文件**: `include/CrossSectionAnalysis.h`, `src/CrossSectionAnalysis.cxx`

**功能**:

- 继承自 `RDataFrameAnalysis`
- 包含唯一的派生类专属方法: `GetXSSingleBunch()`
- 重写 `RunAnalysis()` 方法:
  - 处理 `GetXSSingleBunch` 类型
  - 其他类型转发给基类

**代码示例**:

```cpp
bool CrossSectionAnalysis::RunAnalysis(const std::string &analysisType) {
  if (!m_isInitialized && !InitializeTChain()) {
    return false;
  }

  if (analysisType == "GetXSSingleBunch") {
    GetXSSingleBunch();
    return true;
  } else {
    // 调用基类的分析方法
    return RDataFrameAnalysis::RunAnalysis(analysisType);
  }
}
```

#### NeutronFluxAnalysis

**文件**: `include/NeutronFluxAnalysis.h`, `src/NeutronFluxAnalysis.cxx`

**功能**:

- 继承自 `RDataFrameAnalysis`
- 无专属方法,所有功能继承自基类
- 提供语义上的分类
- 重写 `RunAnalysis()` 直接调用基类实现

**代码示例**:

```cpp
bool NeutronFluxAnalysis::RunAnalysis(const std::string &analysisType) {
  // 直接调用基类实现
  return RDataFrameAnalysis::RunAnalysis(analysisType);
}
```

### 3. 主程序修改 (`rdataframe_analysis.cpp`)

**自动类选择逻辑**:

```cpp
// 截面分析类型列表
std::vector<std::string> xsAnalysisTypes = {
    "GetXSSingleBunch"
};

// 检查是否为截面分析
bool isCrossSectionAnalysis = false;
for (const auto& type : xsAnalysisTypes) {
    if (analysisType == type) {
        isCrossSectionAnalysis = true;
        break;
    }
}

if (isCrossSectionAnalysis) {
    // 使用截面分析类
    CrossSectionAnalysis analysis(configReader);
    success = analysis.RunAnalysis(analysisType);
} else {
    // 使用中子通量分析类
    NeutronFluxAnalysis analysis(configReader);
    success = analysis.RunAnalysis(analysisType);
}
```

### 4. 构建系统修改 (`CMakeLists.txt`)

添加新的源文件:

```cmake
add_executable(rdataframe_analysis
    rdataframe_analysis.cpp
    src/RDataFrameAnalysis.cxx
    src/CrossSectionAnalysis.cxx      # 新增
    src/NeutronFluxAnalysis.cxx       # 新增
    src/ConfigReader.cxx
)
```

### 5. 工具函数修复 (`include/utils.h`)

**问题**: 多重定义错误

**解决方案**:

- ✅ 在所有函数定义前添加 `inline` 关键字
- ✅ 将 `color` 数组改为 `static`
- ✅ 修复 `TMath::Sqrt` 拼写错误

## 类层次结构

```
RDataFrameAnalysis (基类)
├── 通用分析方法:
│   ├── GetGammaFlash()
│   ├── GetThR1()
│   ├── GetThR2()
│   ├── GetReactionRate()
│   ├── GetDtForCalL()
│   ├── CalFlightPath()
│   ├── GetPileupCorr()
│   ├── CoincheckSimple()
│   ├── CoincheckSingleBunchSimple()
│   ├── CountT0()
│   └── GetHRateXSUF()
│
├── CrossSectionAnalysis (派生类)
│   └── GetXSSingleBunch() [专属方法]
│
└── NeutronFluxAnalysis (派生类)
    └── [无专属方法,全部继承]
```

## 设计优势

1. **代码复用**: 所有通用功能保留在基类中
2. **清晰的职责分离**: 截面分析和通量分析分别由不同类处理
3. **易于扩展**: 未来可以轻松添加新的分析类型
4. **向后兼容**: 保持了原有的分析接口
5. **自动选择**: 主程序自动根据分析类型选择合适的类

## 编译状态

✅ **编译成功**

```
[ 45%] Built target flatten_event
[ 54%] Building CXX object CMakeFiles/rdataframe_analysis.dir/src/RDataFrameAnalysis.cxx.o
[ 63%] Building CXX object CMakeFiles/rdataframe_analysis.dir/src/CrossSectionAnalysis.cxx.o
[ 72%] Linking CXX executable bin/rdataframe_analysis
[100%] Built target rdataframe_analysis
```

## 文件清单

### 新增文件

- `ana/include/CrossSectionAnalysis.h`
- `ana/src/CrossSectionAnalysis.cxx`
- `ana/include/NeutronFluxAnalysis.h`
- `ana/src/NeutronFluxAnalysis.cxx`

### 修改文件

- `ana/include/RDataFrameAnalysis.h`
- `ana/src/RDataFrameAnalysis.cxx`
- `ana/rdataframe_analysis.cpp`
- `ana/CMakeLists.txt`
- `ana/include/utils.h`

## 使用方法

重构后的使用方式与之前完全相同:

```bash
# 截面分析 (自动使用 CrossSectionAnalysis)
./bin/rdataframe_analysis config.json GetXSSingleBunch

# 其他分析 (自动使用 NeutronFluxAnalysis)
./bin/rdataframe_analysis config.json GetReactionRate
./bin/rdataframe_analysis config.json CountT0
```

## 后续建议

1. **功能测试**: 测试所有分析类型以确保功能正常
2. **性能验证**: 确认重构没有引入性能问题
3. **文档更新**: 更新用户文档说明新的类结构
4. **代码审查**: 进行代码审查以确保质量

## 注意事项

- 所有现有的分析方法都保留在基类中,只有 `GetXSSingleBunch` 移到了派生类
- 主程序会自动选择正确的分析类,用户无需关心内部实现
- 重构保持了完全的向后兼容性
