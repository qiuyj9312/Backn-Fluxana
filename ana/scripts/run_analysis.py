#!/usr/bin/env python3
"""
RDataFrame Analysis Tool - Python Wrapper
Interactive script to run RDataFrame analysis
"""

import os
import sys
import subprocess
import argparse
import json
from typing import Optional, Dict, List


# 分析类型按 DataType 分组
# DataType 对应 filepath.json 中的 DataType 字段，决定使用哪个分析类
ANALYSIS_GROUPS = {
    "Flux": {
        "desc": "中子通量分析 (NeutronFluxAnalysis)",
        "types": {
            "CountT0":                    "T0 计数",
            "GetGammaFlash":              "Gamma 闪光时间拟合",
            "GetThR1":                    "阈值分析 R1（能量 vs 幅度）",
            "GetDtForCalL":               "飞行路径时间差",
            "CalFlightPath":              "飞行路径长度计算",
            "GetThR2":                    "阈值分析 R2（指数拟合）",
            "GetReactionRate":            "反应率直方图",
            "EvalDeltaTc1":               "评估相邻fTc1差值",
            "GetPileupCorr":              "堆积修正",
            "GetHRateXSUF":               "解谱后反应率",
            "Coincheck":                  "符合检查",
            "CalFlux":                    "中子通量计算",
            "CalUncertainty":             "通量不确定度计算",
            "AnalyzeWithRDataFrame":      "完整 RDataFrame 分析",
        }
    },
    "XS": {
        "desc": "截面分析 (CrossSectionAnalysis)",
        "types": {
            "GetXSSingleBunch": "单束截面计算",
            "CalUncertainty":   "截面不确定度计算（TODO）",
        }
    },
}

# 扁平化的分析类型字典（用于验证和描述查找）
ANALYSIS_TYPES: Dict[str, str] = {
    name: desc
    for group in ANALYSIS_GROUPS.values()
    for name, desc in group["types"].items()
}


def check_executable() -> Optional[str]:
    """检查 rdataframe_analysis 可执行文件是否存在"""
    # 获取 ana 目录（scripts 的父目录）
    ana_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    # 检查 build/bin/ 目录
    build_dir = os.path.join(ana_dir, "build", "bin")
    exe_path = os.path.join(build_dir, "rdataframe_analysis")
    if os.path.exists(exe_path):
        return exe_path
    
    # 检查 ana 目录
    exe_path = os.path.join(ana_dir, "rdataframe_analysis")
    if os.path.exists(exe_path):
        return exe_path
    
    # 检查系统 PATH
    import shutil
    if shutil.which("rdataframe_analysis"):
        return "rdataframe_analysis"
    
    return None


def check_root_environment() -> bool:
    """检查 ROOT 环境是否配置"""
    import shutil
    return shutil.which("root") is not None


def load_config(config_path: str) -> Optional[Dict]:
    """加载配置文件"""
    try:
        with open(config_path, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"错误：配置文件不存在: {config_path}")
        return None
    except json.JSONDecodeError as e:
        print(f"错误：配置文件格式错误: {e}")
        return None


def prepare_output_directories(project_root: str):
    """根据配置文件准备输出目录结构"""
    config_path = os.path.join(project_root, "config", "filepath.json")
    config = load_config(config_path)
    if not config:
        print("警告：无法加载配置文件，跳过输出目录检查。")
        return

    data_typ = config.get("DataTyp")
    exp_name = config.get("ExpName")
    
    if not data_typ or not exp_name:
        print("警告：配置文件中缺少 DataTyp 或 ExpName 字段，跳过输出目录检查。")
        return

    if data_typ == "Flux":
        base_path = config.get("FluxPath")
    else:
        base_path = config.get("XSPath")

    if not base_path:
        print(f"警告：配置文件中未找到对应的输出路径，跳过检查。")
        return

    exp_dir = os.path.join(base_path, exp_name)
    subdirs = ["cutgamma", "Outcome", "para"]
    
    print(f"检查输出目录 ({exp_dir}):")

    for subdir in subdirs:
        dir_to_create = os.path.join(exp_dir, subdir)
        if not os.path.exists(dir_to_create):
            try:
                os.makedirs(dir_to_create)
                print(f"  [+] 已创建子目录: {subdir}")
            except Exception as e:
                print(f"  [x] 错误：无法创建 {subdir}: {e}")
        else:
            print(f"  [v] 已存在子目录: {subdir}")
    print()


def print_banner():
    """打印欢迎横幅"""
    print("=" * 60)
    print(" " * 15 + "RDataFrame Analysis Tool")
    print("=" * 60)
    print()


def list_analysis_types():
    """按 DataType 分组列出所有可用的分析类型"""
    print("\n可用的分析类型：\n")
    idx = 1
    for data_type, group in ANALYSIS_GROUPS.items():
        print(f"  [ DataType = {data_type} ]  {group['desc']}")
        for name, desc in group["types"].items():
            print(f"    {idx:2d}. {name:30s} - {desc}")
            idx += 1
        print()


def interactive_select_analysis() -> Optional[str]:
    """交互式选择分析类型"""
    print_banner()
    list_analysis_types()

    # 构建全局编号 → 分析名称的映射
    indexed: List[str] = [
        name
        for group in ANALYSIS_GROUPS.values()
        for name in group["types"]
    ]

    while True:
        try:
            choice = input("请选择分析类型（输入编号或名称，输入 'q' 退出）: ").strip()

            if choice.lower() in ['q', 'quit', 'exit']:
                print("退出程序。")
                return None

            # 尝试作为编号解析
            if choice.isdigit():
                idx = int(choice)
                if 1 <= idx <= len(indexed):
                    return indexed[idx - 1]
                else:
                    print(f"错误：编号必须在 1-{len(indexed)} 之间")
                    continue

            # 精确名称匹配
            if choice in ANALYSIS_TYPES:
                return choice

            # 模糊匹配
            matches = [name for name in ANALYSIS_TYPES if choice.lower() in name.lower()]
            if len(matches) == 1:
                return matches[0]
            elif len(matches) > 1:
                print(f"错误：匹配到多个分析类型: {', '.join(matches)}")
                print("请输入更精确的名称或使用编号。")
            else:
                print(f"错误：未找到分析类型 '{choice}'")
                print("请输入有效的编号或名称。")

        except KeyboardInterrupt:
            print("\n\n程序被中断。")
            return None
        except Exception as e:
            print(f"错误：{e}")


def run_analysis(exe_path: str, analysis_type: str) -> int:
    """运行分析"""
    print(f"\n正在运行分析: {analysis_type}")
    print(f"说明: {ANALYSIS_TYPES.get(analysis_type, '未知')}")
    print("-" * 60)
    
    # 确定工作目录：项目根目录（XSana）
    # exe_path 可能是 /path/to/XSana/ana/build/bin/rdataframe_analysis
    # 我们需要在 /path/to/XSana 目录运行
    ana_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    project_root = os.path.dirname(ana_dir)  # XSana 目录
    
    try:
        # 直接运行，实时显示输出
        result = subprocess.run(
            [exe_path, analysis_type],
            cwd=project_root  # 在项目根目录运行
        )
        return result.returncode
    except KeyboardInterrupt:
        print("\n\n分析被用户中断。")
        return 1
    except Exception as e:
        print(f"\n错误：运行分析失败: {e}")
        return 1


def main():
    parser = argparse.ArgumentParser(
        description="RDataFrame Analysis Tool - 交互式分析脚本",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 交互模式
  %(prog)s
  
  # 直接运行指定分析
  %(prog)s GetThR1
  
  # 列出所有可用分析类型
  %(prog)s --list
        """
    )
    
    parser.add_argument(
        "analysis_type",
        nargs='?',
        help="分析类型（可选，不指定则进入交互模式）"
    )
    
    parser.add_argument(
        "--list", "-l",
        action="store_true",
        help="列出所有可用的分析类型"
    )
    
    parser.add_argument(
        "--executable",
        help="指定 rdataframe_analysis 可执行文件路径"
    )
    
    args = parser.parse_args()
    
    # 如果只是列出分析类型
    if args.list:
        list_analysis_types()
        return 0
    
    # 检查 ROOT 环境
    if not check_root_environment():
        print("警告：未检测到 ROOT 环境，请确保已正确配置 ROOT。")
        print("提示：运行 'source /path/to/root/bin/thisroot.sh'")
        print()
    
    # 查找可执行文件
    exe_path = args.executable
    if exe_path is None:
        exe_path = check_executable()
        if exe_path is None:
            print("错误：未找到 rdataframe_analysis 可执行文件")
            print("提示：")
            print("  1. 确保已编译项目: cd ana/build && cmake .. && make")
            print("  2. 或使用 --executable 参数指定可执行文件路径")
            return 1
    
    if not os.path.exists(exe_path) and exe_path != "rdataframe_analysis":
        print(f"错误：可执行文件不存在: {exe_path}")
        return 1
        
    # 提前创建并检查输出文件夹
    ana_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    project_root = os.path.dirname(ana_dir)
    prepare_output_directories(project_root)
        
    # 确定分析类型
    analysis_type = args.analysis_type
    
    if analysis_type is None:
        # 交互模式
        analysis_type = interactive_select_analysis()
        if analysis_type is None:
            return 0
    else:
        # 验证分析类型
        if analysis_type not in ANALYSIS_TYPES:
            print(f"错误：未知的分析类型 '{analysis_type}'")
            print("\n可用的分析类型：")
            for name in ANALYSIS_TYPES.keys():
                print(f"  - {name}")
            return 1
    
    # 运行分析
    return run_analysis(exe_path, analysis_type)


if __name__ == "__main__":
    sys.exit(main())
