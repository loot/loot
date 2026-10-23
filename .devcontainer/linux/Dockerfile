FROM mcr.microsoft.com/devcontainers/rust:2-1-trixie

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    gettext \
    libboost-locale-dev \
    libicu-dev \
    libtbb-dev \
    qt6-base-dev \
    && rm -rf /var/lib/apt/lists/*

USER vscode
WORKDIR /home/vscode

RUN curl --proto '=https' --tlsv1.2 -sSfLO https://releases.astral.sh/github/uv/releases/download/0.13.5/uv-x86_64-unknown-linux-gnu.tar.gz \
    && echo "f68d291c75783122dad58ab77def0e13f3233f358731db5d6d7e8715603b928f  uv-x86_64-unknown-linux-gnu.tar.gz" | sha256sum -c \
    && mkdir -p /home/vscode/.local/bin \
    && tar -xzf uv-x86_64-unknown-linux-gnu.tar.gz -C /home/vscode/.local/bin --strip-components=1 \
    && rm uv-x86_64-unknown-linux-gnu.tar.gz

# Fix GetPreferredUILanguages.shouldReturnAtLeastOneLanguage() test
ENV LC_ALL=en_US.utf8
