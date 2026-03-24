#!/usr/bin/env python3
import os
import json

def main():
    # 获取脚本所在的目录，作为项目根目录
    # 这样可以确保即使用户在任意路径下调用 `python init.py` 也能得到正确的基准路径
    proj_dir = os.path.dirname(os.path.abspath(__file__))
    print(f"当前项目目录: {proj_dir}")

    # 需要创建的目录列表
    directories_to_create = [
        "Para",
        "Data/OriginData",
        "Data/RootData",
        "Data/T0Data",
        "OutPut/XS",
        "OutPut/Flux",
        "Simulation/fixm_nattention",
        "Protondata",
        "config"
    ]

    print("正在创建数据和输目录...")
    for d in directories_to_create:
        dir_path = os.path.join(proj_dir, d)
        os.makedirs(dir_path, exist_ok=True)
        # print(f"  确保目录存在: {dir_path}")
        
    print("目录创建完成。")

    # 生成 config/filepath.json
    config_file = os.path.join(proj_dir, "config", "filepath.json")
    print(f"正在生成 {config_file} ...")

    config_data = {
        "ExpName": "example",
        "DataType": "Flux",
        "ParaPath": f"{proj_dir}/Para/",
        "OriginData": f"{proj_dir}/Data/OriginData/",
        "RootData": f"{proj_dir}/Data/RootData/",
        "T0Path": f"{proj_dir}/Data/T0Data/",
        "XSPath": f"{proj_dir}/OutPut/XS/",
        "FluxPath": f"{proj_dir}/OutPut/Flux/",
        "SimFixmNAttentionPath": f"{proj_dir}/Simulation/fixm_nattention/",
        "BeamDataPath": f"{proj_dir}/Protondata/",
        "ENDFDATA235UNF": "ENDFXS235UNF.dat",
        "ENDFDATA238UNF": "ENDFXS238UNF.dat",
        "UNDETECLiSi": "UNDETECLiSi.dat",
        "UNENDFDATA235UNF": "UNENDFXS235UNF.dat",
        "UNENDFDATA238UNF": "UNENDFXS238UNF.dat",
        "ENDFDATA235UNTOT": "ENDFXS235UNTOT.dat",
        "ENDFDATA238UNTOT": "ENDFXS238UNTOT.dat",
        "ENDFDATALi6NT": "ENDFXS6LiNT.dat",
        "ENDFDATALi6NTOT": "ENDFXS6LiNTOT.dat",
        "DELTALDATA": "En_DeltaL.dat",
        "EFFLiSi": "eff_LiSi_2_303030.dat"
    }

    # 写入 JSON 文件，采用缩进为 2 且保留非 ASCII 字符
    with open(config_file, "w", encoding="utf-8") as f:
        json.dump(config_data, f, indent=2, ensure_ascii=False)
        # 手动写个换行，保持一般文件习惯
        f.write("\n")

    print("初始化完成！")

if __name__ == "__main__":
    main()
