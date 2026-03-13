# run_analysis.py 使用说明

这是一个 Python 交互脚本，用于方便地调用 `rdataframe_analysis` 可执行文件进行数据分析。

## 功能特点

- **交互模式**：提供友好的菜单界面，引导用户选择分析类型
- **命令行模式**：支持直接指定分析类型，适合脚本化使用
- **自动检测**：自动查找可执行文件和配置文件
- **错误处理**：提供清晰的错误提示和使用建议

## 前置要求

1. 已编译 `rdataframe_analysis` 可执行文件：
   ```bash
   cd /home/qyj/work/XSana/ana
   mkdir -p build && cd build
   cmake ..
   make
   ```

2. ROOT 环境已配置（可选，但建议配置）

## 使用方法

### 1. 交互模式

直接运行脚本，会显示分析类型菜单：

```bash
cd /home/qyj/work/XSana/ana/scripts
python3 run_analysis.py
```

然后根据提示输入编号或名称选择分析类型。

### 2. 命令行模式

直接指定分析类型：

```bash
# 运行阈值分析 R1
python3 run_analysis.py GetThR1

# 运行 Gamma 闪光分析
python3 run_analysis.py GetGammaFlash

# 运行 T0 计数
python3 run_analysis.py CountT0
```

### 3. 列出所有可用分析类型

```bash
python3 run_analysis.py --list
```

### 4. 查看帮助信息

```bash
python3 run_analysis.py --help
```

## 可用的分析类型

| 编号 | 名称 | 说明 |
|------|------|------|
| 1 | GetGammaFlash | Gamma 闪光分析 |
| 2 | GetThR1 | 阈值分析 R1 |
| 3 | GetThR2 | 阈值分析 R2 |
| 4 | GetReactionRate | 反应率分析 |
| 5 | GetDtForCalL | 时间差计算 |
| 6 | CalFlightPath | 飞行路径计算 |
| 7 | GetPileupCorr | 堆积修正 |
| 8 | GetXSSingleBunch | 单束截面 |
| 9 | Coincheck | 符合检查 |
| 10 | CountT0 | T0 计数 |
| 11 | AnalyzeWithRDataFrame | 完整 RDataFrame 分析 |

## 高级选项

### 指定可执行文件路径

如果可执行文件不在默认位置，可以手动指定：

```bash
python3 run_analysis.py --executable /path/to/rdataframe_analysis GetThR1
```

## 故障排除

### 错误：未找到 rdataframe_analysis 可执行文件

**解决方法**：
1. 确保已编译项目
2. 检查 `ana/build/bin/` 目录是否存在可执行文件
3. 或使用 `--executable` 参数指定路径

### 警告：未检测到 ROOT 环境

**解决方法**：
```bash
source /path/to/root/bin/thisroot.sh
```

### 错误：配置文件不存在

**解决方法**：
确保在正确的目录运行脚本，配置文件应位于：
- `config/filepath.json`
- `config/2025_232Th.json`（或其他实验配置）

## 注意事项

1. 脚本会自动在项目根目录（`XSana`）运行可执行文件，以确保配置文件路径正确
2. 分析过程中生成的图表需要手动关闭才能继续
3. 可以使用 `Ctrl+C` 中断正在运行的分析

## 示例工作流程

```bash
# 1. 进入脚本目录
cd /home/qyj/work/XSana/ana/scripts

# 2. 列出所有可用分析
python3 run_analysis.py --list

# 3. 运行特定分析
python3 run_analysis.py GetThR1

# 4. 或使用交互模式
python3 run_analysis.py
# 然后输入编号或名称，例如：2 或 GetThR1
```
