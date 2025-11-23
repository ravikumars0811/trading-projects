"""
S3 client wrapper for image storage operations
"""
import os
import boto3
from typing import Optional, Dict, Any
from botocore.exceptions import ClientError

from .constants import S3_BUCKET_NAME, S3_ENDPOINT


class S3Client:
    """S3 client for managing image storage"""

    def __init__(self, bucket_name: str = None, endpoint_url: str = None):
        """
        Initialize S3 client

        Args:
            bucket_name: S3 bucket name
            endpoint_url: S3 endpoint URL (for LocalStack)
        """
        self.bucket_name = bucket_name or os.getenv('S3_BUCKET_NAME', S3_BUCKET_NAME)
        self.endpoint_url = endpoint_url or os.getenv('S3_ENDPOINT', S3_ENDPOINT)

        # Initialize boto3 client
        self.s3_client = boto3.client(
            's3',
            endpoint_url=self.endpoint_url,
            region_name=os.getenv('AWS_REGION', 'us-east-1')
        )

    def upload_image(
        self,
        image_data: bytes,
        s3_key: str,
        content_type: str,
        metadata: Optional[Dict[str, str]] = None
    ) -> bool:
        """
        Upload image to S3

        Args:
            image_data: Image binary data
            s3_key: S3 object key
            content_type: Image content type
            metadata: Additional metadata

        Returns:
            True if upload successful
        """
        try:
            extra_args = {
                'ContentType': content_type
            }

            if metadata:
                extra_args['Metadata'] = metadata

            self.s3_client.put_object(
                Bucket=self.bucket_name,
                Key=s3_key,
                Body=image_data,
                **extra_args
            )
            return True

        except ClientError as e:
            print(f"Error uploading to S3: {str(e)}")
            raise

    def download_image(self, s3_key: str) -> Optional[bytes]:
        """
        Download image from S3

        Args:
            s3_key: S3 object key

        Returns:
            Image binary data or None if not found
        """
        try:
            response = self.s3_client.get_object(
                Bucket=self.bucket_name,
                Key=s3_key
            )
            return response['Body'].read()

        except ClientError as e:
            if e.response['Error']['Code'] == 'NoSuchKey':
                return None
            print(f"Error downloading from S3: {str(e)}")
            raise

    def delete_image(self, s3_key: str) -> bool:
        """
        Delete image from S3

        Args:
            s3_key: S3 object key

        Returns:
            True if deleted successfully
        """
        try:
            self.s3_client.delete_object(
                Bucket=self.bucket_name,
                Key=s3_key
            )
            return True

        except ClientError as e:
            print(f"Error deleting from S3: {str(e)}")
            raise

    def get_presigned_url(
        self,
        s3_key: str,
        expiration: int = 3600,
        operation: str = 'get_object'
    ) -> str:
        """
        Generate a presigned URL for S3 object

        Args:
            s3_key: S3 object key
            expiration: URL expiration time in seconds
            operation: S3 operation (get_object, put_object, etc.)

        Returns:
            Presigned URL
        """
        try:
            url = self.s3_client.generate_presigned_url(
                operation,
                Params={
                    'Bucket': self.bucket_name,
                    'Key': s3_key
                },
                ExpiresIn=expiration
            )
            return url

        except ClientError as e:
            print(f"Error generating presigned URL: {str(e)}")
            raise

    def check_image_exists(self, s3_key: str) -> bool:
        """
        Check if image exists in S3

        Args:
            s3_key: S3 object key

        Returns:
            True if exists, False otherwise
        """
        try:
            self.s3_client.head_object(
                Bucket=self.bucket_name,
                Key=s3_key
            )
            return True

        except ClientError as e:
            if e.response['Error']['Code'] == '404':
                return False
            raise
