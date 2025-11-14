FROM python:3.11-slim

WORKDIR /app

# Install dependencies
COPY services/user-service/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy shared modules
COPY shared /app/shared

# Copy service code
COPY services/user-service /app

# Create log directory
RUN mkdir -p /var/log

EXPOSE 8001

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8001"]
