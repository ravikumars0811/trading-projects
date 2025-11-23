"""
DynamoDB client wrapper for image metadata operations
"""
import os
import boto3
from datetime import datetime
from typing import Dict, List, Optional, Any
from decimal import Decimal
from boto3.dynamodb.conditions import Key, Attr

from .constants import (
    DYNAMODB_TABLE_NAME,
    DYNAMODB_ENDPOINT,
    ATTR_IMAGE_ID,
    ATTR_USER_ID,
    ATTR_CREATED_AT
)


class DynamoDBClient:
    """DynamoDB client for managing image metadata"""

    def __init__(self, table_name: str = None, endpoint_url: str = None):
        """
        Initialize DynamoDB client

        Args:
            table_name: DynamoDB table name
            endpoint_url: DynamoDB endpoint URL (for LocalStack)
        """
        self.table_name = table_name or os.getenv('DYNAMODB_TABLE_NAME', DYNAMODB_TABLE_NAME)
        self.endpoint_url = endpoint_url or os.getenv('DYNAMODB_ENDPOINT', DYNAMODB_ENDPOINT)

        # Initialize boto3 client
        self.dynamodb = boto3.resource(
            'dynamodb',
            endpoint_url=self.endpoint_url,
            region_name=os.getenv('AWS_REGION', 'us-east-1')
        )
        self.table = self.dynamodb.Table(self.table_name)

    def put_image_metadata(self, metadata: Dict[str, Any]) -> Dict[str, Any]:
        """
        Store image metadata in DynamoDB

        Args:
            metadata: Dictionary containing image metadata

        Returns:
            Stored metadata
        """
        # Convert float to Decimal for DynamoDB
        if 'size' in metadata:
            metadata['size'] = Decimal(str(metadata['size']))

        self.table.put_item(Item=metadata)
        return metadata

    def get_image_metadata(self, image_id: str, user_id: str) -> Optional[Dict[str, Any]]:
        """
        Retrieve image metadata by ID and user ID

        Args:
            image_id: Image ID
            user_id: User ID

        Returns:
            Image metadata or None if not found
        """
        response = self.table.get_item(
            Key={
                ATTR_IMAGE_ID: image_id,
                ATTR_USER_ID: user_id
            }
        )

        item = response.get('Item')
        if item:
            # Convert Decimal back to float
            if 'size' in item:
                item['size'] = float(item['size'])

        return item

    def delete_image_metadata(self, image_id: str, user_id: str) -> bool:
        """
        Delete image metadata

        Args:
            image_id: Image ID
            user_id: User ID

        Returns:
            True if deleted successfully
        """
        self.table.delete_item(
            Key={
                ATTR_IMAGE_ID: image_id,
                ATTR_USER_ID: user_id
            }
        )
        return True

    def query_images_by_user(
        self,
        user_id: str,
        limit: int = 20,
        last_evaluated_key: Optional[Dict] = None
    ) -> Dict[str, Any]:
        """
        Query all images for a specific user

        Args:
            user_id: User ID
            limit: Maximum number of items to return
            last_evaluated_key: Pagination token

        Returns:
            Dictionary containing items and pagination info
        """
        query_params = {
            'IndexName': 'UserIdIndex',
            'KeyConditionExpression': Key(ATTR_USER_ID).eq(user_id),
            'Limit': limit,
            'ScanIndexForward': False  # Sort by created_at descending
        }

        if last_evaluated_key:
            query_params['ExclusiveStartKey'] = last_evaluated_key

        response = self.table.query(**query_params)

        # Convert Decimal to float
        items = response.get('Items', [])
        for item in items:
            if 'size' in item:
                item['size'] = float(item['size'])

        return {
            'items': items,
            'last_evaluated_key': response.get('LastEvaluatedKey')
        }

    def scan_images_with_filters(
        self,
        user_id: Optional[str] = None,
        tags: Optional[List[str]] = None,
        content_type: Optional[str] = None,
        limit: int = 20,
        last_evaluated_key: Optional[Dict] = None
    ) -> Dict[str, Any]:
        """
        Scan images with optional filters

        Args:
            user_id: Filter by user ID
            tags: Filter by tags (images containing any of these tags)
            content_type: Filter by content type
            limit: Maximum number of items to return
            last_evaluated_key: Pagination token

        Returns:
            Dictionary containing items and pagination info
        """
        # Build filter expression
        filter_expressions = []

        if user_id:
            filter_expressions.append(Attr(ATTR_USER_ID).eq(user_id))

        if tags:
            tag_filters = [Attr('tags').contains(tag) for tag in tags]
            if len(tag_filters) == 1:
                filter_expressions.append(tag_filters[0])
            else:
                # OR condition for tags
                combined_tag_filter = tag_filters[0]
                for tag_filter in tag_filters[1:]:
                    combined_tag_filter = combined_tag_filter | tag_filter
                filter_expressions.append(combined_tag_filter)

        if content_type:
            filter_expressions.append(Attr('content_type').eq(content_type))

        # Combine all filters with AND
        scan_params = {
            'Limit': limit
        }

        if filter_expressions:
            combined_filter = filter_expressions[0]
            for expr in filter_expressions[1:]:
                combined_filter = combined_filter & expr
            scan_params['FilterExpression'] = combined_filter

        if last_evaluated_key:
            scan_params['ExclusiveStartKey'] = last_evaluated_key

        response = self.table.scan(**scan_params)

        # Convert Decimal to float
        items = response.get('Items', [])
        for item in items:
            if 'size' in item:
                item['size'] = float(item['size'])

        return {
            'items': items,
            'last_evaluated_key': response.get('LastEvaluatedKey')
        }
