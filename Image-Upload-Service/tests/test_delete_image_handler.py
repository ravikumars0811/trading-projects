"""
Unit tests for delete image Lambda handler
"""
import unittest
import json
from unittest.mock import Mock, patch, MagicMock
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lambda_functions'))


class TestDeleteImageHandler(unittest.TestCase):
    """Test cases for delete image Lambda handler"""

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

    @patch('delete_image.handler.S3Client')
    @patch('delete_image.handler.DynamoDBClient')
    def test_delete_image_success(self, mock_dynamodb, mock_s3):
        """Test successful image deletion"""
        from delete_image.handler import lambda_handler

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.get_image_metadata.return_value = self.test_metadata
        mock_dynamodb_instance.delete_image_metadata.return_value = True

        # Mock S3 client
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.delete_image.return_value = True

        event = {
            'pathParameters': {'image_id': 'test-image-123'},
            'queryStringParameters': {'user_id': 'user123'}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 204)
        body = json.loads(response['body'])
        self.assertTrue(body['success'])
        self.assertEqual(body['data']['image_id'], 'test-image-123')

        # Verify S3 delete was called
        mock_s3_instance.delete_image.assert_called_once_with(
            self.test_metadata['s3_key']
        )

        # Verify DynamoDB delete was called
        mock_dynamodb_instance.delete_image_metadata.assert_called_once_with(
            'test-image-123', 'user123'
        )

    @patch('delete_image.handler.DynamoDBClient')
    def test_delete_image_not_found(self, mock_dynamodb):
        """Test deleting non-existent image"""
        from delete_image.handler import lambda_handler

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

    def test_delete_image_missing_image_id(self):
        """Test deleting image without image_id"""
        from delete_image.handler import lambda_handler

        event = {
            'pathParameters': {},
            'queryStringParameters': {'user_id': 'user123'}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('image_id is required', body['error']['message'])

    def test_delete_image_missing_user_id(self):
        """Test deleting image without user_id"""
        from delete_image.handler import lambda_handler

        event = {
            'pathParameters': {'image_id': 'test-image-123'},
            'queryStringParameters': {}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('user_id is required', body['error']['message'])

    @patch('delete_image.handler.S3Client')
    @patch('delete_image.handler.DynamoDBClient')
    def test_delete_image_s3_failure_continues(self, mock_dynamodb, mock_s3):
        """Test that deletion continues even if S3 fails"""
        from delete_image.handler import lambda_handler

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.get_image_metadata.return_value = self.test_metadata
        mock_dynamodb_instance.delete_image_metadata.return_value = True

        # Mock S3 client to raise exception
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.delete_image.side_effect = Exception('S3 error')

        event = {
            'pathParameters': {'image_id': 'test-image-123'},
            'queryStringParameters': {'user_id': 'user123'}
        }

        response = lambda_handler(event, {})

        # Should still succeed despite S3 failure
        self.assertEqual(response['statusCode'], 204)

        # Verify DynamoDB delete was still called
        mock_dynamodb_instance.delete_image_metadata.assert_called_once()

    @patch('delete_image.handler.S3Client')
    @patch('delete_image.handler.DynamoDBClient')
    def test_delete_image_dynamodb_failure(self, mock_dynamodb, mock_s3):
        """Test deletion when DynamoDB fails"""
        from delete_image.handler import lambda_handler

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.get_image_metadata.return_value = self.test_metadata
        mock_dynamodb_instance.delete_image_metadata.side_effect = Exception('DynamoDB error')

        # Mock S3 client
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.delete_image.return_value = True

        event = {
            'pathParameters': {'image_id': 'test-image-123'},
            'queryStringParameters': {'user_id': 'user123'}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 500)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('Failed to delete image metadata', body['error']['message'])


if __name__ == '__main__':
    unittest.main()
