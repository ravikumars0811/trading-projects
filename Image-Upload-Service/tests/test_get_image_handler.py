"""
Unit tests for get image Lambda handler
"""
import unittest
import json
import base64
from unittest.mock import Mock, patch, MagicMock
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lambda_functions'))


class TestGetImageHandler(unittest.TestCase):
    """Test cases for get image Lambda handler"""

    def setUp(self):
        """Set up test fixtures"""
        self.test_metadata = {
            'image_id': 'test-image-123',
            'user_id': 'user123',
            'filename': 'test.jpg',
            'content_type': 'image/jpeg',
            'size': 1024,
            's3_key': 'images/user123/test-image-123.jpg'
        }
        self.test_image_data = b'fake_image_data'

    @patch('get_image.handler.S3Client')
    @patch('get_image.handler.DynamoDBClient')
    def test_get_image_presigned_url(self, mock_dynamodb, mock_s3):
        """Test getting image with presigned URL"""
        from get_image.handler import lambda_handler

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.get_image_metadata.return_value = self.test_metadata

        # Mock S3 client
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.check_image_exists.return_value = True
        mock_s3_instance.get_presigned_url.return_value = 'https://test-url.com/image'

        event = {
            'pathParameters': {'image_id': 'test-image-123'},
            'queryStringParameters': {'user_id': 'user123'}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 200)
        body = json.loads(response['body'])
        self.assertTrue(body['success'])
        self.assertIn('presigned_url', body['data'])
        self.assertEqual(body['data']['presigned_url'], 'https://test-url.com/image')
        self.assertIn('metadata', body['data'])

        # Verify S3 presigned URL was generated
        mock_s3_instance.get_presigned_url.assert_called_once()

    @patch('get_image.handler.S3Client')
    @patch('get_image.handler.DynamoDBClient')
    def test_get_image_download(self, mock_dynamodb, mock_s3):
        """Test downloading image data"""
        from get_image.handler import lambda_handler

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.get_image_metadata.return_value = self.test_metadata

        # Mock S3 client
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.check_image_exists.return_value = True
        mock_s3_instance.download_image.return_value = self.test_image_data

        event = {
            'pathParameters': {'image_id': 'test-image-123'},
            'queryStringParameters': {'user_id': 'user123', 'download': 'true'}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 200)
        body = json.loads(response['body'])
        self.assertTrue(body['success'])
        self.assertIn('image_data', body['data'])

        # Verify image data is base64 encoded
        decoded_data = base64.b64decode(body['data']['image_data'])
        self.assertEqual(decoded_data, self.test_image_data)

        # Verify S3 download was called
        mock_s3_instance.download_image.assert_called_once()

    @patch('get_image.handler.DynamoDBClient')
    def test_get_image_not_found(self, mock_dynamodb):
        """Test getting non-existent image"""
        from get_image.handler import lambda_handler

        # Mock DynamoDB client to return None
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.get_image_metadata.return_value = None

        event = {
            'pathParameters': {'image_id': 'nonexistent'},
            'queryStringParameters': {'user_id': 'user123'}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 404)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('not found', body['error']['message'])

    def test_get_image_missing_image_id(self):
        """Test getting image without image_id"""
        from get_image.handler import lambda_handler

        event = {
            'pathParameters': {},
            'queryStringParameters': {'user_id': 'user123'}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('image_id is required', body['error']['message'])

    def test_get_image_missing_user_id(self):
        """Test getting image without user_id"""
        from get_image.handler import lambda_handler

        event = {
            'pathParameters': {'image_id': 'test-image-123'},
            'queryStringParameters': {}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('user_id is required', body['error']['message'])

    @patch('get_image.handler.S3Client')
    @patch('get_image.handler.DynamoDBClient')
    def test_get_image_s3_not_found(self, mock_dynamodb, mock_s3):
        """Test getting image when S3 file doesn't exist"""
        from get_image.handler import lambda_handler

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.get_image_metadata.return_value = self.test_metadata

        # Mock S3 client to return False
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.check_image_exists.return_value = False

        event = {
            'pathParameters': {'image_id': 'test-image-123'},
            'queryStringParameters': {'user_id': 'user123'}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 404)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('not found in storage', body['error']['message'])


if __name__ == '__main__':
    unittest.main()
