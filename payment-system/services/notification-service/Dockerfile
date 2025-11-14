FROM python:3.11-slim

WORKDIR /app

COPY services/notification-service/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY shared /app/shared
COPY services/notification-service /app

RUN mkdir -p /var/log

CMD ["python", "main.py"]
