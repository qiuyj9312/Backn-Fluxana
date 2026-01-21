# WNSEvent 扁平化工具

将 WNSEvent 数据结构转换为扁平化的 RDataFrame Tree，只保留基本类型和 vector 类型。

## 文件结构

```
.
├── CMakeLists.txt           # CMake 编译配置
├── README_FLATTEN.md        # 本文档
├── src/                     # C++ 源代码
│   ├── flatten_event.cxx    # 扁平化工具主程序
│   └── WNSEvent.cxx         # WNSEvent 类实现
├── include/                 # 头文件
│   ├── WNSEvent.h           # WNSEvent 类定义
│   ├── WNSEventLinkDef.h    # ROOT Dictionary LinkDef
│   ├── mystyle.h            # 样式定义
│   └── pubfunc.h           # 公共函数
├── Lab/                     # 实验目录
│   └── WNSEventDict_rdict.pcm  # ROOT PCM 文件
└── scripts/                 # Python 脚本
    └── run_flatten.py       # Python 调用脚本（支持批量处理）
```

## 扁平化后的数据结构

### Tree 列定义

| 列名           | 类型               | 说明               |
| -------------- | ------------------ | ------------------ |
| `fRunNumber`   | `Int_t`            | 运行编号           |
| `fEventNumber` | `Int_t`            | 事件编号           |
| `fFileNumber`  | `Int_t`            | 文件编号           |
| `fT0Sec`       | `ULong64_t`        | 起始时间（秒）     |
| `fT0NanoSec`   | `ULong64_t`        | 起始时间（纳秒）   |
| `fArrayLength` | `Int_t`            | 数组长度           |
| `fvChannelID`  | `vector<Int_t>`    | 通道 ID 列表       |
| `fvT0`         | `vector<Double_t>` | 零交叉时间         |
| `fvTc1`        | `vector<Double_t>` | 常数分数时间 (0.9) |
| `fvTc2`        | `vector<Double_t>` | 常数分数时间 (1.0) |
| `fvhpn`        | `vector<Double_t>` | 脉冲高度（负）     |
| `fvhp`         | `vector<Double_t>` | 脉冲高度（正）     |
| `fvhn`         | `vector<Double_t>` | 脉冲高度（负）     |

## 编译步骤

### 1. 确保已安装 ROOT

```bash
# 检查 ROOT 是否安装
root-config --version

# 如果未安装，需要先安装 ROOT
```

### 2. 创建构建目录并编译

```bash
# 创建构建目录
mkdir build
cd build

# 运行 CMake
cmake ..

# 编译
make
```

编译成功后，可执行文件位于 `build/bin/flatten_event`

## 使用方法

### 方法 1: 使用 Python 脚本（推荐）

**批量模式（默认）：**

```bash
python scripts/run_flatten.py
```

此模式会自动读取 `config/filepath.json` 和对应的实验配置文件，批量处理所有文件。

**单文件模式：**

```bash
python scripts/run_flatten.py <input_file.root> <output_file.root>
```

示例：

```bash
# 批量处理
python scripts/run_flatten.py

# 单文件处理
python scripts/run_flatten.py Data/input.root Data/output_flat.root

# 指定配置文件
python scripts/run_flatten.py --config config/filepath.json
```

### 方法 2: 直接运行可执行文件

```bash
./build/bin/flatten_event <input_file.root> <output_file.root> [tree_name]
```

示例：

```bash
./build/bin/flatten_event Data/input.root Data/output_flat.root
./build/bin/flatten_event Data/input.root Data/output_flat.root WNSRawTree
```

## 使用 RDataFrame 分析扁平化数据

```cpp
// 读取扁平化后的 Tree
ROOT::RDataFrame df("tree", "Data/output_flat.root");

// 访问基本类型列
auto runNumbers = df.Take<Int_t>("fRunNumber");
auto eventNumbers = df.Take<Int_t>("fEventNumber");

// 访问 vector 类型列
auto channels = df.Take<vector<Int_t>>("fvChannelID");
auto times = df.Take<vector<Double_t>>("fvT0");

// 定义新列
auto df2 = df.Define("nChannels", [](const vector<Int_t>& ch) {
    return ch.size();
}, {"fvChannelID"});

// 过滤事件
auto df3 = df.Filter([](Int_t run) {
    return run > 1000;
}, {"fRunNumber"});

// 绘制直方图
auto h1 = df.Histo1D("fRunNumber");
auto h2 = df.Define("firstChannel", [](const vector<Int_t>& ch) {
    return ch.empty() ? -1 : ch[0];
}, {"fvChannelID"}).Histo1D("firstChannel");

// 保存结果
h1->Draw();
```

## Python 示例（使用 PyROOT）

```python
import ROOT

# 读取扁平化后的 Tree
df = ROOT.RDataFrame("tree", "Data/output_flat.root")

# 访问基本类型列
run_numbers = df.Take("fRunNumber")

# 访问 vector 类型列
channels = df.Take("fvChannelID")

# 定义新列
df2 = df.Define("nChannels", "fvChannelID.size()")

# 过滤事件
df3 = df.Filter("fRunNumber > 1000")

# 绘制直方图
h1 = df.Histo1D("fRunNumber")
h1.Draw()
```

## 输出示例

```
========================================
  WNSEvent Flattening Tool
========================================

Input file:  Data/input.root
Output file: Data/output_flat.root

Total entries: 10000

Processing entries...
  Progress: 0 / 10000 (0.0%)
  Progress: 1000 / 10000 (10.0%)
  Progress: 2000 / 10000 (20.0%)
  ...
  Progress: 10000 / 10000 (100.0%)

Writing output file...
Conversion completed successfully!
Output file: Data/output_flat.root

========================================
Output Tree Structure
========================================
******************************************************************************
*Tree    :tree       : Flattened WNS Event Tree                        *
*Entries :    10000 : Total =          12345678 bytes  File  Size =  9876543 *
*        :          : Tree compression factor =   1.25                      *
******************************************************************************
*Br    0 :fRunNumber : fRunNumber/I                                      *
*Br    1 :fEventNumber : fEventNumber/I                                   *
*Br    2 :fFileNumber : fFileNumber/I                                    *
*Br    3 :fT0Sec     : fT0Sec/l                                          *
*Br    4 :fT0NanoSec : fT0NanoSec/l                                      *
*Br    5 :fArrayLength : fArrayLength/I                                  *
*Br    6 :fvChannelID : vector<Int_t>                                    *
*Br    7 :fvT0       : vector<Double_t>                                  *
*Br    8 :fvTc1      : vector<Double_t>                                  *
*Br    9 :fvTc2      : vector<Double_t>                                  *
*Br   10 :fvhpn      : vector<Double_t>                                  *
*Br   11 :fvhp       : vector<Double_t>                                  *
*Br   12 :fvhn       : vector<Double_t>                                  *
******************************************************************************

Total entries: 10000
```

## 技术细节

### 扁平化原理

原始 WNSEvent 包含嵌套的 `EventHeader` 类：

```cpp
class WNSEvent {
    EventHeader fEventHeader;  // 嵌套类
    vector<Int_t> fvChannelID;
    // ...
};
```

扁平化后，`EventHeader` 的字段直接展开：

```cpp
class WNSEventFlat {
    Int_t fRunNumber;      // 从 EventHeader.fRunNumber
    Int_t fEventNumber;    // 从 EventHeader.fEventNumber
    Int_t fFileNumber;     // 从 EventHeader.fFileNumber
    ULong64_t fT0Sec;      // 从 EventHeader.fT0Sec
    ULong64_t fT0NanoSec;  // 从 EventHeader.fT0NanoSec
    vector<Int_t> fvChannelID;
    // ...
};
```

### RDataFrame 兼容性

扁平化后的结构完全兼容 RDataFrame，因为：

- 所有成员变量都是基本类型（`Int_t`, `ULong64_t`, `Double_t`）
- 或者是 `vector<T>` 类型（RDataFrame 原生支持）
- 没有嵌套类或复杂对象

## 常见问题

### Q: 编译时提示找不到 ROOT？

A: 确保已安装 ROOT 并正确配置环境变量：

```bash
source /path/to/root/bin/thisroot.sh
```

### Q: 运行时提示找不到 flatten_event？

A: 确保已编译程序：

```bash
mkdir build && cd build
cmake .. && make
```

### Q: 输入文件中没有 'tree'？

A: 确保输入文件包含名为 'tree' 的 TTree，且分支名为 'event'

### Q: 如何查看输出文件的结构？

A: 使用 ROOT 命令：

```bash
root -l output_flat.root
root [0] tree->Print()
```

## 许可证

本工具遵循原 WNSEvent 项目的许可证。
