# Extensões VS Code para All Press C++

## 🔧 Essenciais para C++

### **C/C++ Development**
```json
{
  "recommendations": [
    "ms-vscode.cpptools-extension-pack",
    "ms-vscode.cpptools",
    "ms-vscode.cmake-tools",
    "twxs.cmake",
    "ms-vscode.makefile-tools"
  ]
}
```

**ms-vscode.cpptools-extension-pack**
- IntelliSense, debugging, code browsing
- Essencial para desenvolvimento C++

**ms-vscode.cmake-tools**
- Integração completa com CMake
- Build, debug, teste diretamente do VS Code

**twxs.cmake**
- Syntax highlighting para CMakeLists.txt
- Auto-complete para comandos CMake

## 🏗️ Build & Dependencies

### **Package Management**
```json
{
  "recommendations": [
    "conan-io.conan",
    "ms-vscode.vscode-vcpkg"
  ]
}
```

**conan-io.conan**
- Integração com Conan package manager
- Gestão de dependências C++

**ms-vscode.vscode-vcpkg**
- Alternativa para vcpkg se preferir

## 🐛 Debugging & Testing

### **Debug Tools**
```json
{
  "recommendations": [
    "vadimcn.vscode-lldb",
    "webfreak.debug",
    "ms-vscode.test-adapter-converter",
    "matepek.vscode-catch2-test-adapter"
  ]
}
```

**vadimcn.vscode-lldb**
- Debugger LLDB para C++ (superior ao GDB)
- Melhor para macOS e Linux moderno

**matepek.vscode-catch2-test-adapter**
- Integração com framework de testes Catch2
- Test explorer integrado

## 📝 Code Quality & Formatting

### **Linting & Formatting**
```json
{
  "recommendations": [
    "xaver.clang-format",
    "notskm.clang-tidy",
    "cschlosser.doxdocgen",
    "jeff-hykin.better-cpp-syntax"
  ]
}
```

**xaver.clang-format**
- Formatação automática de código
- Essencial para manter consistência

**notskm.clang-tidy**
- Static analysis para C++
- Detecta bugs e code smells

**cschlosser.doxdocgen**
- Geração automática de documentação Doxygen
- Templates para comentários

## 🌐 Web & API Development

### **REST & WebSocket**
```json
{
  "recommendations": [
    "humao.rest-client",
    "rangav.vscode-thunder-client",
    "42crunch.vscode-openapi"
  ]
}
```

**humao.rest-client**
- Testar APIs REST diretamente do VS Code
- Essencial para testar endpoints C++

**rangav.vscode-thunder-client**
- Alternativa moderna ao Postman
- Interface mais limpa

## 📊 Database & JSON

### **Data Management**
```json
{
  "recommendations": [
    "alexcvzz.vscode-sqlite",
    "qwtel.sqlite-viewer",
    "vscode-icons-team.vscode-icons"
  ]
}
```

**alexcvzz.vscode-sqlite**
- Visualizar e editar bancos SQLite
- Essencial para debug do banco

## 🔍 Code Navigation

### **Enhanced Navigation**
```json
{
  "recommendations": [
    "christian-kohler.path-intellisense",
    "formulahendry.code-runner",
    "gruntfuggly.todo-tree",
    "streetsidesoftware.code-spell-checker"
  ]
}
```

**christian-kohler.path-intellisense**
- Auto-complete para caminhos de arquivo
- Útil para includes C++

**formulahendry.code-runner**
- Executar código rapidamente
- Útil para testes rápidos

## 🎨 UI & Frontend (Para React)

### **React Development**
```json
{
  "recommendations": [
    "bradlc.vscode-tailwindcss",
    "esbenp.prettier-vscode",
    "ms-vscode.vscode-typescript-next",
    "ms-vscode.vscode-eslint"
  ]
}
```

**bradlc.vscode-tailwindcss**
- IntelliSense para Tailwind CSS
- Para o frontend React

## 🔧 DevOps & Deployment

### **Docker & Git**
```json
{
  "recommendations": [
    "ms-azuretools.vscode-docker",
    "eamodio.gitlens",
    "github.vscode-pull-request-github",
    "redhat.vscode-yaml"
  ]
}
```

**ms-azuretools.vscode-docker**
- Gestão completa de containers
- Build, run, debug containers

**eamodio.gitlens**
- Git supercharged
- História, blame, comparações

## 📈 Performance & Monitoring

### **Profiling & Analysis**
```json
{
  "recommendations": [
    "ms-vscode.hexdump",
    "austin.code-gnu-global",
    "llvm-vs-code-extensions.vscode-clangd"
  ]
}
```

**llvm-vs-code-extensions.vscode-clangd**
- Language server alternativo (mais rápido que cpptools)
- Melhor IntelliSense para projetos grandes

## 🎯 Extensões Específicas para All Press

### **Print & Graphics**
```json
{
  "recommendations": [
    "ms-vscode.live-server",
    "ms-vscode.vscode-json",
    "redhat.vscode-xml",
    "dotjoshjohnson.xml"
  ]
}
```

**ms-vscode.live-server**
- Preview de páginas web
- Útil para testar interface

**ms-vscode.vscode-json**
- Melhor suporte a JSON
- Para configurações e APIs

## 📋 Configuração Completa

### **settings.json recomendado**
```json
{
  "C_Cpp.default.cppStandard": "c++17",
  "C_Cpp.default.compilerPath": "/usr/bin/g++",
  "C_Cpp.default.intelliSenseMode": "gcc-x64",
  "cmake.configureOnOpen": true,
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.generator": "Ninja",
  "files.associations": {
    "*.hpp": "cpp",
    "*.tpp": "cpp",
    "CMakeLists.txt": "cmake"
  },
  "editor.formatOnSave": true,
  "C_Cpp.clang_format_style": "Google",
  "cmake.debugConfig": {
    "args": ["--debug", "--port", "8000"]
  },
  "git.enableSmartCommit": true,
  "terminal.integrated.defaultProfile.linux": "bash",
  "rest-client.environmentVariables": {
    "local": {
      "baseUrl": "http://localhost:8000",
      "wsUrl": "ws://localhost:8001"
    }
  }
}
```

### **launch.json para debugging**
```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug All Press Server",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/all_press_server",
      "args": ["--debug", "--config", "config/debug.conf"],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ],
      "preLaunchTask": "cmake: build"
    }
  ]
}
```

### **tasks.json para build tasks**
```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "cmake: configure",
      "type": "shell",
      "command": "cmake",
      "args": ["-S", ".", "-B", "build", "-G", "Ninja"],
      "group": "build",
      "options": {
        "cwd": "${workspaceFolder}"
      }
    },
    {
      "label": "cmake: build",
      "type": "shell",
      "command": "cmake",
      "args": ["--build", "build"],
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "dependsOn": "cmake: configure"
    },
    {
      "label": "conan: install",
      "type": "shell",
      "command": "conan",
      "args": ["install", ".", "--build=missing", "--install-folder=build"],
      "group": "build",
      "options": {
        "cwd": "${workspaceFolder}"
      }
    }
  ]
}
```

## 📦 Instalação Rápida

**Via Command Palette:**
```bash
# Instalar todas as extensões essenciais de uma vez
code --install-extension ms-vscode.cpptools-extension-pack
code --install-extension ms-vscode.cmake-tools
code --install-extension conan-io.conan
code --install-extension vadimcn.vscode-lldb
code --install-extension xaver.clang-format
code --install-extension humao.rest-client
code --install-extension alexcvzz.vscode-sqlite
code --install-extension eamodio.gitlens
```

**Ou criar workspace:**
```json
// .vscode/extensions.json
{
  "recommendations": [
    "ms-vscode.cpptools-extension-pack",
    "ms-vscode.cmake-tools",
    "conan-io.conan",
    "vadimcn.vscode-lldb",
    "xaver.clang-format",
    "notskm.clang-tidy",
    "humao.rest-client",
    "alexcvzz.vscode-sqlite",
    "eamodio.gitlens",
    "ms-azuretools.vscode-docker"
  ]
}
```

Essas extensões fornecerão um ambiente de desenvolvimento C++ completo e profissional para o projeto All Press, com suporte desde o desenvolvimento até deployment.