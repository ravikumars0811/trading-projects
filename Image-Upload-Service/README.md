# Image Upload Service

A scalable, production-ready image upload and management service built with AWS serverless technologies. This service provides a complete solution for uploading, storing, retrieving, and managing images with metadata support.

## Features

- **Image Upload**: Upload images with metadata (description, tags)
- **Image Storage**: Secure storage in S3
- **Metadata Management**: Store and query image metadata in DynamoDB
- **Filtering**: Search images by tags and content type
- **Pagination**: Efficient cursor-based pagination
- **Multi-user Support**: Concurrent user access with user-based isolation
- **Scalable Architecture**: Serverless architecture using Lambda, API Gateway, S3, and DynamoDB
- **Local Development**: Complete LocalStack setup for local testing

## Architecture

```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       v
┌─────────────────┐
│  API Gateway    │
└──────┬──────────┘
       │
       v
┌─────────────────────────────────┐
│     Lambda Functions            │
│  ┌────────┐  ┌────────────┐    │
│  │ Upload │  │    List    │    │
│  └────────┘  └────────────┘    │
│  ┌────────┐  ┌────────────┐    │
│  │  Get   │  │   Delete   │    │
│  └────────┘  └────────────┘    │
└───────┬───────────────┬─────────┘
        │               │
        v               v
   ┌────────┐      ┌──────────┐
   │   S3   │      │ DynamoDB │
   │(Images)│      │(Metadata)│
   └────────┘      └──────────┘
```

## Technology Stack

- **Language**: Python 3.7+
- **Cloud Services**:
  - AWS Lambda (Compute)
  - AWS API Gateway (API Management)
  - AWS S3 (Object Storage)
  - AWS DynamoDB (NoSQL Database)
- **Local Development**: LocalStack
- **Testing**: pytest, unittest, moto
- **Deployment**: Docker, Docker Compose

## Project Structure

```
Image-Upload-Service/
├── lambda_functions/
│   ├── common/                  # Shared utilities
│   │   ├── __init__.py
│   │   ├── constants.py         # Configuration constants
│   │   ├── dynamodb_client.py   # DynamoDB operations
│   │   ├── s3_client.py         # S3 operations
│   │   ├── validators.py        # Input validation
│   │   └── response_helper.py   # Response formatting
│   ├── upload/                  # Upload image handler
│   │   └── handler.py
│   ├── list_images/             # List images handler
│   │   └── handler.py
│   ├── get_image/               # Get/download image handler
│   │   └── handler.py
│   └── delete_image/            # Delete image handler
│       └── handler.py
├── tests/                       # Unit tests
│   ├── test_validators.py
│   ├── test_upload_handler.py
│   ├── test_list_images_handler.py
│   ├── test_get_image_handler.py
│   └── test_delete_image_handler.py
├── scripts/
│   └── setup_localstack.sh      # LocalStack initialization
├── docs/
│   └── API_DOCUMENTATION.md     # Complete API documentation
├── config/
├── docker-compose.yml           # LocalStack setup
├── requirements.txt             # Project dependencies
├── requirements-test.txt        # Test dependencies
├── Makefile                     # Common commands
└── README.md
```

## Prerequisites

- Python 3.7 or higher
- Docker and Docker Compose
- AWS CLI (for LocalStack interaction)
- Make (optional, for convenience commands)

## Quick Start

### 1. Clone and Setup

```bash
# Navigate to the project directory
cd Image-Upload-Service

# Install dependencies
pip install -r requirements.txt
pip install -r requirements-test.txt
```

### 2. Start LocalStack

```bash
# Using Make
make start

# Or using Docker Compose directly
docker-compose up -d
```

### 3. Initialize Resources

```bash
# Using Make
make setup

# Or run the script directly
chmod +x scripts/setup_localstack.sh
./scripts/setup_localstack.sh
```

The setup script will:
- Create S3 bucket for image storage
- Create DynamoDB table with indexes
- Deploy Lambda functions
- Create API Gateway with endpoints

### 4. Test the Service

```bash
# Run unit tests
make test

# Or using pytest directly
pytest tests/ -v --cov=lambda_functions
```

## Usage

### Getting Started

After running the setup, you'll see output like:

```
API Gateway URL: http://localhost:4566/restapis/abc123/dev/_user_request_
```

Use this base URL for all API calls.

### Example: Complete Workflow

```bash
# Set your API URL
API_URL="http://localhost:4566/restapis/{YOUR_API_ID}/dev/_user_request_"

# 1. Upload an image
IMAGE_BASE64=$(base64 -w 0 myimage.jpg)
curl -X POST "$API_URL/images" \
  -H "Content-Type: application/json" \
  -d "{
    \"user_id\": \"user123\",
    \"filename\": \"vacation.jpg\",
    \"content_type\": \"image/jpeg\",
    \"image_data\": \"$IMAGE_BASE64\",
    \"description\": \"My vacation photo\",
    \"tags\": [\"vacation\", \"beach\", \"2025\"]
  }"

# Response will include image_id
# {"success":true,"data":{"image_id":"abc-123-def",...}}

# 2. List all images for user
curl -X GET "$API_URL/images?user_id=user123"

# 3. List images with filters
curl -X GET "$API_URL/images?user_id=user123&tags=vacation,beach"
curl -X GET "$API_URL/images?user_id=user123&content_type=image/jpeg"

# 4. Get image with presigned URL
curl -X GET "$API_URL/images/abc-123-def?user_id=user123"

# 5. Download image data
curl -X GET "$API_URL/images/abc-123-def?user_id=user123&download=true" | \
  jq -r '.data.image_data' | base64 -d > downloaded.jpg

# 6. Delete image
curl -X DELETE "$API_URL/images/abc-123-def?user_id=user123"
```

### Python Example

```python
import requests
import base64

BASE_URL = "http://localhost:4566/restapis/{API_ID}/dev/_user_request_"

# Upload image
def upload_image(filepath, user_id, description="", tags=None):
    with open(filepath, 'rb') as f:
        image_data = base64.b64encode(f.read()).decode('utf-8')

    response = requests.post(f"{BASE_URL}/images", json={
        "user_id": user_id,
        "filename": filepath.split('/')[-1],
        "content_type": "image/jpeg",
        "image_data": image_data,
        "description": description,
        "tags": tags or []
    })
    return response.json()

# List images
def list_images(user_id, tags=None, content_type=None):
    params = {"user_id": user_id}
    if tags:
        params["tags"] = ",".join(tags)
    if content_type:
        params["content_type"] = content_type

    response = requests.get(f"{BASE_URL}/images", params=params)
    return response.json()

# Get image
def get_image(image_id, user_id, download=False):
    params = {"user_id": user_id, "download": str(download).lower()}
    response = requests.get(f"{BASE_URL}/images/{image_id}", params=params)
    return response.json()

# Delete image
def delete_image(image_id, user_id):
    response = requests.delete(
        f"{BASE_URL}/images/{image_id}",
        params={"user_id": user_id}
    )
    return response.json()

# Usage
result = upload_image("photo.jpg", "user123", "My photo", ["nature", "landscape"])
image_id = result['data']['image_id']
print(f"Uploaded: {image_id}")

images = list_images("user123", tags=["nature"])
print(f"Found {len(images['data']['images'])} images")

delete_image(image_id, "user123")
print("Image deleted")
```

## API Documentation

For complete API documentation, see [API_DOCUMENTATION.md](docs/API_DOCUMENTATION.md).

### Available Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/images` | Upload new image |
| GET | `/images` | List images with filters |
| GET | `/images/{image_id}` | Get/download image |
| DELETE | `/images/{image_id}` | Delete image |

### Supported Filters

1. **Tags**: Filter by one or more tags (OR logic)
2. **Content Type**: Filter by MIME type (exact match)

## Testing

### Run All Tests

```bash
make test
```

### Run Specific Test Files

```bash
pytest tests/test_upload_handler.py -v
pytest tests/test_validators.py -v
```

### Test Coverage

```bash
pytest tests/ --cov=lambda_functions --cov-report=html
open htmlcov/index.html
```

## Configuration

### Environment Variables

The service uses the following environment variables (configured in setup script):

- `DYNAMODB_TABLE_NAME`: DynamoDB table name (default: ImageMetadata)
- `S3_BUCKET_NAME`: S3 bucket name (default: image-storage-bucket)
- `DYNAMODB_ENDPOINT`: DynamoDB endpoint URL (LocalStack: http://localhost:4566)
- `S3_ENDPOINT`: S3 endpoint URL (LocalStack: http://localhost:4566)
- `AWS_REGION`: AWS region (default: us-east-1)

### Limits and Constraints

- **Max Image Size**: 10MB
- **Max Description Length**: 1000 characters
- **Max Tags**: 20 per image
- **Max Tag Length**: 50 characters per tag
- **Max Filename Length**: 255 characters
- **Pagination Limit**: 100 items per page

## Development

### Make Commands

```bash
make install       # Install dependencies
make install-test  # Install test dependencies
make start         # Start LocalStack
make stop          # Stop LocalStack
make setup         # Setup resources
make test          # Run tests
make clean         # Clean up
make restart       # Restart everything
make all           # Complete setup
```

### Adding New Features

1. **Add Lambda Function**:
   - Create new directory in `lambda_functions/`
   - Implement `handler.py` with `lambda_handler` function
   - Add tests in `tests/`

2. **Update Setup Script**:
   - Add Lambda creation in `scripts/setup_localstack.sh`
   - Add API Gateway integration

3. **Run Tests**:
   ```bash
   make test
   ```

## Deployment

### LocalStack (Development)

Already configured! Just run:
```bash
make all
```

### AWS (Production)

For production deployment:

1. **Infrastructure as Code**:
   - Use AWS SAM, CloudFormation, or Terraform
   - Define DynamoDB table, S3 bucket, Lambda functions, and API Gateway

2. **CI/CD Pipeline**:
   - Set up GitHub Actions, GitLab CI, or Jenkins
   - Run tests before deployment
   - Deploy to staging, then production

3. **Environment Configuration**:
   - Update endpoints to production AWS services
   - Configure proper IAM roles and permissions
   - Set up CloudWatch for monitoring

4. **Security Enhancements**:
   - Implement proper authentication (Cognito, OAuth2)
   - Add API key management
   - Enable CloudTrail logging
   - Configure VPC for Lambda functions
   - Enable S3 bucket encryption

## Scalability Considerations

The architecture is designed for scalability:

1. **Lambda Auto-scaling**: Lambda functions scale automatically based on requests
2. **DynamoDB On-Demand**: Use on-demand billing or configure auto-scaling
3. **S3 Unlimited Storage**: S3 scales automatically
4. **API Gateway**: Handles up to 10,000 requests per second by default
5. **Global Secondary Index**: Optimizes queries by user_id

### Performance Tips

- Use presigned URLs instead of downloading through Lambda
- Implement CloudFront for image delivery
- Enable DynamoDB DAX for caching
- Use S3 Transfer Acceleration for faster uploads
- Implement image optimization (resize, compress) with Lambda layers

## Troubleshooting

### LocalStack Not Starting

```bash
# Check Docker
docker ps

# Check logs
docker-compose logs -f

# Restart
make restart
```

### Setup Script Fails

```bash
# Check AWS CLI
aws --version

# Check LocalStack health
curl http://localhost:4566/_localstack/health

# Re-run setup
make setup
```

### Tests Failing

```bash
# Install test dependencies
make install-test

# Check Python version
python --version  # Should be 3.7+

# Run with verbose output
pytest tests/ -v -s
```

### API Calls Failing

```bash
# Verify LocalStack is running
curl http://localhost:4566/_localstack/health

# Check API Gateway ID
aws --endpoint-url=http://localhost:4566 apigateway get-rest-apis

# Verify Lambda functions
aws --endpoint-url=http://localhost:4566 lambda list-functions
```

## Security Best Practices

For production deployment:

1. **Authentication**: Implement OAuth2 or JWT tokens
2. **Authorization**: Use IAM roles and policies
3. **Encryption**: Enable at-rest and in-transit encryption
4. **Input Validation**: Already implemented; review regularly
5. **Rate Limiting**: Implement API Gateway rate limits
6. **Logging**: Enable CloudWatch logs and CloudTrail
7. **Secrets Management**: Use AWS Secrets Manager
8. **CORS**: Configure proper CORS policies
9. **API Keys**: Require API keys for access
10. **Image Scanning**: Scan uploaded images for malware

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Run tests: `make test`
6. Submit a pull request

## License

This project is provided as-is for educational and development purposes.

## Support

For issues and questions:
- Check [API Documentation](docs/API_DOCUMENTATION.md)
- Review [Troubleshooting](#troubleshooting) section
- Open an issue on GitHub

## Acknowledgments

- Built with AWS serverless technologies
- LocalStack for local development
- Python boto3 library for AWS interactions
