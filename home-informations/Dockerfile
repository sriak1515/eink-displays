FROM python:3.12-slim
COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/

RUN apt-get update && apt-get install -y --no-install-recommends \
  libraqm0 \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY uv.lock /app
COPY pyproject.toml /app

# Install dependencies
RUN --mount=type=cache,target=/root/.cache/uv \
    uv sync --frozen --no-install-project --no-editable

# Copy the project into the intermediate image
COPY src/ /app

COPY fonts /app/fonts

RUN mkdir /data

ENV DATA_DIR="/data"

CMD ["/app/.venv/bin/fastapi", "run", "/app/main.py", "--port", "80"]