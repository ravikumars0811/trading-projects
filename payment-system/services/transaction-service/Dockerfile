FROM python:3.11-slim

WORKDIR /app

COPY services/transaction-service/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY shared /app/shared
COPY services/transaction-service /app

RUN mkdir -p /var/log

EXPOSE 8003

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8003"]
