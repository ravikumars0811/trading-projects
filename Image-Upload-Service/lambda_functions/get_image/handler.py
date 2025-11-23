"""
Lambda function for viewing/downloading images
"""
import json
import base64
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from common import (
    DynamoDBClient,
    S3Client,
    validate_image_id,
    validate_user_id,
    success_response,
    error_response,
    not_found_response,
    validation_error_response,
    internal_error_response
)


def lambda_handler(event, context):
    """
    Handle get/download image requests

    Path parameters:
    - image_id: Image ID

    Query parameters:
    - user_id: User ID
    - download: Optional - If 'true', return image data. If 'false' or omitted, return presigned URL
    """
    try:
        # Extract path parameters
        path_params = event.get('pathParameters', {}) or {}
        image_id = path_params.get('image_id')

        # Extract query parameters
        query_params = event.get('queryStringParameters', {}) or {}
        user_id = query_params.get('user_id')
        download = query_params.get('download', 'false').lower() == 'true'

        # Validate required parameters
        if not image_id:
            return validation_error_response("image_id is required")

        if not user_id:
            return validation_error_response("user_id is required")

        if not validate_image_id(image_id):
            return validation_error_response("Invalid image_id format")

        if not validate_user_id(user_id):
            return validation_error_response("Invalid user_id format")

        # Initialize clients
        dynamodb_client = DynamoDBClient()
        s3_client = S3Client()

        # Get image metadata
        try:
            metadata = dynamodb_client.get_image_metadata(image_id, user_id)
            if not metadata:
                return not_found_response("Image not found")
        except Exception as e:
            print(f"Error fetching metadata: {str(e)}")
            return internal_error_response(f"Failed to fetch image metadata: {str(e)}")

        s3_key = metadata.get('s3_key')

        # Check if image exists in S3
        try:
            if not s3_client.check_image_exists(s3_key):
                return not_found_response("Image file not found in storage")
        except Exception as e:
            print(f"Error checking S3: {str(e)}")
            return internal_error_response(f"Failed to check image storage: {str(e)}")

        # If download is requested, return image data
        if download:
            try:
                image_data = s3_client.download_image(s3_key)
                if not image_data:
                    return not_found_response("Image file not found in storage")

                # Encode image data as base64
                image_data_base64 = base64.b64encode(image_data).decode('utf-8')

                response_data = {
                    'metadata': metadata,
                    'image_data': image_data_base64
                }

                return success_response(response_data)

            except Exception as e:
                print(f"Error downloading image: {str(e)}")
                return internal_error_response(f"Failed to download image: {str(e)}")

        # Otherwise, return presigned URL
        else:
            try:
                presigned_url = s3_client.get_presigned_url(
                    s3_key=s3_key,
                    expiration=3600  # 1 hour
                )

                response_data = {
                    'metadata': metadata,
                    'presigned_url': presigned_url,
                    'expires_in': 3600
                }

                return success_response(response_data)

            except Exception as e:
                print(f"Error generating presigned URL: {str(e)}")
                return internal_error_response(f"Failed to generate presigned URL: {str(e)}")

    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        return internal_error_response(f"Unexpected error: {str(e)}")
