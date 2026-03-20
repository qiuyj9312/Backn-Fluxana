import http.server
import socketserver
import json
import os
import urllib.parse
import argparse
import sys
import subprocess
import threading
import uuid
import time
from typing import Dict, Any

# 尝试导入 run_analysis 中的信息
try:
    import run_analysis
    ANALYSIS_GROUPS = run_analysis.ANALYSIS_GROUPS
    check_executable = run_analysis.check_executable
except ImportError:
    print("错误: 无法导入 run_analysis.py，请确保该脚本在同一目录下。")
    sys.exit(1)

HTML_CONTENT = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Run Analysis UI - XSana</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-dark: #0f172a;
            --bg-panel: #1e293b;
            --bg-card: #2dd4bf10;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --accent: #2dd4bf;
            --accent-hover: #14b8a6;
            --danger: #f43f5e;
            --success: #10b981;
            --border: #334155;
            --terminal-bg: #000000;
            --terminal-fg: #10b981;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Inter', sans-serif;
        }

        body {
            background-color: var(--bg-dark);
            color: var(--text-main);
            display: flex;
            height: 100vh;
            overflow: hidden;
        }

        /* Sidebar */
        .sidebar {
            width: 320px;
            background-color: var(--bg-panel);
            border-right: 1px solid var(--border);
            display: flex;
            flex-direction: column;
        }

        .sidebar-header {
            padding: 20px;
            border-bottom: 1px solid var(--border);
            font-size: 1.2rem;
            font-weight: 600;
            color: var(--accent);
            letter-spacing: 0.5px;
            flex-shrink: 0;
        }

        .file-list {
            flex: 1;
            overflow-y: auto;
            padding: 10px 0;
        }

        .group-header {
            padding: 12px 20px;
            font-size: 0.85rem;
            text-transform: uppercase;
            font-weight: 700;
            color: var(--text-muted);
            letter-spacing: 1px;
            margin-top: 10px;
        }

        .file-item {
            padding: 10px 20px 10px 30px;
            cursor: pointer;
            transition: all 0.2s;
            color: var(--text-muted);
            font-size: 0.95rem;
            display: flex;
            flex-direction: column;
        }
        
        .file-item .title {
            font-family: 'JetBrains Mono', monospace;
            font-size: 0.9rem;
            color: var(--text-main);
            margin-bottom: 4px;
        }

        .file-item .desc {
            font-size: 0.8rem;
            opacity: 0.7;
        }

        .file-item:hover {
            background-color: rgba(255, 255, 255, 0.05);
        }

        .file-item.active {
            background-color: var(--bg-card);
            border-right: 3px solid var(--accent);
        }
        
        .file-item.active .title {
            color: var(--accent);
        }

        /* Main Content */
        .main {
            flex: 1;
            display: flex;
            flex-direction: column;
            background: linear-gradient(135deg, var(--bg-dark) 0%, #1e1e2f 100%);
        }

        .topbar {
            height: 70px;
            padding: 0 30px;
            border-bottom: 1px solid var(--border);
            display: flex;
            justify-content: space-between;
            align-items: center;
            background-color: rgba(30, 41, 59, 0.8);
            backdrop-filter: blur(10px);
            flex-shrink: 0;
        }

        .filename-display {
            font-size: 1.2rem;
            font-weight: 500;
            color: var(--text-main);
            display: flex;
            align-items: center;
            gap: 15px;
        }

        .status-badge {
            font-size: 0.8rem;
            padding: 4px 10px;
            border-radius: 12px;
            font-weight: 600;
            text-transform: uppercase;
        }

        .status-idle { background-color: var(--border); color: var(--text-muted); }
        .status-running { background-color: rgba(45, 212, 191, 0.2); color: var(--accent); animation: pulse 1.5s infinite; }
        .status-success { background-color: rgba(16, 185, 129, 0.2); color: var(--success); }
        .status-failed { background-color: rgba(244, 63, 94, 0.2); color: var(--danger); }

        @keyframes pulse {
            0% { opacity: 0.6; }
            50% { opacity: 1; }
            100% { opacity: 0.6; }
        }

        .btn-run {
            background-color: var(--accent);
            color: #000;
            border: none;
            padding: 10px 24px;
            border-radius: 6px;
            font-weight: 600;
            font-size: 0.95rem;
            cursor: pointer;
            transition: all 0.2s ease;
            box-shadow: 0 4px 12px rgba(45, 212, 191, 0.2);
            display: flex;
            align-items: center;
            gap: 8px;
        }

        .btn-run:hover {
            background-color: var(--accent-hover);
            transform: translateY(-1px);
            box-shadow: 0 6px 16px rgba(45, 212, 191, 0.3);
        }

        .btn-run:disabled {
            background-color: var(--border);
            color: var(--text-muted);
            cursor: not-allowed;
            transform: none;
            box-shadow: none;
        }

        .btn-stop {
            background-color: var(--danger);
            color: #fff;
            border: none;
            padding: 10px 24px;
            border-radius: 6px;
            font-weight: 600;
            font-size: 0.95rem;
            cursor: pointer;
            transition: all 0.2s ease;
            box-shadow: 0 4px 12px rgba(244, 63, 94, 0.2);
            display: none;
            align-items: center;
            gap: 8px;
            margin-left: 10px;
        }

        .btn-stop:hover {
            background-color: #e11d48;
            transform: translateY(-1px);
            box-shadow: 0 6px 16px rgba(244, 63, 94, 0.3);
        }

        .editor-container {
            flex: 1;
            padding: 20px;
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }

        .info-panel {
            background-color: rgba(30, 41, 59, 0.5);
            border: 1px solid var(--border);
            border-radius: 8px;
            padding: 20px;
            margin-bottom: 20px;
            display: none;
        }
        
        .info-panel h3 { margin-bottom: 8px; color: var(--accent); }
        .info-panel p { color: var(--text-muted); font-size: 0.95rem; line-height: 1.5; }

        .terminal {
            flex: 1;
            background-color: var(--terminal-bg);
            border-radius: 8px;
            border: 1px solid var(--border);
            padding: 15px;
            overflow-y: auto;
            font-family: 'JetBrains Mono', monospace;
            font-size: 0.85rem;
            color: var(--text-main);
            line-height: 1.4;
            display: flex;
            flex-direction: column;
            box-shadow: inset 0 2px 10px rgba(0,0,0,0.5);
        }

        .terminal-line { margin-bottom: 2px; white-space: pre-wrap; word-wrap: break-word; }
        .terminal-info { color: #60a5fa; }
        .terminal-warn { color: #facc15; }
        .terminal-error { color: #f87171; }
        .terminal-success { color: var(--success); }
        .terminal-highlight { color: var(--accent); }

        .empty-state {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100%;
            color: var(--text-muted);
        }
        .empty-icon {
            font-size: 3rem;
            margin-bottom: 16px;
            opacity: 0.5;
        }

        /* Scrollbar */
        ::-webkit-scrollbar { width: 8px; height: 8px; }
        ::-webkit-scrollbar-track { background: rgba(0,0,0,0.1); }
        ::-webkit-scrollbar-thumb { background: var(--border); border-radius: 4px; }
        ::-webkit-scrollbar-thumb:hover { background: #475569; }

        .toast {
            position: fixed;
            bottom: 24px;
            right: 24px;
            background: var(--bg-panel);
            border-left: 4px solid var(--accent);
            color: white;
            padding: 16px 24px;
            border-radius: 4px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.5);
            transform: translateX(150%);
            transition: transform 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
            z-index: 1000;
        }
        .toast.error { border-left-color: var(--danger); }
        .toast.show { transform: translateX(0); }
    </style>
</head>
<body>

    <div class="sidebar">
        <div class="sidebar-header">
            ⚡ Analysis UI
        </div>
        <div class="file-list" id="analysisList">
            <!-- Loaded dynamically -->
        </div>
    </div>

    <div class="main">
        <div class="topbar">
            <div class="filename-display">
                <span id="currentAnalysis">Select an analysis</span>
                <span id="statusBadge" class="status-badge status-idle" style="display: none;">IDLE</span>
            </div>
            <div style="display: flex; align-items: center;">
                <button class="btn-run" id="runBtn" style="display: none;">
                    🚀 Run Analysis
                </button>
                <button class="btn-stop" id="stopBtn">
                    🛑 Stop
                </button>
            </div>
        </div>
        
        <div class="editor-container" id="mainContainer">
            <div class="empty-state" id="emptyState">
                <div class="empty-icon">📊</div>
                <h2>No Analysis Selected</h2>
                <p>Choose an analysis type from the sidebar to begin.</p>
            </div>
            
            <div class="info-panel" id="infoPanel">
                <h3 id="infoTitle">Analysis Name</h3>
                <p id="infoDesc">Description</p>
                <p id="infoGroup" style="margin-top: 8px; font-size: 0.8rem; opacity: 0.5;">Group: </p>
            </div>
            
            <div class="terminal" id="terminal" style="display: none;">
                <div style="color: var(--text-muted); text-align: center; margin-bottom: 10px;">--- Terminal Output ---</div>
            </div>
        </div>
    </div>

    <div class="toast" id="toast">Message!</div>

    <script>
        let currentType = null;
        let currentJobId = null;
        let logOffset = 0;
        let pollInterval = null;
        let isRunning = false;

        document.addEventListener('DOMContentLoaded', () => {
            fetchAnalysisTypes();
            document.getElementById('runBtn').addEventListener('click', runAnalysis);
            document.getElementById('stopBtn').addEventListener('click', stopAnalysis);
        });

        async function fetchAnalysisTypes() {
            try {
                const res = await fetch('/api/analysis-types');
                const data = await res.json();
                const list = document.getElementById('analysisList');
                list.innerHTML = '';
                
                // data = { "PreProcessing": { "desc": "...", "types": { "V": "..." } }, ... }
                for (const [groupName, groupData] of Object.entries(data)) {
                    const header = document.createElement('div');
                    header.className = 'group-header';
                    header.textContent = `${groupName} — ${groupData.desc}`;
                    list.appendChild(header);
                    
                    for (const [typeName, typeDesc] of Object.entries(groupData.types)) {
                        const item = document.createElement('div');
                        item.className = 'file-item';
                        item.dataset.type = typeName;
                        item.dataset.desc = typeDesc;
                        item.dataset.group = groupName;
                        item.innerHTML = `
                            <div class="title">${typeName}</div>
                            <div class="desc">${typeDesc}</div>
                        `;
                        item.onclick = () => selectAnalysis(typeName, typeDesc, groupName, item);
                        list.appendChild(item);
                    }
                }
            } catch (e) {
                showToast("Failed to load analysis types.", true);
                console.error(e);
            }
        }

        function selectAnalysis(typeName, desc, groupName, element) {
            if (isRunning) {
                showToast("Cannot change analysis while running.", true);
                return;
            }
        
            currentType = typeName;
            
            // Update UI
            document.querySelectorAll('.file-item').forEach(el => el.classList.remove('active'));
            element.classList.add('active');
            
            document.getElementById('emptyState').style.display = 'none';
            document.getElementById('infoPanel').style.display = 'block';
            document.getElementById('terminal').style.display = 'flex';
            document.getElementById('runBtn').style.display = 'flex';
            
            document.getElementById('currentAnalysis').textContent = typeName;
            document.getElementById('statusBadge').style.display = 'inline-block';
            updateBadge('idle');
            
            document.getElementById('infoTitle').textContent = typeName;
            document.getElementById('infoDesc').textContent = desc;
            document.getElementById('infoGroup').textContent = `Group: ${groupName}`;
            
            // Clear terminal if previous job was done
            if(pollInterval == null) {
                clearTerminal();
            }
        }
        
        function updateBadge(state) {
            const badge = document.getElementById('statusBadge');
            badge.className = 'status-badge';
            if (state === 'idle') {
                badge.classList.add('status-idle');
                badge.textContent = 'IDLE';
            } else if (state === 'running') {
                badge.classList.add('status-running');
                badge.textContent = 'RUNNING';
            } else if (state === 'success') {
                badge.classList.add('status-success');
                badge.textContent = 'SUCCESS';
            } else if (state === 'failed') {
                badge.classList.add('status-failed');
                badge.textContent = 'FAILED';
            }
        }

        function clearTerminal() {
            const term = document.getElementById('terminal');
            term.innerHTML = '<div style="color: var(--text-muted); text-align: center; margin-bottom: 10px;">--- Terminal Output ---</div>';
        }

        function appendLog(text) {
            const term = document.getElementById('terminal');
            const line = document.createElement('div');
            line.className = 'terminal-line';
            
            // Basic colorization
            if (text.toLowerCase().includes('error') || text.toLowerCase().includes('failed') || text.toLowerCase().includes('exception')) {
                line.classList.add('terminal-error');
            } else if (text.toLowerCase().includes('warn')) {
                line.classList.add('terminal-warn');
            } else if (text.includes('[+]') || text.toLowerCase().includes('success')) {
                line.classList.add('terminal-success');
            } else if (text.includes('[-]')) {
                line.classList.add('terminal-highlight');
            }
            
            line.textContent = text;
            term.appendChild(line);
            
            // Auto scroll to bottom
            term.scrollTop = term.scrollHeight;
        }

        async function runAnalysis() {
            if (!currentType || isRunning) return;
            
            clearTerminal();
            updateBadge('running');
            const btn = document.getElementById('runBtn');
            const stopBtn = document.getElementById('stopBtn');
            btn.disabled = true;
            btn.innerHTML = '⏳ Running...';
            stopBtn.style.display = 'flex';
            
            isRunning = true;
            logOffset = 0;

            try {
                const res = await fetch(`/api/run`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ analysis_type: currentType })
                });
                
                const result = await res.json();
                if (result.status === 'success') {
                    currentJobId = result.job_id;
                    appendLog(`[*] Started job ID: ${currentJobId} for ${currentType}...`);
                    pollInterval = setInterval(pollLogs, 500);
                } else {
                    throw new Error(result.message);
                }
            } catch(e) {
                showToast(e.message || 'Failed to start analysis', true);
                finishRun('failed');
            }
        }
        
        async function pollLogs() {
            if (!currentJobId) return;
            
            try {
                const res = await fetch(`/api/status?job_id=${currentJobId}&offset=${logOffset}`);
                const data = await res.json();
                
                if (data.logs && data.logs.length > 0) {
                    data.logs.forEach(msg => appendLog(msg));
                    logOffset += data.logs.length;
                }
                
                if (data.status !== 'running') {
                    clearInterval(pollInterval);
                    pollInterval = null;
                    finishRun(data.status, data.exit_code);
                }
            } catch (e) {
                console.error("Poll error", e);
            }
        }

        function finishRun(status, exitCode) {
            isRunning = false;
            currentJobId = null;
            
            updateBadge(status);
            const btn = document.getElementById('runBtn');
            const stopBtn = document.getElementById('stopBtn');
            
            btn.disabled = false;
            btn.innerHTML = '🚀 Run Analysis';
            stopBtn.style.display = 'none';
            
            if (status === 'success') {
                showToast("Analysis completed successfully!");
                appendLog(`[*] Analysis finished with exit code ${exitCode}`);
            } else if (exitCode === -9 || exitCode === -15) {
                showToast("Analysis was cancelled by user.", true);
                appendLog(`[!] Analysis manually stopped.`);
            } else {
                showToast("Analysis failed.", true);
                appendLog(`[!] Analysis failed with exit code ${exitCode}`);
            }
        }

        async function stopAnalysis() {
            if (!isRunning || !currentJobId) return;

            const stopBtn = document.getElementById('stopBtn');
            stopBtn.disabled = true;
            stopBtn.innerHTML = '🛑 Stopping...';

            try {
                const res = await fetch(`/api/stop`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ job_id: currentJobId })
                });
                
                const result = await res.json();
                if (result.status !== 'success') {
                    throw new Error(result.message);
                }
            } catch(e) {
                showToast(e.message || 'Failed to stop analysis', true);
                stopBtn.disabled = false;
                stopBtn.innerHTML = '🛑 Stop';
            }
        }

        function showToast(msg, isError = false) {
            const toast = document.getElementById('toast');
            toast.innerText = msg;
            toast.className = 'toast';
            if (isError) toast.classList.add('error');
            
            // Force reflow
            void toast.offsetWidth;
            
            toast.classList.add('show');
            setTimeout(() => {
                toast.classList.remove('show');
            }, 3000);
        }
    </script>
</body>
</html>
"""

# 全局任务字典
# { job_id: {"status": "running|success|failed", "logs": [], "exit_code": None, "process": current_process} }
JOBS: Dict[str, Dict[str, Any]] = {}

def bg_run_process(job_id, exe_path, analysis_type):
    job = JOBS[job_id]
    try:
        ana_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        project_root = os.path.dirname(ana_dir)
        
        # 运行进程并捕获输出
        process = subprocess.Popen(
            [exe_path, analysis_type],
            cwd=project_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1, # 行缓冲
            universal_newlines=True
        )
        job['process'] = process
        
        for line in iter(process.stdout.readline, ''):
            if line:
                job['logs'].append(line.rstrip('\\n'))
                
        process.stdout.close()
        return_code = process.wait()
        
        job['exit_code'] = return_code
        job['status'] = 'success' if return_code == 0 else 'failed'
        
    except Exception as e:
        job['logs'].append(f"Error running process: {e}")
        job['exit_code'] = -1
        job['status'] = 'failed'

class AnalysisUIHandler(http.server.SimpleHTTPRequestHandler):
    exe_path = None

    def do_GET(self):
        parsed_path = urllib.parse.urlparse(self.path)
        
        if parsed_path.path == '/':
            content = HTML_CONTENT.encode('utf-8')
            self.send_response(200)
            self.send_header("Content-type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
            
        elif parsed_path.path == '/api/analysis-types':
            content = json.dumps(ANALYSIS_GROUPS).encode()
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
            
        elif parsed_path.path == '/api/status':
            query = urllib.parse.parse_qs(parsed_path.query)
            job_id = query.get('job_id', [''])[0]
            offset_str = query.get('offset', ['0'])[0]
            
            try:
                offset = int(offset_str)
            except ValueError:
                offset = 0
                
            if not job_id or job_id not in JOBS:
                self.send_error(404, "Job not found")
                return
                
            job = JOBS[job_id]
            logs = job['logs'][offset:]
            
            response_data = {
                "status": job['status'],
                "exit_code": job['exit_code'],
                "logs": logs,
                "next_offset": offset + len(logs)
            }
            
            content = json.dumps(response_data).encode()
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
                
        else:
            self.send_error(404)

    def do_POST(self):
        parsed_path = urllib.parse.urlparse(self.path)
        
        if parsed_path.path == '/api/run':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            
            try:
                json_data = json.loads(post_data.decode('utf-8'))
                analysis_type = json_data.get('analysis_type')
                
                if not analysis_type:
                    raise ValueError("analysis_type missing")
                
                if not AnalysisUIHandler.exe_path:
                    # 尝试重新寻找
                    AnalysisUIHandler.exe_path = check_executable()
                    if not AnalysisUIHandler.exe_path:
                        raise ValueError("rdataframe_analysis executable not found.")
                        
                job_id = str(uuid.uuid4())
                JOBS[job_id] = {
                    "status": "running",
                    "logs": [],
                    "exit_code": None,
                    "process": None
                }
                
                # 开始在后台线程运行
                t = threading.Thread(target=bg_run_process, args=(job_id, AnalysisUIHandler.exe_path, analysis_type))
                t.daemon = True
                t.start()
                
                response_data = {"status": "success", "job_id": job_id}
                content = json.dumps(response_data).encode()
                
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.send_header("Content-Length", str(len(content)))
                self.end_headers()
                self.wfile.write(content)
                
            except Exception as e:
                content = json.dumps({"status": "error", "message": str(e)}).encode()
                self.send_response(500)
                self.send_header("Content-Length", str(len(content)))
                self.end_headers()
                self.wfile.write(content)
                
        elif parsed_path.path == '/api/stop':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            
            try:
                json_data = json.loads(post_data.decode('utf-8'))
                job_id = json_data.get('job_id')
                
                if not job_id or job_id not in JOBS:
                    raise ValueError("job_id missing or invalid")
                
                job = JOBS[job_id]
                if job['status'] == 'running' and job['process']:
                    job['process'].terminate()
                    job['logs'].append("[-] Received terminate signal...")
                
                content = json.dumps({"status": "success"}).encode()
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.send_header("Content-Length", str(len(content)))
                self.end_headers()
                self.wfile.write(content)
                
            except Exception as e:
                content = json.dumps({"status": "error", "message": str(e)}).encode()
                self.send_response(500)
                self.send_header("Content-Length", str(len(content)))
                self.end_headers()
                self.wfile.write(content)
        else:
            self.send_error(404)

def run_server(port, exe_path):
    AnalysisUIHandler.exe_path = exe_path
    
    # 允许重用端口
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", port), AnalysisUIHandler) as httpd:
        print(f"[*] Starting Analysis UI on http://localhost:{port}")
        if exe_path:
            print(f"[*] Executable path: {exe_path}")
        else:
            print("[!] Executable not found. It will be searched on first run.")
        print("[*] Press Ctrl+C to stop.")
        
        # 打开 prepare_output_directories
        try:
            ana_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            project_root = os.path.dirname(ana_dir)
            run_analysis.prepare_output_directories(project_root)
        except Exception as e:
            print(f"Failed to prepare output directories: {e}")
            
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[*] Stopping server.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Web UI for RDataFrame Analysis")
    parser.add_argument("--port", type=int, default=8081, help="Port to run the UI server on")
    parser.add_argument("--executable", type=str, help="Path to rdataframe_analysis executable")
    
    args = parser.parse_args()
    
    exe = args.executable
    if not exe:
        exe = check_executable()
        
    run_server(args.port, exe)
