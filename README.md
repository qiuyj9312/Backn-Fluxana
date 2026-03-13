# XSana (Cross Section Analysis)

## 项目简介 (Project Description)

XSana 是一个用于核物理实验数据分析与模拟的综合软件框架。它主要聚焦于中子能谱测量、时间飞行（TOF - Time of Flight）分析、核反应截面（Cross Section）测量以及探测器的响应模拟。该项目结合了用于数据高效处理的 **ROOT RDataFrame** 框架与用于蒙特卡罗物理模拟的 **Geant4** 工具包，构建了从实验运行模拟到最终物理量提取的完整流水线。

## 核心特性 (Core Features)

- **高性能数据分析**：基于 C++ 和 `ROOT RDataFrame`（包含部分多线程支持）处理海量核实验数据，可实现快速的直方图填充、Pileup（堆积）校正、符合（Coincidence）事件过滤与误差计算。
- **探测器模拟**：深度集成了 Geant4 (`Simulation/fixm_nattention`, `Simulation/fixm_ffeff`)，能够真实模拟实验中粒子传输、能量沉积与探测器响应。支持通过宏文件（Macro）或交互式 UI 运行。
- **配置驱动**：采用 JSON 格式配置（如 `202512_Flux.json`, `2025_232Th.json`），灵活控制积分步长（`Intergralnsub`）、阈值等核心参数，无需频繁重新编译代码。支持 Python 基可视化 UI 配置器修改参数。
- **分析模块化**：项目划分了诸如 `NeutronFluxAnalysis`（中子通量分析）、`CrossSectionAnalysis`（截面分析）、解卷（DemoUnfolding）等专属模块，使得代码结构更加清晰，复用更加便捷。

## 目录结构 (Directory Structure)

```
XSana/
├── ana/                 # ROOT 数据分析与处理代码 (RDataFrame)
│   ├── src/             # 分析模块源码 (NeutronFluxAnalysis, CrossSectionAnalysis 等)
│   ├── include/         # 头文件目录
│   └── DemoUnfolding.cpp# 解卷测试模块
├── Simulation/          # Geant4 蒙特卡罗模拟目录
│   ├── fixm_nattention/ # 模拟程序一 (sim.cc)
│   └── fixm_ffeff/      # 模拟程序二
├── config/              # JSON 配置文件存放目录 (例如 filepath.json)
├── Data/                # 实验/模拟输入基础数据
├── OutPut/              # 分析与模拟结果输出 (ROOT 文件、PDF 图像等)
└── Para/                # 数据所需的一些环境参数或对照截面库
```

## 环境依赖 (Dependencies)

- **C++17** 或更高版本
- **CMake** (3.15+) : 项目的构建系统。
- **ROOT** (6.24+) : 必须支持 C++14/17，强依赖其 `RDataFrame` 接口及其各项组件（如 `TGraph`, `TH1D` 等）。
- **Geant4** (11.3+) : 用于运行 `Simulation` 目录下的模拟程序（需预编译出带 UI 组件如 OpenGL 和 Qt 的版本以支持交互）。
- **Python 3** (带 `json` 模块): 附带的 JSON 编辑/处理脚本。

## 快速指南 (Getting Started)

### 1. 运行 Geant4 模拟 (Simulation)

进入模拟目录并编译：

```bash
cd Simulation/fixm_nattention
mkdir build && cd build
cmake ..
make -j4
```

**运行模拟**：

- **交互模式**：`./sim` 启动 UI 界面并可视化探测器。
- **批处理模式**：`./sim run.mac` 通过宏文件执行并发模拟。

### 2. 执行数据分析 (Analysis)

处理实验数据时，主要在 `ana` 目录下完成代码编译，然后通过 `ana/scripts` 目录下预置的 Python 脚本进行自动化运行和调用：

**编译分析程序**：

```bash
cd ana
mkdir build && cd build
cmake ..
make -j4
```

**运行分析脚本（推荐方式）**：

```bash
cd scripts
python3 run_analysis.py
```

`run_analysis.py` 会提供一个交互式菜单，引导您选择各种分析类型（如 Gamma 闪光、阈值分析、堆积修正、符合检查等）。脚本会自动查找生成的 C++ 可执行程序，并依据 `config/202512_Flux.json` 或 `config/filepath.json` 等配置文件读取数据路径进行分析。输出文件通常为 `.root` 直方图分布保存至 `OutPut/` 目录中。

## 许可证 (License)

遵循本机构/课题组的内部使用及开源规范。
