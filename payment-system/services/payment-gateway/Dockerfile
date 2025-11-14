FROM python:3.11-slim

WORKDIR /app

COPY services/payment-gateway/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY shared /app/shared
COPY services/payment-gateway /app

RUN mkdir -p /var/log

EXPOSE 8004

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8004"]
