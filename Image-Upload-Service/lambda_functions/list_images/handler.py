"""
Lambda function for listing images with filters
"""
import json
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from common import (
    DynamoDBClient,
    validate_user_id,
    validate_pagination_params,
    success_response,
    error_response,
    validation_error_response,
    internal_error_response,
    DEFAULT_PAGE_SIZE
)


def lambda_handler(event, context):
    """
    Handle list images requests with filtering

    Query parameters:
    - user_id: Required - Filter by user ID
    - tags: Optional - Comma-separated list of tags (OR filter)
    - content_type: Optional - Filter by content type
    - limit: Optional - Page size (default 20, max 100)
    - last_key: Optional - Pagination token (base64 encoded JSON)
    """
    try:
        # Parse query parameters
        query_params = event.get('queryStringParameters', {}) or {}

        # Extract parameters
        user_id = query_params.get('user_id')
        tags_param = query_params.get('tags')
        content_type = query_params.get('content_type')
        limit = int(query_params.get('limit', DEFAULT_PAGE_SIZE))
        last_key_param = query_params.get('last_key')

        # Validate user_id
        if not user_id:
            return validation_error_response("user_id is required")

        if not validate_user_id(user_id):
            return validation_error_response("Invalid user_id format")

        # Validate pagination params
        is_valid, error_msg = validate_pagination_params(limit, last_key_param)
        if not is_valid:
            return validation_error_response(error_msg)

        # Parse tags
        tags = None
        if tags_param:
            tags = [tag.strip() for tag in tags_param.split(',') if tag.strip()]

        # Parse last_key
        last_key = None
        if last_key_param:
            try:
                import base64
                last_key_json = base64.b64decode(last_key_param).decode('utf-8')
                last_key = json.loads(last_key_json)
            except Exception as e:
                return validation_error_response(f"Invalid last_key format: {str(e)}")

        # Initialize DynamoDB client
        dynamodb_client = DynamoDBClient()

        # Query images
        try:
            # If filters are provided, use scan with filters
            # Otherwise, use query by user_id for better performance
            if tags or content_type:
                result = dynamodb_client.scan_images_with_filters(
                    user_id=user_id,
                    tags=tags,
                    content_type=content_type,
                    limit=limit,
                    last_evaluated_key=last_key
                )
            else:
                result = dynamodb_client.query_images_by_user(
                    user_id=user_id,
                    limit=limit,
                    last_evaluated_key=last_key
                )

            items = result.get('items', [])
            next_last_key = result.get('last_evaluated_key')

            # Prepare response
            response_data = {
                'images': items,
                'count': len(items),
                'filters': {
                    'user_id': user_id,
                    'tags': tags,
                    'content_type': content_type
                }
            }

            # Add pagination token if there are more results
            if next_last_key:
                import base64
                last_key_json = json.dumps(next_last_key)
                last_key_encoded = base64.b64encode(last_key_json.encode('utf-8')).decode('utf-8')
                response_data['next_page_token'] = last_key_encoded
                response_data['has_more'] = True
            else:
                response_data['has_more'] = False

            return success_response(response_data)

        except Exception as e:
            print(f"Error querying images: {str(e)}")
            return internal_error_response(f"Failed to query images: {str(e)}")

    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        return internal_error_response(f"Unexpected error: {str(e)}")
