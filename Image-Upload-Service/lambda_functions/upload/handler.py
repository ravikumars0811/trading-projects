"""
Lambda function for uploading images with metadata
"""
import json
import base64
import uuid
from datetime import datetime
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from common import (
    DynamoDBClient,
    S3Client,
    validate_image_upload,
    validate_metadata,
    created_response,
    error_response,
    validation_error_response,
    internal_error_response,
    ATTR_IMAGE_ID,
    ATTR_USER_ID,
    ATTR_FILENAME,
    ATTR_CONTENT_TYPE,
    ATTR_SIZE,
    ATTR_DESCRIPTION,
    ATTR_TAGS,
    ATTR_CREATED_AT,
    ATTR_UPDATED_AT,
    ATTR_S3_KEY
)


def lambda_handler(event, context):
    """
    Handle image upload requests

    Expected event body:
    {
        "user_id": "user123",
        "filename": "image.jpg",
        "content_type": "image/jpeg",
        "image_data": "base64_encoded_image_data",
        "description": "Optional description",
        "tags": ["tag1", "tag2"]
    }
    """
    try:
        # Parse request body
        if 'body' in event:
            body = json.loads(event['body']) if isinstance(event['body'], str) else event['body']
        else:
            body = event

        # Extract required fields
        user_id = body.get('user_id')
        filename = body.get('filename')
        content_type = body.get('content_type')
        image_data_base64 = body.get('image_data')
        description = body.get('description', '')
        tags = body.get('tags', [])

        # Validate required fields
        if not all([user_id, filename, content_type, image_data_base64]):
            return validation_error_response(
                "Missing required fields: user_id, filename, content_type, image_data"
            )

        # Decode image data
        try:
            image_data = base64.b64decode(image_data_base64)
        except Exception as e:
            return validation_error_response(f"Invalid base64 image data: {str(e)}")

        image_size = len(image_data)

        # Validate image upload
        is_valid, error_msg = validate_image_upload(content_type, image_size, filename)
        if not is_valid:
            return validation_error_response(error_msg)

        # Generate unique image ID
        image_id = str(uuid.uuid4())
        timestamp = datetime.utcnow().isoformat()

        # Create S3 key
        file_extension = os.path.splitext(filename)[1]
        s3_key = f"images/{user_id}/{image_id}{file_extension}"

        # Prepare metadata
        metadata = {
            ATTR_IMAGE_ID: image_id,
            ATTR_USER_ID: user_id,
            ATTR_FILENAME: filename,
            ATTR_CONTENT_TYPE: content_type,
            ATTR_SIZE: image_size,
            ATTR_DESCRIPTION: description,
            ATTR_TAGS: tags,
            ATTR_CREATED_AT: timestamp,
            ATTR_UPDATED_AT: timestamp,
            ATTR_S3_KEY: s3_key
        }

        # Validate metadata
        is_valid, error_msg = validate_metadata(metadata)
        if not is_valid:
            return validation_error_response(error_msg)

        # Initialize clients
        s3_client = S3Client()
        dynamodb_client = DynamoDBClient()

        # Upload image to S3
        try:
            s3_client.upload_image(
                image_data=image_data,
                s3_key=s3_key,
                content_type=content_type,
                metadata={
                    'user_id': user_id,
                    'image_id': image_id,
                    'filename': filename
                }
            )
        except Exception as e:
            return internal_error_response(f"Failed to upload image to S3: {str(e)}")

        # Store metadata in DynamoDB
        try:
            dynamodb_client.put_image_metadata(metadata)
        except Exception as e:
            # Rollback: delete from S3
            try:
                s3_client.delete_image(s3_key)
            except:
                pass
            return internal_error_response(f"Failed to store metadata: {str(e)}")

        # Prepare response
        response_data = {
            'image_id': image_id,
            'user_id': user_id,
            'filename': filename,
            'content_type': content_type,
            'size': image_size,
            'description': description,
            'tags': tags,
            'created_at': timestamp,
            's3_key': s3_key
        }

        return created_response(response_data)

    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        return internal_error_response(f"Unexpected error: {str(e)}")
