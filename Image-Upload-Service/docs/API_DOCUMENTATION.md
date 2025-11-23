# Image Upload Service - API Documentation

## Overview

The Image Upload Service provides a RESTful API for managing image uploads, storage, and retrieval. The service supports multiple concurrent users and includes metadata management with filtering capabilities.

## Base URL

**LocalStack Development:**
```
http://localhost:4566/restapis/{API_ID}/dev/_user_request_
```

Replace `{API_ID}` with your actual API Gateway ID from the setup output.

## Authentication

Currently, the API uses user_id-based authorization. In a production environment, this should be replaced with proper authentication mechanisms (OAuth2, JWT, etc.).

---

## Endpoints

### 1. Upload Image

Upload a new image with metadata.

**Endpoint:** `POST /images`

**Request Body:**
```json
{
  "user_id": "string (required)",
  "filename": "string (required)",
  "content_type": "string (required)",
  "image_data": "string (required, base64 encoded)",
  "description": "string (optional, max 1000 chars)",
  "tags": ["string"] (optional, max 20 tags, each max 50 chars)
}
```

**Supported Content Types:**
- `image/jpeg` (.jpg, .jpeg)
- `image/png` (.png)
- `image/gif` (.gif)
- `image/webp` (.webp)

**Maximum Image Size:** 10MB

**Success Response (201 Created):**
```json
{
  "success": true,
  "data": {
    "image_id": "uuid",
    "user_id": "string",
    "filename": "string",
    "content_type": "string",
    "size": 12345,
    "description": "string",
    "tags": ["string"],
    "created_at": "2025-11-23T12:00:00.000Z",
    "s3_key": "images/user_id/image_id.ext"
  }
}
```

**Error Responses:**

- **400 Bad Request** - Invalid input
  ```json
  {
    "success": false,
    "error": {
      "message": "Missing required fields: ...",
      "code": "VALIDATION_ERROR"
    }
  }
  ```

- **500 Internal Server Error** - Server error
  ```json
  {
    "success": false,
    "error": {
      "message": "Failed to upload image to S3: ..."
    }
  }
  ```

**Example cURL:**
```bash
# First, encode your image to base64
IMAGE_BASE64=$(base64 -w 0 image.jpg)

# Upload the image
curl -X POST http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images \
  -H "Content-Type: application/json" \
  -d "{
    \"user_id\": \"user123\",
    \"filename\": \"sunset.jpg\",
    \"content_type\": \"image/jpeg\",
    \"image_data\": \"$IMAGE_BASE64\",
    \"description\": \"Beautiful sunset photo\",
    \"tags\": [\"sunset\", \"nature\", \"photography\"]
  }"
```

---

### 2. List Images

Retrieve a list of images with optional filtering.

**Endpoint:** `GET /images`

**Query Parameters:**
- `user_id` (required) - Filter by user ID
- `tags` (optional) - Comma-separated list of tags (OR filter)
- `content_type` (optional) - Filter by content type (e.g., "image/jpeg")
- `limit` (optional) - Number of results per page (default: 20, max: 100)
- `last_key` (optional) - Pagination token from previous response

**Success Response (200 OK):**
```json
{
  "success": true,
  "data": {
    "images": [
      {
        "image_id": "uuid",
        "user_id": "string",
        "filename": "string",
        "content_type": "string",
        "size": 12345,
        "description": "string",
        "tags": ["string"],
        "created_at": "2025-11-23T12:00:00.000Z",
        "updated_at": "2025-11-23T12:00:00.000Z",
        "s3_key": "string"
      }
    ],
    "count": 10,
    "has_more": false,
    "next_page_token": "base64_encoded_token",
    "filters": {
      "user_id": "user123",
      "tags": ["sunset", "nature"],
      "content_type": "image/jpeg"
    }
  }
}
```

**Error Responses:**

- **400 Bad Request** - Invalid parameters
  ```json
  {
    "success": false,
    "error": {
      "message": "user_id is required"
    }
  }
  ```

**Example cURL:**

```bash
# List all images for a user
curl -X GET "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images?user_id=user123"

# List images with tag filter
curl -X GET "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images?user_id=user123&tags=sunset,nature"

# List images with content type filter
curl -X GET "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images?user_id=user123&content_type=image/jpeg"

# List images with pagination
curl -X GET "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images?user_id=user123&limit=10"

# Get next page (use next_page_token from previous response)
curl -X GET "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images?user_id=user123&limit=10&last_key=ENCODED_TOKEN"
```

---

### 3. Get Image

Retrieve image metadata and either a presigned URL or the actual image data.

**Endpoint:** `GET /images/{image_id}`

**Path Parameters:**
- `image_id` (required) - The image ID

**Query Parameters:**
- `user_id` (required) - User ID
- `download` (optional) - If "true", returns base64 encoded image data; if "false" or omitted, returns presigned URL

**Success Response (200 OK) - Presigned URL:**
```json
{
  "success": true,
  "data": {
    "metadata": {
      "image_id": "uuid",
      "user_id": "string",
      "filename": "string",
      "content_type": "string",
      "size": 12345,
      "description": "string",
      "tags": ["string"],
      "created_at": "2025-11-23T12:00:00.000Z",
      "updated_at": "2025-11-23T12:00:00.000Z",
      "s3_key": "string"
    },
    "presigned_url": "https://...",
    "expires_in": 3600
  }
}
```

**Success Response (200 OK) - Download:**
```json
{
  "success": true,
  "data": {
    "metadata": { ... },
    "image_data": "base64_encoded_image_data"
  }
}
```

**Error Responses:**

- **404 Not Found** - Image doesn't exist
  ```json
  {
    "success": false,
    "error": {
      "message": "Image not found"
    }
  }
  ```

**Example cURL:**

```bash
# Get presigned URL
curl -X GET "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images/IMAGE_ID?user_id=user123"

# Download image data
curl -X GET "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images/IMAGE_ID?user_id=user123&download=true"

# Save downloaded image to file
curl -X GET "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images/IMAGE_ID?user_id=user123&download=true" | \
  jq -r '.data.image_data' | base64 -d > downloaded_image.jpg
```

---

### 4. Delete Image

Delete an image and its metadata.

**Endpoint:** `DELETE /images/{image_id}`

**Path Parameters:**
- `image_id` (required) - The image ID

**Query Parameters:**
- `user_id` (required) - User ID

**Success Response (204 No Content):**
```json
{
  "success": true,
  "data": {
    "message": "Image deleted successfully",
    "image_id": "uuid"
  }
}
```

**Error Responses:**

- **404 Not Found** - Image doesn't exist
  ```json
  {
    "success": false,
    "error": {
      "message": "Image not found"
    }
  }
  ```

**Example cURL:**

```bash
curl -X DELETE "http://localhost:4566/restapis/{API_ID}/dev/_user_request_/images/IMAGE_ID?user_id=user123"
```

---

## Error Codes

| HTTP Status | Error Code | Description |
|-------------|------------|-------------|
| 400 | VALIDATION_ERROR | Invalid input parameters |
| 404 | - | Resource not found |
| 500 | - | Internal server error |

---

## Rate Limiting

Currently, no rate limiting is implemented in the development environment. In production, implement appropriate rate limiting based on your requirements.

---

## Filter Capabilities

The service supports two primary filters on the List Images endpoint:

### 1. Tag Filter
- Filter images by one or more tags
- Uses OR logic (images with ANY of the specified tags)
- Example: `tags=sunset,nature` returns images tagged with "sunset" OR "nature"

### 2. Content Type Filter
- Filter images by MIME type
- Exact match only
- Example: `content_type=image/jpeg` returns only JPEG images

### Combining Filters
- Multiple filters can be combined
- Filters use AND logic between different filter types
- Example: `user_id=user123&tags=sunset&content_type=image/jpeg`
  - Returns JPEG images for user123 that have the "sunset" tag

---

## Pagination

All list endpoints support cursor-based pagination:

1. Specify `limit` parameter (default: 20, max: 100)
2. Check `has_more` in the response
3. If `has_more` is true, use `next_page_token` for the next request
4. Pass `next_page_token` as `last_key` parameter

**Example:**
```bash
# First request
curl "...?user_id=user123&limit=10"
# Response includes: "next_page_token": "eyJ..."

# Next request
curl "...?user_id=user123&limit=10&last_key=eyJ..."
```

---

## Best Practices

1. **Image Size**: Keep images under 10MB for optimal performance
2. **Tags**: Use descriptive, consistent tags for better searchability
3. **Pagination**: Use appropriate page sizes based on your UI needs
4. **Error Handling**: Always check the `success` field in responses
5. **Content Types**: Stick to supported image formats
6. **User IDs**: Use alphanumeric characters, hyphens, and underscores only

---

## Testing

Example Python script for testing the API:

```python
import requests
import base64
import json

BASE_URL = "http://localhost:4566/restapis/{API_ID}/dev/_user_request_"

# Upload image
with open('test.jpg', 'rb') as f:
    image_data = base64.b64encode(f.read()).decode('utf-8')

response = requests.post(f"{BASE_URL}/images", json={
    "user_id": "test_user",
    "filename": "test.jpg",
    "content_type": "image/jpeg",
    "image_data": image_data,
    "tags": ["test"]
})
print(f"Upload: {response.status_code}")
image_id = response.json()['data']['image_id']

# List images
response = requests.get(f"{BASE_URL}/images", params={
    "user_id": "test_user"
})
print(f"List: {response.status_code}")

# Get image
response = requests.get(f"{BASE_URL}/images/{image_id}", params={
    "user_id": "test_user"
})
print(f"Get: {response.status_code}")

# Delete image
response = requests.delete(f"{BASE_URL}/images/{image_id}", params={
    "user_id": "test_user"
})
print(f"Delete: {response.status_code}")
```
