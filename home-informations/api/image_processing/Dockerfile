FROM python:3.12-slim
COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/

RUN apt-get update && apt-get install -y --no-install-recommends \
  libraqm0 \
  build-essential \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY uv.lock /src
COPY pyproject.toml /src

# Install dependencies
RUN --mount=type=cache,target=/root/.cache/uv \
    uv sync --frozen --no-install-project --no-editable

# Build and install the C extension module
COPY src /src/src
COPY setup.py /src
RUN uv build --wheel
