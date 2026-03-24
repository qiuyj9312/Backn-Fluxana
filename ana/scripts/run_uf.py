#!/bin/bash
###
 # @Descripttion: 
 # @version: 
 # @Author: Qiu Yijia
 # @Date: 2023-10-09 19:13:41
 # @LastEditors: Qiu Yijia
 # @LastEditTime: 2023-10-24 19:22:03
### 
import sys
import json
import subprocess

def run_script(nhist, runnumT, scalefactor, encut, isprint):
    infilename = 'hrate.root'    
    command = ['./ana/build/bin/DemoUnfolding', "-inf", infilename, "-noh", nhist, "-run", runnumT, "-fsc", scalefactor, "-dis", isprint, "-ecut", encut] 
    process2 = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, bufsize=1)
    # 实时打印输出
    while True:
        output = process2.stdout.readline()
        if not output and process2.poll() is not None:
            break
        if output:
            print(output.strip())
    print("所有命令执行完毕。")

def main():
    run_script("1", '15', '1', '9e3', '0')
    run_script("2", '15', '1', '9e3', '0')
    run_script("3", '15', '1', '9e3', '0')
    run_script("4", '15', '1', '9e3', '0')
    run_script("5", '15', '1', '9e3', '0')
    run_script("6", '15', '1', '9e3', '0')
    run_script("7", '15', '1', '9e3', '0')
    run_script("8", '15', '1', '9e3', '0')

# 执行主函数
if __name__ == "__main__":
    main()