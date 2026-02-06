FROM ghcr.io/ortham/dev-rust-mingw-gcc:master

USER root

RUN apt-get update && apt-get install -y --no-install-recommends \
    gettext \
    p7zip-full \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN curl -sSLO https://archives.boost.io/release/1.90.0/source/boost_1_90_0.zip \
    && unzip boost_1_90_0.zip \
    && cd boost_1_90_0 \
    && echo "using gcc : mingw : x86_64-w64-mingw32-g++ ;" > user-config.jam \
    && ./bootstrap.sh \
    && ./b2 install --prefix=/usr/x86_64-w64-mingw32 --user-config=./user-config.jam toolset=gcc-mingw link=static runtime-link=shared variant=release address-model=64 target-os=windows cxxflags="-std=c++17 -fPIC" --with-locale \
    && cd .. \
    && rm -rf boost_1_90_0 boost_1_90_0.zip

RUN curl -sSfLO https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz \
    && echo "ab5a03176ee106d3f0fa90e381da478ddae405918153cca248e682cd0c4a2269 bzip2-1.0.8.tar.gz" | sha256sum -c \
    && tar -xf bzip2-1.0.8.tar.gz \
    && cd bzip2-1.0.8 \
    && make CC=x86_64-w64-mingw32-gcc libbz2.a \
    && export PREFIX=/usr/x86_64-w64-mingw32 \
    && cp -f bzlib.h $PREFIX/include \
    && cp -f libbz2.a $PREFIX/lib \
    && chmod a+r $PREFIX/include/bzlib.h $PREFIX/lib/libbz2.a \
    && cd .. \
    && rm -rf bzip2-1.0.8 bzip2-1.0.8.tar.gz

USER vscode
WORKDIR /home/vscode

RUN curl --proto '=https' --tlsv1.2 -sSfLO https://github.com/astral-sh/uv/releases/download/0.9.28/uv-x86_64-unknown-linux-gnu.tar.gz \
    && echo "66ad1822dd9cf96694b95c24f25bc05cff417a65351464da01682a91796d1f2b uv-x86_64-unknown-linux-gnu.tar.gz" | sha256sum -c \
    && mkdir -p /home/vscode/.local/bin \
    && tar -xzf uv-x86_64-unknown-linux-gnu.tar.gz -C /home/vscode/.local/bin --strip-components=1 \
    && rm uv-x86_64-unknown-linux-gnu.tar.gz

RUN curl --proto '=https' --tlsv1.2 -sSfLO https://github.com/Ortham/svg_to_ico/releases/download/1.3.1/svg_to_ico.tar.xz \
    && echo "04897b6384e7c3141c8343335b66bde3b184d7d65940980c38689b3ef4453f2f svg_to_ico.tar.xz" \
    && tar xJf svg_to_ico.tar.xz -C /home/vscode/.local/bin \
    && rm svg_to_ico.tar.xz

RUN /home/vscode/.local/bin/uv run --with aqtinstall==3.3.0 -- aqt install-qt windows desktop 6.9.1 win64_mingw \
    && /home/vscode/.local/bin/uv run --with aqtinstall==3.3.0 -- aqt install-qt linux desktop 6.9.1 linux_gcc_64 --archives icu qtbase \
    && sudo mkdir /opt/qt \
    && sudo mv /home/vscode/6.9.1 /opt/qt/ \
    && rm aqtinstall.log

RUN curl --proto '=https' --tlsv1.2 -sSfLO https://github.com/PowerShell/PowerShell/releases/download/v7.5.4/powershell-7.5.4-linux-x64.tar.gz \
    && echo "1fd7983fe56ca9e6233f126925edb24bf6b6b33e356b69996d925c4db94e2fef powershell-7.5.4-linux-x64.tar.gz" | sha256sum -c \
    && sudo mkdir /opt/powershell-7.5.4-linux-x64 \
    && sudo tar -xf powershell-7.5.4-linux-x64.tar.gz -C /opt/powershell-7.5.4-linux-x64 \
    && sudo chmod +x /opt/powershell-7.5.4-linux-x64/pwsh \
    && rm powershell-7.5.4-linux-x64.tar.gz

# The installer exits successfully but doesn't work correctly if it's run during the image build.
RUN curl --proto '=https' --tlsv1.2 -sSfLO https://dl.winehq.org/wine/wine-mono/7.4.0/wine-mono-7.4.0-x86.msi \
    && echo "6413ff328ebbf7ec7689c648feb3546d8102ded865079d1fbf0331b14b3ab0ec wine-mono-7.4.0-x86.msi" | sha256sum -c

# Can't run the installer before a display is available.
RUN curl --proto '=https' --tlsv1.2 -sSfLO https://github.com/jrsoftware/issrc/releases/download/is-6_7_0/innosetup-6.7.0.exe \
    && echo "f45c7d68d1e660cf13877ec36738a5179ce72a33414f9959d35e99b68c52a697 innosetup-6.7.0.exe" | sha256sum -c

# Fix GetPreferredUILanguages.shouldReturnAtLeastOneLanguage() test
ENV PATH="$PATH:/opt/powershell-7.5.4-linux-x64" \
    WINEPATH="$WINEPATH;/opt/qt/6.9.1/mingw_64/bin"
