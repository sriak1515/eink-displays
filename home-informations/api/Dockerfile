FROM python:3.12-slim as builder
COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/

RUN apt-get update && apt-get install -y --no-install-recommends \
  libraqm0 \
  build-essential \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY image_processing/uv.lock /src
COPY image_processing/pyproject.toml /src

# Install dependencies
RUN --mount=type=cache,target=/root/.cache/uv \
    uv sync --frozen --no-install-project --no-editable

# Build and install the C extension module
COPY image_processing/setup.py /src
COPY image_processing/src /src/src
RUN uv build --wheel

FROM python:3.12-slim
COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/

RUN apt-get update && apt-get install -y --no-install-recommends \
  libraqm0 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY uv.lock /src
COPY pyproject.toml /src

# Install dependencies
RUN --mount=type=cache,target=/root/.cache/uv \
    uv sync --frozen --no-install-project --no-editable

# Install the C extension module
COPY --from=builder /src/dist /dist
RUN uv pip install $(ls /dist/image_processing*.whl) && rm -rf /dist

# Copy alembic migrations
COPY alembic.ini /src/
COPY migrations /src/migrations

# Copy the project
COPY app/ /src/app

COPY fonts /src/app/fonts


RUN mkdir /data

ENV DATA_DIR="/data"

CMD ["bash", "-c", "/src/.venv/bin/alembic upgrade head && /src/.venv/bin/fastapi run /src/app/main.py --port 80"]