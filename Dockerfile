FROM ubuntu:22.04

# Evitar prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependências do sistema
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    python3 \
    python3-pip \
    libcups2-dev \
    libcupsimage2-dev \
    libmagick++-dev \
    libpoppler-cpp-dev \
    libsqlite3-dev \
    libboost-all-dev \
    libssl-dev \
    ghostscript \
    && rm -rf /var/lib/apt/lists/*

# Instalar Conan
RUN pip3 install conan

# Criar diretório da aplicação
WORKDIR /app

# Copiar arquivos do projeto
COPY . .

# Configurar Conan
RUN conan profile detect --force

# Instalar dependências do Conan
RUN conan install . --output-folder=build --build=missing -s build_type=Release

# Build da aplicação
WORKDIR /app/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
RUN cmake --build . -j$(nproc)

# Criar diretórios necessários
RUN mkdir -p /app/logs /app/uploads /app/output

# Expor portas
EXPOSE 8080 8081

# Volume para persistência
VOLUME ["/app/logs", "/app/uploads", "/app/output", "/app/config"]

# Comando de inicialização
CMD ["./all_press_c"]
