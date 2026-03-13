import http.server
import socketserver
import json
import os
import urllib.parse
import argparse
import sys

# HTML template with embedded CSS and JS
HTML_CONTENT = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>JSON Configuration Editor</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
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
            --border: #334155;
            --input-bg: #0f172a;
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
            width: 280px;
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
        }

        .file-list {
            flex: 1;
            overflow-y: auto;
            padding: 10px 0;
        }

        .file-item {
            padding: 12px 20px;
            cursor: pointer;
            transition: all 0.2s;
            color: var(--text-muted);
            font-size: 0.95rem;
            display: flex;
            align-items: center;
        }
        
        .file-item::before {
            content: '📄';
            margin-right: 10px;
            font-size: 1.1em;
            opacity: 0.7;
        }

        .file-item:hover {
            background-color: rgba(255, 255, 255, 0.05);
            color: var(--text-main);
        }

        .file-item.active {
            background-color: var(--bg-card);
            color: var(--accent);
            border-right: 3px solid var(--accent);
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
        }

        .filename-display {
            font-size: 1.2rem;
            font-weight: 500;
            color: var(--text-main);
        }

        .btn-save {
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
        }

        .btn-save:hover {
            background-color: var(--accent-hover);
            transform: translateY(-1px);
            box-shadow: 0 6px 16px rgba(45, 212, 191, 0.3);
        }
        
        .btn-action {
            background-color: transparent;
            color: var(--text-muted);
            border: 1px solid var(--border);
            padding: 4px 8px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 0.8rem;
            margin-left: 8px;
            transition: all 0.2s;
        }
        
        .btn-action:hover {
            border-color: var(--accent);
            color: var(--accent);
        }

        .btn-toggle-edit {
            background-color: transparent;
            color: var(--text-muted);
            border: 1px dashed var(--border);
            padding: 8px 16px;
            border-radius: 6px;
            font-size: 0.85rem;
            cursor: pointer;
            transition: all 0.2s;
            margin-right: 16px;
        }
        .btn-toggle-edit.active {
            background-color: rgba(244, 63, 94, 0.1);
            color: var(--danger);
            border-color: var(--danger);
        }

        .editor-container {
            flex: 1;
            overflow-y: auto;
            padding: 30px;
        }

        /* JSON Form Styles */
        .json-object {
            margin-left: 12px;
            border-left: 1px dashed var(--border);
            padding-left: 16px;
            margin-bottom: 8px;
        }

        .json-array {
            margin-left: 12px;
            border-left: 1px dashed var(--border);
            padding-left: 16px;
            margin-bottom: 8px;
        }

        .json-row {
            display: flex;
            align-items: flex-start;
            margin-bottom: 12px;
            animation: fadeIn 0.3s ease-in-out;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(4px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .json-key {
            min-width: 160px;
            color: #60a5fa;
            font-weight: 500;
            font-size: 0.95rem;
            padding-top: 8px;
        }

        .json-value {
            flex: 1;
            min-width: 150px;
            max-width: 600px;
        }

        .input-base {
            width: 100%;
            min-width: 80px;
            background-color: var(--input-bg);
            border: 1px solid var(--border);
            color: var(--text-main);
            padding: 8px 12px;
            border-radius: 6px;
            font-size: 0.95rem;
            transition: all 0.2s;
        }

        .input-base:focus {
            outline: none;
            border-color: var(--accent);
            box-shadow: 0 0 0 2px rgba(45, 212, 191, 0.1);
        }
        
        .type-string { color: #facc15; }
        .type-number { color: #f87171; }
        .type-boolean { color: #c084fc; }

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
        .toast.show {
            transform: translateX(0);
        }

        /* Scrollbar */
        ::-webkit-scrollbar {
            width: 8px;
            height: 8px;
        }
        ::-webkit-scrollbar-track {
            background: rgba(0,0,0,0.1);
        }
        ::-webkit-scrollbar-thumb {
            background: var(--border);
            border-radius: 4px;
        }
        ::-webkit-scrollbar-thumb:hover {
            background: #475569;
        }
        
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
        
        /* Collapsible Support */
        .collapsible-header {
            cursor: pointer;
            display: flex;
            align-items: center;
            margin-bottom: 8px;
            user-select: none;
        }
        .collapsible-header:hover {
            color: var(--text-main);
        }
        .chevron {
            display: inline-block;
            transition: transform 0.2s;
            margin-right: 6px;
            font-size: 0.8em;
            color: var(--text-muted);
        }
        .chevron.open {
            transform: rotate(90deg);
        }
    </style>
</head>
<body>

    <div class="sidebar">
        <div class="sidebar-header">
            Nexus Config
        </div>
        <div class="file-list" id="fileList">
            <!-- Files loaded dynamically -->
        </div>
    </div>

    <div class="main">
        <div class="topbar">
            <div class="filename-display" id="filenameDisplay">Select a file</div>
            <div style="display: flex; align-items: center;">
                <button class="btn-toggle-edit" id="toggleEditBtn" style="display: none;">⚙️ Advanced Edit</button>
                <button class="btn-save" id="saveBtn" style="display: none;">Save Changes</button>
            </div>
        </div>
        <div class="editor-container" id="editorContainer">
            <div class="empty-state">
                <div class="empty-icon">📂</div>
                <h2>No File Selected</h2>
                <p>Choose a JSON configuration file from the sidebar to edit.</p>
            </div>
        </div>
    </div>

    <div class="toast" id="toast">File saved successfully!</div>

    <script>
        let currentFile = null;
        let jsonData = null;
        let advancedEditMode = false;

        document.addEventListener('DOMContentLoaded', () => {
            fetchFileList();

            document.getElementById('saveBtn').addEventListener('click', saveFile);
            
            document.getElementById('toggleEditBtn').addEventListener('click', () => {
                advancedEditMode = !advancedEditMode;
                const btn = document.getElementById('toggleEditBtn');
                if(advancedEditMode) {
                    btn.classList.add('active');
                    btn.innerText = '⚙️ Editing Structure...';
                } else {
                    btn.classList.remove('active');
                    btn.innerText = '⚙️ Advanced Edit';
                }
                if(currentFile) renderEditor();
            });
        });

        async function fetchFileList() {
            try {
                const res = await fetch('/api/files');
                const data = await res.json();
                const list = document.getElementById('fileList');
                list.innerHTML = '';
                
                data.files.forEach(f => {
                    const div = document.createElement('div');
                    div.className = 'file-item';
                    div.textContent = f;
                    div.onclick = () => loadFile(f);
                    if (f === currentFile) div.classList.add('active');
                    list.appendChild(div);
                });
            } catch (e) {
                console.error("Failed to load files", e);
            }
        }

        async function loadFile(filename) {
            currentFile = filename;
            document.getElementById('filenameDisplay').textContent = filename;
            document.getElementById('saveBtn').style.display = 'block';
            document.getElementById('toggleEditBtn').style.display = 'block';
            
            // update active state in sidebar
            document.querySelectorAll('.file-item').forEach(el => {
                if(el.textContent === filename) el.classList.add('active');
                else el.classList.remove('active');
            });

            try {
                const res = await fetch(`/api/file?name=${encodeURIComponent(filename)}`);
                jsonData = await res.json();
                renderEditor();
            } catch (e) {
                console.error("Failed to load file content", e);
            }
        }

        function renderEditor() {
            const container = document.getElementById('editorContainer');
            container.innerHTML = '';
            
            const rootWrapper = document.createElement('div');
            buildTree(jsonData, rootWrapper, []);
            container.appendChild(rootWrapper);
        }

        /**
         * Recursively builds the JSON configuration tree UI.
         */
        function buildTree(data, parentElement, path) {
            if (Array.isArray(data)) {
                const arrContainer = document.createElement('div');
                arrContainer.className = 'json-array';
                
                data.forEach((item, index) => {
                    const row = document.createElement('div');
                    row.className = 'json-row';
                    
                    const keyLabel = document.createElement('div');
                    keyLabel.className = 'json-key';
                    keyLabel.innerText = `[${index}]`;
                    
                    const valContainer = document.createElement('div');
                    valContainer.className = 'json-value';
                    
                    if (item !== null && typeof item === 'object') {
                        // Display array item as an object
                        const objContainer = document.createElement('div');
                        objContainer.className = 'json-object';
                        buildTree(item, objContainer, [...path, index]);
                        valContainer.appendChild(objContainer);
                    } else {
                        buildTree(item, valContainer, [...path, index]);
                    }
                    
                    // Allow deleting array items
                    row.appendChild(keyLabel);
                    row.appendChild(valContainer);
                    if (advancedEditMode) {
                        const delBtn = document.createElement('button');
                        delBtn.className = 'btn-action';
                        delBtn.innerText = 'Del';
                        delBtn.onclick = () => {
                            updateDataByPath([...path], currentArr => {
                                currentArr.splice(index, 1);
                                return currentArr;
                            });
                            renderEditor(); // re-render
                        };
                        row.appendChild(delBtn);
                    }
                    
                    arrContainer.appendChild(row);
                });
                
                // Add item button
                if (advancedEditMode) {
                    const addBtn = document.createElement('button');
                    addBtn.className = 'btn-action';
                    addBtn.style.marginTop = '8px';
                    addBtn.innerText = '+ Add Item';
                    addBtn.onclick = () => {
                        updateDataByPath([...path], currentArr => {
                            let newItem = "";
                            if (currentArr.length > 0) {
                                // Copy structure of first item
                                newItem = JSON.parse(JSON.stringify(currentArr[0]));
                                clearValues(newItem);
                            }
                            currentArr.push(newItem);
                            return currentArr;
                        });
                        renderEditor();
                    };
                    arrContainer.appendChild(addBtn);
                }
                parentElement.appendChild(arrContainer);

            } else if (data !== null && typeof data === 'object') {
                const objContainer = document.createElement('div');
                objContainer.className = 'json-object';
                
                const keys = Object.keys(data);
                keys.forEach(key => {
                    const row = document.createElement('div');
                    row.className = 'json-row';
                    
                    const isComplex = (typeof data[key] === 'object' && data[key] !== null);
                    
                    const keyLabelContainer = document.createElement('div');
                    keyLabelContainer.className = 'json-key';
                    
                    if (isComplex) {
                        const collHeader = document.createElement('div');
                        collHeader.className = 'collapsible-header';
                        collHeader.innerHTML = `<span class="chevron open">▶</span> ${key}`;
                        
                        const valContainer = document.createElement('div');
                        valContainer.className = 'json-value';
                        
                        collHeader.onclick = () => {
                            const chevron = collHeader.querySelector('.chevron');
                            chevron.classList.toggle('open');
                            valContainer.style.display = valContainer.style.display === 'none' ? 'block' : 'none';
                        };
                        
                        keyLabelContainer.appendChild(collHeader);
                        row.appendChild(keyLabelContainer);
                        
                        buildTree(data[key], valContainer, [...path, key]);
                        row.appendChild(valContainer);
                    } else {
                        keyLabelContainer.innerText = key;
                        row.appendChild(keyLabelContainer);
                        
                        const valContainer = document.createElement('div');
                        valContainer.className = 'json-value';
                        buildTree(data[key], valContainer, [...path, key]);
                        row.appendChild(valContainer);
                    }
                    
                    objContainer.appendChild(row);
                });
                
                // --- Add Property button for Objects ---
                if (advancedEditMode) {
                    const addPropBtn = document.createElement('button');
                    addPropBtn.className = 'btn-action';
                    addPropBtn.style.marginTop = '8px';
                    addPropBtn.innerText = '+ Add Property';
                    addPropBtn.onclick = () => {
                        const newKey = prompt('Enter name for the new property:');
                        if (newKey && newKey.trim() !== '') {
                            updateDataByPath([...path], currentObj => {
                                if (currentObj.hasOwnProperty(newKey)) {
                                    alert(`Property "${newKey}" already exists!`);
                                    return currentObj;
                                }
                                // To determine what to add, we can try to copy the first existing value's structure,
                                // or default to an empty string if it's empty.
                                let newPropValue = "";
                                const existingKeys = Object.keys(currentObj);
                                if (existingKeys.length > 0) {
                                    const templateKey = existingKeys[existingKeys.length - 1]; // Use last as template
                                    newPropValue = JSON.parse(JSON.stringify(currentObj[templateKey]));
                                    clearValues(newPropValue);
                                }
                                
                                currentObj[newKey] = newPropValue;
                                return currentObj;
                            });
                            renderEditor(); // re-render
                        }
                    };
                    
                    // Allow deleting object property if it's not the root object
                    if (path.length > 0) {
                      const delPropWrapper = document.createElement('div');
                      delPropWrapper.style.marginTop = '8px';
                      delPropWrapper.appendChild(addPropBtn);
                      
                      const delObjBtn = document.createElement('button');
                      delObjBtn.className = 'btn-action';
                      delObjBtn.innerText = 'Del Entire Object';
                      delObjBtn.onclick = () => {
                          if(confirm(`Delete object ${path[path.length-1]}?`)){
                              const parentPath = path.slice(0, -1);
                              const keyToDelete = path[path.length - 1];
                              updateDataByPath(parentPath, parentObj => {
                                  delete parentObj[keyToDelete];
                                  return parentObj;
                              });
                              renderEditor();
                          }
                      };
                      delPropWrapper.appendChild(delObjBtn);
                      parentElement.appendChild(objContainer);
                      parentElement.appendChild(delPropWrapper);
                    } else {
                      parentElement.appendChild(objContainer);
                      parentElement.appendChild(addPropBtn);
                    }
                } else {
                    parentElement.appendChild(objContainer);
                }
                
            } else {
                // Primitive value (string, number, boolean)
                const input = document.createElement('input');
                input.className = 'input-base';
                
                const currentKey = path.length > 0 ? path[path.length - 1] : '';
                
                if (typeof data === 'number') {
                    input.type = 'number';
                    input.step = 'any';
                    input.classList.add('type-number');
                } else if (typeof data === 'boolean') {
                    input.type = 'checkbox';
                    input.checked = data;
                    input.style.width = 'auto';
                } else {
                    input.type = 'text';
                    input.classList.add('type-string');
                }
                
                if (typeof data !== 'boolean') {
                    input.value = data;
                    input.placeholder = `[${data}]`;
                }
                
                if (currentKey === 'unit') {
                    input.disabled = true;
                    input.style.opacity = '0.5';
                    input.style.cursor = 'not-allowed';
                    input.title = 'Unit cannot be modified';
                }
                
                input.onchange = (e) => {
                    let val;
                    if (input.type === 'number') val = Number(e.target.value);
                    else if (input.type === 'checkbox') val = e.target.checked;
                    else val = e.target.value;
                    
                    updateDataByPath([...path], () => val);
                };
                
                parentElement.appendChild(input);
            }
        }

        // Helper to clear values when adding a new array item
        function clearValues(obj) {
            if (Array.isArray(obj)) {
                obj.length = 0; // clear array
            } else if (obj !== null && typeof obj === 'object') {
                for (const k in obj) {
                    if (typeof obj[k] === 'object') clearValues(obj[k]);
                    else if (typeof obj[k] === 'number') obj[k] = 0;
                    else if (typeof obj[k] === 'boolean') obj[k] = false;
                    else obj[k] = "";
                }
            }
            return obj;
        }

        // Update the global jsonData object based on path
        function updateDataByPath(path, updater) {
            if (path.length === 0) {
                jsonData = updater(jsonData);
                return;
            }
            let current = jsonData;
            for (let i = 0; i < path.length - 1; i++) {
                current = current[path[i]];
            }
            const lastKey = path[path.length - 1];
            current[lastKey] = updater(current[lastKey]);
        }

        async function saveFile() {
            if (!currentFile || !jsonData) return;
            
            const btn = document.getElementById('saveBtn');
            const originalText = btn.innerText;
            btn.innerText = 'Saving...';
            btn.disabled = true;

            try {
                const res = await fetch(`/api/file?name=${encodeURIComponent(currentFile)}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(jsonData, null, 2)
                });
                
                const result = await res.json();
                if (result.status === 'success') {
                    showToast('Configuration saved successfully!');
                } else {
                    alert('Error saving: ' + result.message);
                }
            } catch(e) {
                alert('Connection error');
                console.error(e);
            } finally {
                btn.innerText = originalText;
                btn.disabled = false;
            }
        }

        function showToast(msg) {
            const toast = document.getElementById('toast');
            toast.innerText = msg;
            toast.classList.add('show');
            setTimeout(() => {
                toast.classList.remove('show');
            }, 3000);
        }
    </script>
</body>
</html>
"""

class JSONEditorHandler(http.server.SimpleHTTPRequestHandler):
    config_dir = "."

    def do_GET(self):
        parsed_path = urllib.parse.urlparse(self.path)
        
        if parsed_path.path == '/':
            content = HTML_CONTENT.encode('utf-8')
            self.send_response(200)
            self.send_header("Content-type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
            
        elif parsed_path.path == '/api/files':
            files = [f for f in os.listdir(self.config_dir) if f.endswith('.json')]
            files.sort()
            
            content = json.dumps({"files": files}).encode()
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
            
        elif parsed_path.path == '/api/file':
            query = urllib.parse.parse_qs(parsed_path.query)
            filename = query.get('name', [''])[0]
            
            if not filename or not filename.endswith('.json'):
                self.send_error(400, "Invalid filename")
                return
                
            filepath = os.path.join(self.config_dir, filename)
            if not os.path.exists(filepath):
                self.send_error(404, "File not found")
                return
                
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    content = f.read().encode('utf-8')
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.send_header("Content-Length", str(len(content)))
                self.end_headers()
                self.wfile.write(content)
            except Exception as e:
                self.send_error(500, str(e))
                
        else:
            self.send_error(404)

    def do_POST(self):
        parsed_path = urllib.parse.urlparse(self.path)
        
        if parsed_path.path == '/api/file':
            query = urllib.parse.parse_qs(parsed_path.query)
            filename = query.get('name', [''])[0]
            
            if not filename or not filename.endswith('.json') or '..' in filename or '/' in filename:
                self.send_response(400)
                self.end_headers()
                self.wfile.write(json.dumps({"status": "error", "message": "Invalid filename"}).encode())
                return
                
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)
            
            filepath = os.path.join(self.config_dir, filename)
            
            try:
                # Verify it's valid JSON
                json_data = json.loads(post_data.decode('utf-8'))
                
                # Write with nice formatting
                json_str = json.dumps(json_data, indent=2, ensure_ascii=False)
                # Compact primitive lists (numbers, strings)
                import re
                json_str = re.sub(r'\[([\d\s.,\-]+)\]', lambda m: '[' + ', '.join(x.strip() for x in m.group(1).split(',')) + ']', json_str)
                
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(json_str)
                    
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


def run_server(port, directory):
    # Ensure directory exists
    if not os.path.exists(directory):
        print(f"Error: Directory '{directory}' does not exist.")
        sys.exit(1)
        
    JSONEditorHandler.config_dir = directory
    
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", port), JSONEditorHandler) as httpd:
        print(f"[*] Starting JSON Editor UI on http://localhost:{port}")
        print(f"[*] Config directory: {os.path.abspath(directory)}")
        print("[*] Press Ctrl+C to stop.")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[*] Stopping server.")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Local Web JSON Editor")
    parser.add_argument("--port", type=int, default=8080, help="Port to run the server on")
    parser.add_argument("--dir", type=str, default="../config", help="Directory containing JSON config files")
    
    args = parser.parse_args()
    
    # Defaults directory to ../config assuming script is in ana/scripts/
    target_dir = os.path.abspath(args.dir)
    run_server(args.port, target_dir)
