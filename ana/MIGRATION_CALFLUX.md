# CalFlux 迁移总结

## 概述

成功将 `/home/qyj/work/specana/ana/FIXM/calflux_dbbunch.cpp` 迁移到 `NeutronFluxAnalysis` 框架中。

## 迁移日期

2026-02-11

## 源文件

- **原始文件**: `/home/qyj/work/specana/ana/FIXM/calflux_dbbunch.cpp`
- **目标类**: `NeutronFluxAnalysis`

## 实现的功能

### CalFlux 方法

计算中子通量,包括以下步骤:

1. **加载输入数据** (`LoadFluxInputData`)
   - 从 `hrate.root` 加载反应率直方图
   - 从 `hratexsuf.root` 加载展开后的反应率
   - 从 `fluxattenuation.root` 加载通量衰减修正
   - 合并低能区(< 10 keV)和高能区数据
   - 应用探测器效率修正
   - 应用通量衰减修正

2. **按样品类型计算通量** (`CalculateFluxByType`)
   - 按样品类型(如 U5, U8, Th)聚合各通道数据
   - 计算总面积密度
   - 归一化到实验时间、有效面积和束流功率
   - 归一化到 100 kW 标准束流功率

3. **加载不确定度** (`LoadFluxUncertainty`)
   - 从 `herror.root` 加载各样品类型的总不确定度

4. **写入结果** (`WriteFluxResults`)
   - 输出 `Flux.root`: 包含各样品类型的通量直方图
   - 输出 `<SampleType>_Flux.dat`: 文本格式的通量数据
   - 输出积分通量统计信息

5. **绘制图形** (`DrawFluxPlots`)
   - 绘制各样品类型的通量对比图
   - 对 U8 应用低能截断(< 1 MeV)

## 文件修改

### 1. NeutronFluxAnalysis.h

添加了:

- `CalFlux()` 方法声明
- `FluxData` 辅助数据结构
- 5 个辅助函数声明:
  - `LoadFluxInputData()`
  - `CalculateFluxByType()`
  - `LoadFluxUncertainty()`
  - `WriteFluxResults()`
  - `DrawFluxPlots()`

### 2. NeutronFluxAnalysis.cxx

添加了:

- `CalFlux()` 主方法实现(~70 行)
- 5 个辅助函数实现(~200 行)
- 修改 `RunAnalysis()` 方法,添加 CalFlux 调度

### 3. rdataframe_analysis.cpp

更新了:

- 帮助信息,添加 `CalFlux` 和 `CalUncertainty` 选项

## 使用方法

```bash
# 计算中子通量
./bin/rdataframe_analysis CalFlux

# 计算不确定度
./bin/rdataframe_analysis CalUncertainty
```

## 输入文件

CalFlux 需要以下输入文件(位于 `<XSPath>/<ExpName>/Outcome/FIXM/`):

1. `hrate.root` - 反应率直方图(低能区)
2. `hratexsuf.root` - 展开后的反应率直方图(高能区)
3. `fluxattenuation.root` - 通量衰减修正
4. `herror.root` - 不确定度数据(可选)

## 输出文件

CalFlux 生成以下输出文件(位于 `<XSPath>/<ExpName>/Outcome/FIXM/`):

1. `Flux.root` - 包含各样品类型的通量直方图
2. `<SampleType>_Flux.dat` - 文本格式的通量数据
   - 列: Energy(MeV), dN/dlogE, Uncertainty, Relative_Uncertainty(%)

## 配置参数

从配置文件中读取:

- 实验时间 (`timeList`)
- 通道配置 (质量、半径、面积密度、探测器效率等)
- 能量分档参数 (`bpd`)

### 待完成项 (TODO)

以下参数目前使用硬编码值,需要添加到 `ConfigReader`:

1. **BeamPower** (束流功率)
   - 当前值: 95.0 kW
   - 配置位置: `ExperimentCondition.BeamPower.value`

2. **BeamRadius** (束流半径)
   - 当前值: 30.0 mm
   - 配置位置: `ExperimentCondition.BeamRadius.value`

## 与原始代码的差异

1. **架构改进**
   - 从独立脚本迁移到类方法
   - 使用辅助函数分解复杂逻辑
   - 遵循 DRY 原则

2. **配置管理**
   - 使用 `ConfigReader` 统一管理配置
   - 避免硬编码文件路径

3. **代码简化**
   - 移除了注释掉的 Method 2 代码
   - 移除了未使用的比率图绘制代码

4. **错误处理**
   - 添加了更完善的文件检查
   - 添加了详细的日志输出

## 编译状态

✅ 编译成功 (2026-02-11 15:26)

## 测试建议

1. 准备必要的输入文件
2. 运行 `./bin/rdataframe_analysis CalFlux`
3. 检查输出文件:
   - `Flux.root` 是否生成
   - 各样品类型的 `.dat` 文件是否正确
4. 验证通量值与原始脚本的结果一致

## 依赖关系

CalFlux 依赖于以下分析步骤的输出:

1. `GetReactionRate` - 生成 `hrate.root`
2. `GetHRateXSUF` - 生成 `hratexsuf.root`
3. 通量衰减分析 - 生成 `fluxattenuation.root`
4. `CalUncertainty` - 生成 `herror.root` (可选)

## 后续工作

1. 添加 `GetBeamPower()` 和 `GetBeamRadius()` 到 `ConfigReader`
2. 测试 CalFlux 功能
3. 验证输出结果的正确性
4. 考虑添加单元测试
