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

# Copy alembic migrations
COPY alembic.ini /src/
COPY migrations /src/migrations

# Copy the project
COPY app/ /src/app

COPY fonts /src/app/fonts

RUN mkdir /data

ENV DATA_DIR="/data"

CMD ["bash", "-c", "/src/.venv/bin/alembic upgrade head && /src/.venv/bin/fastapi run /src/app/main.py --port 80"]