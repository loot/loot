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

RUN curl --proto '=https' --tlsv1.2 -sSfLO https://github.com/astral-sh/uv/releases/download/0.9.28/uv-x86_64-unknown-linux-gnu.tar.gz \
    && echo "66ad1822dd9cf96694b95c24f25bc05cff417a65351464da01682a91796d1f2b uv-x86_64-unknown-linux-gnu.tar.gz" | sha256sum -c \
    && mkdir -p /home/vscode/.local/bin \
    && tar -xzf uv-x86_64-unknown-linux-gnu.tar.gz -C /home/vscode/.local/bin --strip-components=1 \
    && rm uv-x86_64-unknown-linux-gnu.tar.gz

# Fix GetPreferredUILanguages.shouldReturnAtLeastOneLanguage() test
ENV LC_ALL=en_US.utf8
