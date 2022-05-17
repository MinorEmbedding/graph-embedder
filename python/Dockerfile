FROM python:3.9.10
WORKDIR /app

RUN mkdir /app/out
RUN mkdir /app/logs

COPY requirements.txt .
RUN pip install -r requirements.txt

COPY ./src /app/src

ENTRYPOINT [ "python", "-m", "src.solver.evolution" ]