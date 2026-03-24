# XSana (Cross Section Analysis Framework)

XSana 是一个由 **ROOT RDataFrame** 驱动的高性能、自动化核物理实验数据分析框架。该项目旨在为课题组内部及外部合作者提供一套从实验初始化、数据预处理、物理量提取（通量/截面）到模拟对比的一站式解决方案。

---

## 🚀 快速上手 (Quick Start)

### 1. 环境准备

运行本项目前，请确保您的系统已满足以下条件：

- **ROOT (6.24+)**：必须包含 `RDataFrame` 支持。
- **C++17** 或更高版本的编译器。
- **Python 3**。

**重要提示**：在执行任何分析命令前，请务必加载 ROOT 环境变量（例如运行 `source /path/to/root/bin/thisroot.sh`）。

### 2. 一键项目初始化

项目根目录下包含一个 `init.py` 脚本，用于自动构建项目所需的目录结构并生成基础配置文件：

```bash
python3 init.py
```

该脚本会执行以下操作：

- 创建 `Data/`, `OutPut/`, `Para/` 等必要的子目录。
- 在 `config/` 目录下生成初始的 `filepath.json`。

### 3. 数据准备

根据 `config/filepath.json` 中的 `OriginData` 定义，请确保您的原始 `.root` 数据文件已放置在正确的物理路径下（默认通常为项目根目录下的 `Data/OriginData/`）。
根据 `config/filepath.json` 中的 `T0Data` 定义，请确保您的原始 `.root` 数据文件已放置在正确的物理路径下（默认通常为项目根目录下的 `Data/T0Data/`）。

### 4. 编译分析程序

进入分析目录并执行 CMake 构建：

```bash
cd ana
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

---

## ⚙️ 配置指南 (Configuration)

在 XSana 中，切换不同的实验或分析对象非常简单：

1.  **全局路径管理** (`config/filepath.json`)：
    重点修改 `"ExpName"` 字段。将其值设置为您要调用的物理配置文件名（不带 `.json` 后缀）。
2.  **物理参数配置** (`config/[ExpName].json`)：
    例如 `2025_232Th.json`。此文件定义了时间窗口、能量阈值、积分步长（`Intergralnsub`）等核心物理参数。

---

## 📊 运行分析 (Running Analysis)

> [!TIP]
> **📖 详细物理逻辑说明**
> 关于 `ana` 模块中各个分析函数的底层物理逻辑、计算公式及参数说明，见：
> [XSana 分析函数详细说明 (IHEP Jupyter)](hhttps://jupyter.ihep.ac.cn/s/izkrsQpPM)

XSana 提供了多种工具来调用 `rdataframe_analysis` 可执行文件。推荐优先使用 **网页 UI 模式**。

### 1. 网页 UI 模式 (推荐)

通过可视化界面选择任务、实时查看终端日志并管理分析流程。

```bash
python3 ana/scripts/run_analysis_ui.py
```

启动后，控制台会显示访问地址（通常为 `http://localhost:8080`）。您可以直观地在侧边栏选择任务并一键运行。

### 2. 终端交互模式

通过友好的菜单界面引导用户选择分析类型：

```bash
python3 ana/scripts/run_analysis.py
```

直接运行脚本后，根据提示输入编号或名称即可开始分析。

### 3. 命令行快捷模式

支持直接指定分析类型，适合脚本化或高级用户使用：

```bash
# 直接指定分析名称
python3 ana/scripts/run_analysis.py GetThR1

# 列出所有可用的分析类型
python3 ana/scripts/run_analysis.py --list

# 查看详细帮助
python3 ana/scripts/run_analysis.py --help
```

### 📋 可用的分析类型

| 编号 | 名称                  | 类型     | 说明                        |
| ---- | --------------------- | -------- | --------------------------- |
| 1    | AnalyzeWithRDataFrame | 预处理   | 完整 RDataFrame 分析        |
| 2    | EvalDeltaTc1          | 预处理   | 评估相邻事件时间差          |
| 3    | CountT0               | 预处理   | T0 计数                     |
| 4    | GetGammaFlash         | 预处理   | Gamma 闪光时间拟合          |
| 5    | GetThR1               | 预处理   | 阈值分析 R1（能量 vs 幅度） |
| 6    | GetDtForCalL          | 预处理   | 飞行路径时间差              |
| 7    | CalFlightPath         | 预处理   | 飞行路径长度计算            |
| 8    | GetThR2               | 预处理   | 阈值分析 R2（指数拟合）     |
| 9    | GetReactionRate       | 预处理   | 反应率直方图                |
| 10   | GetPileupCorr         | 预处理   | 脉冲堆积修正                |
| 11   | GetHRateXSUF          | 预处理   | 解谱后反应率                |
| 12   | Coincheck             | 预处理   | 一致性检查                  |
| 13   | RunUnfolding          | 解谱     | 运行解谱程序                |
| 14   | CalSimTrans           | 模拟处理 | 模拟通量衰减计算            |
| 15   | CalUncertainty (Flux) | 通量     | 通量不确定度计算            |
| 16   | CalFlux               | 通量     | 中子通量计算                |
| 17   | CalUncertainty (XS)   | 截面     | 截面不确定度计算            |
| 18   | GetXSSingleBunch      | 截面     | 单束截面计算                |

### 💡 高级用法与注意事项

- **自动检测**：脚本会自动查找 `ana/build/bin/rdataframe_analysis`，若在非标准位置，可使用 `--executable` 参数指定。
- **路径切换**：脚本会自动切换到项目根目录运行，以确保能正确读取 `config/` 下的 JSON 配置。
- **环境要求**：确保 `LD_LIBRARY_PATH` 包含 ROOT 库路径。如果遇到错误，请检查是否已执行 `source thisroot.sh`。
- **中断分析**：可以使用 `Ctrl+C` 随时安全中止当前分析进程。

## 🛰️ 模拟计算 (Simulation)

### 1. 功能概览

`fixm_nattention` 是一个基于 Geant4 的探测器模拟程序，专门用于模拟中子在探测器靶组（室）中的**衰减 (Attenuation)** 过程。模拟结果将作为 `CalSimTrans` 物理任务的输入，以修正最终的通量 (Flux) 计算。

### 2. 编译与运行 (Compilation & Run)

编译模拟程序：

```bash
cd Simulation/fixm_nattention
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

运行模拟：

```bash
./sim run.mac
```

_提示：运行前请确保环境中已加载 Geant4 环境变量（例如运行 `source geant4.sh`）。_

### 3. 靶材配置 (Detector Configuration)

靶材配置需手动修改源代码：

- **修改文件**：`Simulation/fixm_nattention/src/DetectorConstruction.cc`
- **核心参数**（约 382–394 行）：通过修改数组 `a_Material_sample` (材质)、`a_Radius_tub_sample` (半径)、`a_Hight_tub_sample` (厚度) 来定义不同通道的靶组属性。
- **生效方法**：修改并保存后，需返回 `build` 目录执行 `make` 重新编译。

### 4. 后续分析对接 (Post-processing)

模拟运行完成后，会在 `OutPut/` 目录下生成一个以时间戳命名的文件夹（如 `20260313005125`）。

- **编辑配置**：打开 `config/` 目录下对应的实验 JSON 配置文件。
- **更新路径**：在 `"FIXM" -> "Global" -> "SimFixmNAttentionFolders"` 数组中添加该文件夹名称。
- **运行任务**：在任务列表或 Web UI 中选择并启动 **14. CalSimTrans**，程序将自动扫描模拟数据并计算物理修正因子。

---

## 📂 项目目录说明 (Structure)

- **ana/**：包含所有基于 RDataFrame 的 C++ 分析模块逻辑。
- **Simulation/**：Geant4 探测器模拟程序。
- **config/**：存放所有 JSON 格式的路径及物理参数配置。
- **Data/**：原始数据（OriginData）及中间处理数据（RootData/T0Data）。
- **OutPut/**：最终分析结果（通量 Flux / 截面 XS）的输出路径。
- **Para/**：存放对照截面库等环境参数文件。

---
