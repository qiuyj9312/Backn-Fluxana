# RDataFrame 分析脚本使用说明

## 概述

[`rdataframe_analysis.cpp`](rdataframe_analysis.cpp) 是一个功能完善的 ROOT 脚本，用于读取 JSON 配置文件并使用 RDataFrame 分析扁平化的 ROOT 数据。

## 功能特性

1. **配置文件读取**

   - 从 [`config/filepath.json`](../config/filepath.json) 读取根数据目录路径和实验名称
   - 根据实验名称自动加载对应的实验配置文件（如 [`config/2025_232Th.json`](../config/2025_232Th.json)）
   - 使用 nlohmann/json 库进行 JSON 解析

2. **数据链初始化**

   - 从实验配置文件中提取 ROOT 文件列表
   - 使用 TChain 串联所有输入文件
   - 自动处理文件路径拼接

3. **R 数据分析**

   - 使用 ROOT 的 RDataFrame 进行高效数据分析
   - 支持扁平化的数据结构（来自 [`flatten_event.cpp`](flatten_event.cpp)）
   - 提供丰富的统计信息和分析功能

4. **错误处理**
   - 文件不存在检测
   - JSON 解析错误处理
   - TChain 初始化错误处理
   - RDataFrame 异常捕获

## 数据结构

脚本处理的数据结构来自 [`flatten_event.cpp`](flatten_event.cpp) 转换后的扁平化 Tree，包含以下分支：

| 分支名         | 类型               | 描述           |
| -------------- | ------------------ | -------------- |
| `fRunNumber`   | `Int_t`            | 运行号         |
| `fEventNumber` | `Int_t`            | 事件号         |
| `fFileNumber`  | `Int_t`            | 文件号         |
| `fT0Sec`       | `ULong64_t`        | 时间戳（秒）   |
| `fT0NanoSec`   | `ULong64_t`        | 时间戳（纳秒） |
| `fArrayLength` | `Int_t`            | 数组长度       |
| `fvChannelID`  | `vector<Int_t>`    | 通道 ID 列表   |
| `fvT0`         | `vector<Double_t>` | T0 时间列表    |
| `fvTc1`        | `vector<Double_t>` | Tc1 时间列表   |
| `fvTc2`        | `vector<Double_t>` | Tc2 时间列表   |
| `fvhpn`        | `vector<Double_t>` | hpn 值列表     |
| `fvhp`         | `vector<Double_t>` | hp 值列表      |
| `fvhn`         | `vector<Double_t>` | hn 值列表      |

## CMake 编译方法

### 依赖项

- ROOT (版本 >= 6.14)
- nlohmann/json (版本 >= 3.0.0)
- CMake (版本 >= 3.10)
- C++17 兼容的编译器（GCC 7+, Clang 5+）

#### 安装 nlohmann/json 库

**Ubuntu/Debian:**

```bash
sudo apt-get install nlohmann-json3-dev
```

**CentOS/RHEL:**

```bash
sudo yum install json-devel
```

**手动安装:**

```bash
# 下载最新版本
wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
sudo mkdir -p /usr/local/include/nlohmann
sudo mv json.hpp /usr/local/include/nlohmann/
```

### 编译步骤

#### 步骤 1：设置 ROOT 环境

```bash
source /path/to/root/bin/thisroot.sh
```

#### 步骤 2：进入 ana 目录

```bash
cd ana
```

#### 步骤 3：创建构建目录

```bash
mkdir -p build
cd build
```

#### 步骤 4：运行 CMake 配置

```bash
cmake ..
```

CMake 会自动检测 ROOT 和 nlohmann/json 库，并输出配置信息：

```
-- ROOT include dirs: /path/to/root/include
-- ROOT libraries: Core;Tree;RDataFrame...
-- C++ standard: 17
-- Building rdataframe_analysis: YES
-- nlohmann/json include: /usr/include
========================================
Configuration Summary
========================================
```

如果 nlohmann/json 未找到，会显示警告：

```
WARNING: nlohmann/json library not found. rdataframe_analysis will not be built.
```

#### 步骤 5：编译项目

```bash
make -j$(nproc)
```

或使用 CMake：

```bash
cmake --build . -- -j$(nproc)
```

#### 步骤 6：编译完成

编译成功后，可执行文件位于 `build/bin/` 目录：

```
build/bin/
├── flatten_event
└── rdataframe_analysis
```

### 快速编译脚本

如果您想一键编译，可以创建一个简单的脚本：

```bash
#!/bin/bash
cd ana
mkdir -p build && cd build
cmake .. && make -j$(nproc)
```

## 使用方法

### 基本用法

```bash
cd ana/build/bin
./rdataframe_analysis
```

### 指定配置文件

```bash
cd ana/build/bin
./rdataframe_analysis ../config/filepath.json
```

### 从项目根目录运行

```bash
./ana/build/bin/rdataframe_analysis
```

## 配置文件格式

### filepath.json

```json
{
    "ExpName": "2025_232Th",
    "RootData": "/path/to/Data/RootData/",
    ...
}
```

### 实验配置文件 (如 2025_232Th.json)

```json
{
  "ExperimentCondition": {
    "RootTreeName": "tree"
  },
  "Files": {
    "filelist": ["run19778.root", "run19779.root"]
  }
}
```

## 输出示例

脚本运行时会输出以下信息：

```
========================================
  ROOT RDataFrame Analysis Tool
========================================

========================================
Reading filepath configuration
========================================
Config file: config/filepath.json
RootData path: /home/qyj/work/XSana/Data/RootData/
Experiment name: 2025_232Th

========================================
Reading experiment configuration
========================================
Config file: config/2025_232Th.json
Tree name: tree
Number of files: 1
  [0] run19778.root

========================================
Initializing TChain
========================================
Adding file: /home/qyj/work/XSana/Data/RootData/run19778.root
  -> Success (1 trees added)

Summary: 1 files added, 0 files failed
Total entries in chain: 10000

========================================
Analyzing with RDataFrame
========================================
Available columns:
  - fRunNumber
  - fEventNumber
  - fFileNumber
  - fT0Sec
  - fT0NanoSec
  - fArrayLength
  - fvChannelID
  - fvT0
  - fvTc1
  - fvTc2
  - fvhpn
  - fvhp
  - fvhn

Basic Statistics:
  Total entries: 10000
  Number of unique runs: 1

Array Length Statistics:
  Mean:   8.0
  Min:    8
  Max:    8
  StdDev: 0.0

Channel Statistics:
  Average channels per event: 8

Time Range (seconds):
  Min: 1234567890
  Max: 1234568900

T0 Statistics (mean per event):
  Mean:   -1135.5
  Min:    -1140.0
  Max:    -1120.0

First 5 Events:
  Event 0:
    Run Number:      19778
    Event Number:    0
    Array Length:    8
  ...

========================================
RDataFrame Analysis Completed
========================================

Program completed successfully!
```

## R 数据分析功能

脚本演示了以下 RDataFrame 功能：

1. **基本统计**

   - 总事件数统计
   - 唯一运行号计算

2. **描述性统计**

   - 数组长度的均值、最小值、最大值、标准差
   - 通道统计信息

3. **时间分析**

   - 时间戳范围
   - T0 时间统计

4. **数据浏览**
   - 显示可用列名
   - 打印前 N 个事件的详细信息

## 扩展功能

您可以根据需要扩展脚本的功能：

### 添加自定义分析

```cpp
// 在 AnalyzeWithRDataFrame 函数中添加
auto customAnalysis = df.Define("custom_column", [](const std::vector<Double_t>& t0) {
    // 自定义计算逻辑
    return t0.size() > 0 ? t0[0] : 0.0;
}, {"fvT0"});

auto result = customAnalysis.Mean("custom_column");
std::cout << "Custom result: " << *result << std::endl;
```

### 创建直方图

```cpp
auto hist = df.Histo1D({"h1", "Array Length", 100, 0, 20}, "fArrayLength");
hist->Draw();
```

### 过滤数据

```cpp
auto filtered = df.Filter("fArrayLength > 5");
auto count = filtered.Count();
std::cout << "Events with array length > 5: " << *count << std::endl;
```

## 故障排除

### 问题：找不到 ROOT 环境

**解决方案：**

```bash
source /path/to/root/bin/thisroot.sh
```

### 问题：找不到 nlohmann/json 库

**解决方案：**
安装 nlohmann/json 库（见上面的安装说明），或修改 [`CMakeLists.txt`](CMakeLists.txt:19) 中的 `NLOHMANN_JSON_INCLUDE_DIR` 路径。

### 问题：无法打开配置文件

**解决方案：**
确保从项目根目录运行脚本，或提供正确的配置文件路径。

### 问题：TChain 没有条目

**解决方案：**
检查：

1. `RootData` 路径是否正确
2. 文件列表中的文件是否存在
3. 树名称是否正确

### 问题：CMake 配置失败

**解决方案：**

1. 检查 ROOT 是否正确安装和配置
2. 检查 CMake 版本是否 >= 3.10
3. 清理构建目录重新尝试：
   ```bash
   cd ana
   rm -rf build
   mkdir build && cd build
   cmake ..
   ```

## 技术细节

### CMake 配置说明

[`CMakeLists.txt`](CMakeLists.txt:1) 中的关键配置：

| 配置项                                 | 说明                       |
| -------------------------------------- | -------------------------- |
| `CMAKE_CXX_STANDARD`                   | 设置为 17，需要 C++17 支持 |
| `find_package(ROOT REQUIRED)`          | 查找 ROOT 包，必须存在     |
| `find_path(NLOHMANN_JSON_INCLUDE_DIR)` | 查找 nlohmann/json 库      |
| `add_executable(rdataframe_analysis)`  | 创建可执行文件             |
| `target_link_libraries()`              | 链接 ROOT 库               |
| `RUNTIME_OUTPUT_DIRECTORY`             | 输出目录设置为 `build/bin` |

### 配置结构体

```cpp
struct AnalysisConfig {
  std::string rootDataPath;      // ROOT数据目录路径
  std::string expName;            // 实验名称
  std::vector<std::string> fileList; // ROOT文件列表
  std::string treeName;           // 树名称
};
```

### 主要函数

- `ReadJSONFile()`: 读取并解析 JSON 文件
- `ReadFilepathConfig()`: 读取 filepath.json 配置
- `ReadExperimentConfig()`: 读取实验配置文件
- `InitializeTChain()`: 初始化 TChain 对象
- `AnalyzeWithRDataFrame()`: 使用 RDataFrame 进行数据分析

## 许可证

本脚本遵循项目整体许可证。

## 联系方式

如有问题或建议，请联系项目维护者。
