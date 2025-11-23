"""
Lambda function for deleting images
"""
import json
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
    not_found_response,
    validation_error_response,
    internal_error_response,
    HTTP_204_NO_CONTENT
)


def lambda_handler(event, context):
    """
    Handle delete image requests

    Path parameters:
    - image_id: Image ID

    Query parameters:
    - user_id: User ID
    """
    try:
        # Extract path parameters
        path_params = event.get('pathParameters', {}) or {}
        image_id = path_params.get('image_id')

        # Extract query parameters
        query_params = event.get('queryStringParameters', {}) or {}
        user_id = query_params.get('user_id')

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

        # Get image metadata to retrieve S3 key
        try:
            metadata = dynamodb_client.get_image_metadata(image_id, user_id)
            if not metadata:
                return not_found_response("Image not found")
        except Exception as e:
            print(f"Error fetching metadata: {str(e)}")
            return internal_error_response(f"Failed to fetch image metadata: {str(e)}")

        s3_key = metadata.get('s3_key')

        # Delete from S3
        try:
            s3_client.delete_image(s3_key)
        except Exception as e:
            print(f"Error deleting from S3: {str(e)}")
            # Continue to delete metadata even if S3 deletion fails

        # Delete metadata from DynamoDB
        try:
            dynamodb_client.delete_image_metadata(image_id, user_id)
        except Exception as e:
            print(f"Error deleting metadata: {str(e)}")
            return internal_error_response(f"Failed to delete image metadata: {str(e)}")

        # Return success response
        return success_response(
            {
                'message': 'Image deleted successfully',
                'image_id': image_id
            },
            status_code=HTTP_204_NO_CONTENT
        )

    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        return internal_error_response(f"Unexpected error: {str(e)}")
