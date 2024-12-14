FROM python:3.9

WORKDIR /code

RUN mkdir /data

COPY requirements.txt /code/requirements.txt

RUN pip install --no-cache-dir --upgrade -r /code/requirements.txt

COPY fonts /code/fonts

COPY src /code/src

ENV DATA_DIR="/data"

CMD ["fastapi", "run", "src/main.py", "--port", "80"]