FROM mcr.microsoft.com/devcontainers/rust:2-1-trixie

RUN rustup target add x86_64-pc-windows-gnu

RUN apt-get update && apt-get install -y --no-install-recommends \
    autopoint \
    bison \
    flex \
    g++-mingw-w64-x86-64 \
    gettext \
    gperf \
    intltool \
    libgl-dev \
    libtool-bin \
    lzip \
    p7zip-full \
    python3-mako \
    python3-setuptools \
    python-is-python3 \
    ruby \
    wine \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN git clone https://github.com/mxe/mxe.git \
    && cd mxe \
    && git checkout 6da7648ef4bf058e6c2719cd4094a498a340dbc7 \
    && make MXE_TARGETS=x86_64-w64-mingw32.static boost qt6-qtbase --jobs=2 JOBS=8

USER vscode
WORKDIR /home/vscode

RUN curl --proto '=https' --tlsv1.2 -sSfLO https://releases.astral.sh/github/uv/releases/download/0.13.5/uv-x86_64-unknown-linux-gnu.tar.gz \
    && echo "f68d291c75783122dad58ab77def0e13f3233f358731db5d6d7e8715603b928f  uv-x86_64-unknown-linux-gnu.tar.gz" | sha256sum -c \
    && mkdir -p /home/vscode/.local/bin \
    && tar -xzf uv-x86_64-unknown-linux-gnu.tar.gz -C /home/vscode/.local/bin --strip-components=1 \
    && rm uv-x86_64-unknown-linux-gnu.tar.gz

RUN curl --proto '=https' --tlsv1.2 -sSfLO https://github.com/Ortham/svg_to_ico/releases/download/1.3.1/svg_to_ico.tar.xz \
    && echo "04897b6384e7c3141c8343335b66bde3b184d7d65940980c38689b3ef4453f2f svg_to_ico.tar.xz" \
    && tar xJf svg_to_ico.tar.xz -C /home/vscode/.local/bin \
    && rm svg_to_ico.tar.xz

# Fix GetPreferredUILanguages.shouldReturnAtLeastOneLanguage() test
ENV LC_ALL=en_US.utf8 \
    WINEPATH="/usr/lib/gcc/x86_64-w64-mingw32/14-win32/" \
    PATH="$PATH:/opt/mxe/usr/bin"
