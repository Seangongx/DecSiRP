# syntax=docker/dockerfile:1.7
FROM debian:trixie

# System dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake libgl-dev libboost-all-dev \
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
    m4 imagemagick git curl bash clang binutils wget gnuplot xvfb \
    && rm -rf /var/lib/apt/lists/*

ENV bin=/workdir/bin/
RUN mkdir -p "$bin"
WORKDIR $bin

# Build DGtal first: keep this layer cached when the demo changes.
RUN --mount=type=cache,target=/opt/cpm-cache \
    git clone --depth 1 -b DecSiRPdemo \
        https://github.com/kerautret/DGtal.git \
    && cmake -S DGtal -B DGtal/build \
        -DCPM_USE_LOCAL_PACKAGES=ON \
        -DCPM_SOURCE_CACHE=/opt/cpm-cache \
        -DDGTAL_WITH_POLYSCOPE_VIEWER=ON \
        -DBUILD_TESTING=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake --build DGtal/build --parallel 2

# Copy the IPOL demo files only after DGtal has been built.
COPY . .

# Build the algorithm
RUN git clone --depth 1 \
        https://github.com/kerautret/DecSiRP.git \
    && cmake -S DecSiRP -B DecSiRP/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DDGtal_DIR="$bin/DGtal/build" \
    && cmake --build DecSiRP/build --parallel 2 \
    && cp DecSiRP/build/viz "$bin"

# Runtime environment
ENV PYTHONDONTWRITEBYTECODE=1
ENV PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python
ENV PATH="${bin}:${PATH}"
ENV HOME=/home/ipol

RUN groupadd -g 1000 ipol \
    && useradd -m -u 1000 -g 1000 ipol -d "$HOME" \
    && chmod -R 777 "$HOME"

USER ipol

