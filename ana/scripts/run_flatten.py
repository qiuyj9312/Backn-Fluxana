#!/usr/bin/env python3
"""
Flatten Event Tool - Python Wrapper
Convert WNSEvent to flattened RDataFrame Tree
"""

import os
import sys
import subprocess
import argparse
import shutil
import json

def check_executable():
    """Check if the flatten_event executable exists"""
    # Get ana directory (parent of scripts directory)
    ana_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    build_dir = os.path.join(ana_dir, "build", "bin")
    exe_path = os.path.join(build_dir, "flatten_event")
    if os.path.exists(exe_path):
        return exe_path
    
    exe_path = os.path.join(ana_dir, "flatten_event")
    if os.path.exists(exe_path):
        return exe_path
    
    if shutil.which("flatten_event"):
        return "flatten_event"
    
    return None

def load_config(config_path):
    """Load configuration from JSON file"""
    with open(config_path, 'r') as f:
        return json.load(f)

def process_batch(filepath_config, exp_config, exe_path, tree_name, exp_config_path):
    """Process all files in batch mode"""
    origin_data = filepath_config['OriginData']
    root_data = filepath_config['RootData']
    filelist = exp_config['Files']['filelist']
    
    os.makedirs(root_data, exist_ok=True)
    
    print(f"Processing {len(filelist)} files...")
    
    for idx, filename in enumerate(filelist, 1):
        input_file = os.path.join(origin_data, filename)
        
        output_file = os.path.join(root_data, filename)
        
        if not os.path.exists(input_file):
            print(f"[{idx}/{len(filelist)}] SKIP: {filename} (not found)")
            continue
        
        if os.path.exists(output_file):
            print(f"[{idx}/{len(filelist)}] SKIP: {filename} (already exists)")
            continue
        
        print(f"[{idx}/{len(filelist)}] {filename}")
        
        try:
            subprocess.run(
                [exe_path, input_file, output_file, tree_name, exp_config_path],
                check=True,
                capture_output=True
            )
            print(f"  -> Success")
        except subprocess.CalledProcessError:
            print(f"  -> Failed")

def process_single(input_file, output_file, exe_path, tree_name):
    """Process single file"""
    if not os.path.exists(input_file):
        print(f"Error: Input file '{input_file}' does not exist")
        sys.exit(1)
    
    output_dir = os.path.dirname(output_file)
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
    
    print(f"Processing: {input_file} -> {output_file}")
    
    try:
        subprocess.run(
            [exe_path, input_file, output_file, tree_name],
            check=True
        )
        print("Success!")
    except subprocess.CalledProcessError:
        print("Failed!")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Flatten WNSEvent to RDataFrame Tree")
    parser.add_argument("input_file", nargs='?', help="Input ROOT file (single mode)")
    parser.add_argument("output_file", nargs='?', help="Output ROOT file (single mode)")
    parser.add_argument("--config", default="../config/filepath.json", help="Config file path")
    parser.add_argument("--executable", help="Path to flatten_event executable")
    parser.add_argument("--tree-name", default=None, help="Tree name")
    
    args = parser.parse_args()
    
    batch_mode = args.input_file is None and args.output_file is None
    
    exe_path = args.executable
    if exe_path is None:
        exe_path = check_executable()
        if exe_path is None:
            print("Error: flatten_event executable not found")
            sys.exit(1)
    
    if batch_mode:
        # Get ana directory (parent of scripts directory)
        ana_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        config_path = args.config if os.path.isabs(args.config) else os.path.join(ana_dir, args.config)
        
        filepath_config = load_config(config_path)
        exp_name = filepath_config['ExpName']
        config_dir = os.path.dirname(config_path)
        
        exp_config_path = os.path.join(config_dir, f"{exp_name}.json")
        exp_config = load_config(exp_config_path)
        
        # Use RawTreeName from Files section, or command line argument
        tree_name = args.tree_name if args.tree_name else exp_config.get('Files', {}).get('RawTreeName', 'EventBranch')
        
        process_batch(filepath_config, exp_config, exe_path, tree_name, exp_config_path)
    else:
        tree_name = args.tree_name if args.tree_name else 'WNSRawTree'
        process_single(args.input_file, args.output_file, exe_path, tree_name)

if __name__ == "__main__":
    main()
