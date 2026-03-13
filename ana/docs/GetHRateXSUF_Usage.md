# GetHRateXSUF 函数使用说明

## 功能概述

`GetHRateXSUF()` 函数是从原始的 `gethratexs_uf.cpp` 改写而来,集成到 `RDataFrameAnalysis` 类中。该函数的主要功能是:

1. 从 UF 分析结果文件中读取能量分布直方图 (`h_finalE`)
2. 使用蒙特卡洛方法计算 rate×xs 直方图
3. 支持两种计算模式:
   - 不考虑自屏蔽效应 (默认启用)
   - 考虑自屏蔽效应 (代码中已实现,需取消注释)

## 函数签名

```cpp
void GetHRateXSUF(double ntimes = 10.0);
```

### 参数说明

- `ntimes`: 蒙特卡洛采样倍数,默认值为 10.0
  - 该参数控制从能量分布直方图中随机采样的次数
  - 采样次数 = 直方图积分 × ntimes
  - 数值越大,统计精度越高,但计算时间也越长

## 使用方法

### 方法1: 通过 Python 脚本调用

```python
from analysis import RDataFrameAnalysis, ConfigReader

# 创建配置读取器
config_reader = ConfigReader("config/2025_232Th.json", "config/filepath.json")

# 创建分析对象
analysis = RDataFrameAnalysis(config_reader)

# 运行 GetHRateXSUF 分析
analysis.RunAnalysis("GetHRateXSUF")
```

### 方法2: 通过命令行调用

```bash
cd /home/qyj/work/XSana/ana/build
./bin/rdataframe_analysis GetHRateXSUF
```

### 方法3: 在 C++ 代码中直接调用

```cpp
ConfigReader configReader("config/2025_232Th.json", "config/filepath.json");
RDataFrameAnalysis analysis(configReader);

// 使用默认采样倍数 (10.0)
analysis.GetHRateXSUF();

// 或指定采样倍数
analysis.GetHRateXSUF(20.0);  // 使用 20 倍采样
```

## 输入文件要求

### 1. UF 结果文件

函数会从以下路径读取 UF 分析结果:

```
{SpectrumPath}/{ExpName}/Outcome/FIXM/UF_{chID}.root
```

每个文件必须包含名为 `h_finalE` 的 TH1D 直方图,该直方图表示该通道的能量分布。

### 2. ENDF 截面数据文件

需要以下 ENDF 数据文件:

- **U-235 (235U)**:
  - NF (裂变截面): 从配置文件读取 (`GetENDFDataU5NF()`)
  - NTOT (总截面): 从配置文件读取 (`GetENDFDataU5NTOT()`)

- **U-238 (238U)**:
  - NF: `{DataPath}/ENDFXS8LiNF.dat`
  - NTOT: `{DataPath}/ENDFXS8LiNTOT.dat`

### 3. 配置文件

需要在 JSON 配置文件中包含以下信息:

```json
{
  "FIXM": {
    "CHIDUSE": [1, 2, 3, ...],
    "CHID": [1, 2, 3, ...],
    "SampleType": ["235U", "238U", ...],
    "Mass": [100.0, 150.0, ...],
    "Radius": [10.0, 10.0, ...],
    "A": [235.0, 238.0, ...]
  }
}
```

## 输出文件

函数会生成以下输出文件:

```
{SpectrumPath}/{ExpName}/Outcome/FIXM/hratexsuf.root
```

该 ROOT 文件包含每个通道的 rate×xs 直方图:

- `h1_Enxs_{chID}`: 不考虑自屏蔽效应的 rate×xs 直方图
- `h1_EnxsNs_{chID}`: 考虑自屏蔽效应的 rate×xs 直方图 (需取消注释)

## 计算原理

### 不考虑自屏蔽效应

对于每个随机采样的能量 En:

```
weight = 1 / σ_nf(En)
```

其中 σ_nf(En) 是该能量下的裂变截面。

### 考虑自屏蔽效应

对于每个随机采样的能量 En:

```
yield = 1 - exp(-n_d × σ_tot(En))
weight = σ_tot(En) / (yield × σ_nf(En))
```

其中:

- n_d: 面密度 (atoms/barn)
- σ_tot(En): 总截面
- σ_nf(En): 裂变截面

## 代码改进说明

相比原始的 `gethratexs_uf.cpp`,新实现有以下改进:

1. **集成到类结构**: 作为 `RDataFrameAnalysis` 的成员函数,可以复用配置读取和辅助方法
2. **配置驱动**: 从 `ConfigReader` 读取所有配置,无需硬编码路径
3. **使用成员变量**:
   - 使用 `m_xs_nr` 和 `m_xs_ntot` 存储截面数据,避免重复加载和内存泄漏
   - 使用 `m_xsPath`、`m_expName`、`m_channelIDs`、`m_fixmConfig` 等成员变量
4. **复用成员函数**:
   - 调用 `LoadENDFData()` 加载所有 ENDF 数据(包括 U-235、U-238、Li-6)
   - 使用 `PrintSectionHeader()`、`PrintChannelInfo()` 等辅助函数
   - 调用 `InitializeCommonConfig()` 初始化配置
5. **更好的错误处理**:
   - 检查文件存在性和直方图有效性
   - 检查截面数据是否存在于 map 中,避免访问不存在的键
6. **内存管理**:
   - 使用 `SetDirectory(0)` 避免直方图内存泄漏
   - ENDF TGraph 由成员变量管理,无需手动删除
7. **代码可读性**: 使用 lambda 函数封装计算逻辑,代码更清晰
8. **标准化命名**: 支持 "235U"、"238U"、"6Li" 等标准样本类型命名
9. **扩展性**: 通过 `LoadENDFData()` 自动支持更多同位素,无需修改本函数

### 与其他分析函数的一致性

优化后的 `GetHRateXSUF()` 函数与类中其他分析函数(如 `GetReactionRate()`)保持一致的代码风格:

- 使用相同的初始化流程
- 使用相同的 ENDF 数据加载机制
- 使用相同的输出格式和错误处理方式

## 注意事项

1. **能量上限**: 计算时只考虑 En ≤ 3×10^8 eV 的能量范围
2. **截面有效性**: 会检查截面值是否大于 0,避免除零错误
3. **自屏蔽修正**: 默认情况下自屏蔽修正代码被注释,如需启用请取消相关注释
4. **采样倍数**: ntimes 越大统计越好,但计算时间也越长,建议根据实际需求调整

## 示例输出

```
========================================
GetHRateXSUF Analysis
========================================
Number of channels to analyze: 5
Monte Carlo sampling factor: 10
Loaded ENDF data files:
  U5 NF: /path/to/ENDFXS6LiNF.dat
  U5 NTOT: /path/to/ENDFXS6LiNTOT.dat
  U8 NF: /path/to/ENDFXS8LiNF.dat
  U8 NTOT: /path/to/ENDFXS8LiNTOT.dat
Reading UF result files from: /path/to/Outcome/FIXM

Processing channel 1...
  Calculating rate×xs (without self-shielding)...

Processing channel 2...
  Calculating rate×xs (without self-shielding)...

...

Output file saved: /path/to/Outcome/FIXM/hratexsuf.root
```
