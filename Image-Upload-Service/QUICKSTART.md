# Quick Start Guide

Get the Image Upload Service running in 5 minutes!

## Prerequisites

- Docker and Docker Compose installed
- Python 3.7+ installed
- AWS CLI installed

## Step-by-Step Setup

### 1. Install Dependencies

```bash
pip install -r requirements.txt
pip install -r requirements-test.txt
```

### 2. Start LocalStack

```bash
docker-compose up -d
```

Wait ~10 seconds for LocalStack to start.

### 3. Setup Resources

```bash
chmod +x scripts/setup_localstack.sh
./scripts/setup_localstack.sh
```

**Important**: Copy the API URL from the output. It will look like:
```
API Gateway URL: http://localhost:4566/restapis/abc123xyz/dev/_user_request_
```

### 4. Test the Service

Run the test script:

```bash
python scripts/test_api.py http://localhost:4566/restapis/YOUR_API_ID/dev/_user_request_
```

Replace `YOUR_API_ID` with the ID from step 3.

## Quick Test Commands

### Upload an Image

```bash
# Create a test image
python3 -c "from PIL import Image; img = Image.new('RGB', (100,100), 'red'); img.save('test.jpg')"

# Upload it
IMAGE_B64=$(base64 -w 0 test.jpg)
curl -X POST http://localhost:4566/restapis/YOUR_API_ID/dev/_user_request_/images \
  -H "Content-Type: application/json" \
  -d "{\"user_id\":\"user1\",\"filename\":\"test.jpg\",\"content_type\":\"image/jpeg\",\"image_data\":\"$IMAGE_B64\",\"tags\":[\"test\"]}"
```

### List Images

```bash
curl "http://localhost:4566/restapis/YOUR_API_ID/dev/_user_request_/images?user_id=user1"
```

### Get Image

```bash
curl "http://localhost:4566/restapis/YOUR_API_ID/dev/_user_request_/images/IMAGE_ID?user_id=user1"
```

### Delete Image

```bash
curl -X DELETE "http://localhost:4566/restapis/YOUR_API_ID/dev/_user_request_/images/IMAGE_ID?user_id=user1"
```

## Run Unit Tests

```bash
pytest tests/ -v
```

## Stop Services

```bash
docker-compose down
```

## Troubleshooting

### LocalStack Not Ready

If you see connection errors, wait longer or check:

```bash
docker-compose logs -f
curl http://localhost:4566/_localstack/health
```

### Setup Script Fails

Re-run the setup:

```bash
./scripts/setup_localstack.sh
```

### Tests Fail

Check Python version:

```bash
python --version  # Should be 3.7+
```

## Next Steps

- Read [API Documentation](docs/API_DOCUMENTATION.md)
- Review [Architecture](docs/ARCHITECTURE.md)
- Check [README](README.md) for detailed information

## Using Make (Optional)

If you have `make` installed:

```bash
make all        # Complete setup
make test       # Run tests
make restart    # Restart everything
make clean      # Clean up
```

## Python Example

```python
import requests
import base64

BASE_URL = "http://localhost:4566/restapis/YOUR_API_ID/dev/_user_request_"

# Upload
with open('image.jpg', 'rb') as f:
    data = base64.b64encode(f.read()).decode()

response = requests.post(f"{BASE_URL}/images", json={
    "user_id": "user1",
    "filename": "image.jpg",
    "content_type": "image/jpeg",
    "image_data": data,
    "tags": ["vacation"]
})

print(response.json())
```

That's it! You're ready to use the Image Upload Service. 🎉
