"""
Constants used across the image upload service
"""

# DynamoDB
DYNAMODB_TABLE_NAME = "ImageMetadata"
DYNAMODB_ENDPOINT = "http://localhost:4566"  # LocalStack endpoint

# S3
S3_BUCKET_NAME = "image-storage-bucket"
S3_ENDPOINT = "http://localhost:4566"  # LocalStack endpoint

# Image constraints
MAX_IMAGE_SIZE = 10 * 1024 * 1024  # 10MB
ALLOWED_IMAGE_TYPES = {
    'image/jpeg': ['.jpg', '.jpeg'],
    'image/png': ['.png'],
    'image/gif': ['.gif'],
    'image/webp': ['.webp']
}

# DynamoDB attributes
ATTR_IMAGE_ID = "image_id"
ATTR_USER_ID = "user_id"
ATTR_FILENAME = "filename"
ATTR_CONTENT_TYPE = "content_type"
ATTR_SIZE = "size"
ATTR_DESCRIPTION = "description"
ATTR_TAGS = "tags"
ATTR_CREATED_AT = "created_at"
ATTR_UPDATED_AT = "updated_at"
ATTR_S3_KEY = "s3_key"

# Response codes
HTTP_200_OK = 200
HTTP_201_CREATED = 201
HTTP_204_NO_CONTENT = 204
HTTP_400_BAD_REQUEST = 400
HTTP_404_NOT_FOUND = 404
HTTP_500_INTERNAL_ERROR = 500

# Pagination
DEFAULT_PAGE_SIZE = 20
MAX_PAGE_SIZE = 100
