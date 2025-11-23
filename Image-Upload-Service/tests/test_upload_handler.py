"""
Unit tests for upload Lambda handler
"""
import unittest
import json
import base64
from unittest.mock import Mock, patch, MagicMock
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lambda_functions'))


class TestUploadHandler(unittest.TestCase):
    """Test cases for upload Lambda handler"""

    def setUp(self):
        """Set up test fixtures"""
        self.test_image_data = b'fake_image_data'
        self.test_image_base64 = base64.b64encode(self.test_image_data).decode('utf-8')

        self.valid_event = {
            'body': json.dumps({
                'user_id': 'test_user',
                'filename': 'test.jpg',
                'content_type': 'image/jpeg',
                'image_data': self.test_image_base64,
                'description': 'Test image',
                'tags': ['test', 'sample']
            })
        }

    @patch('upload.handler.S3Client')
    @patch('upload.handler.DynamoDBClient')
    @patch('upload.handler.uuid.uuid4')
    def test_upload_success(self, mock_uuid, mock_dynamodb, mock_s3):
        """Test successful image upload"""
        from upload.handler import lambda_handler

        # Mock UUID
        mock_uuid.return_value = Mock()
        mock_uuid.return_value.__str__ = Mock(return_value='test-uuid-123')

        # Mock S3 client
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.upload_image.return_value = True

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.put_image_metadata.return_value = {}

        # Call handler
        response = lambda_handler(self.valid_event, {})

        # Verify response
        self.assertEqual(response['statusCode'], 201)
        body = json.loads(response['body'])
        self.assertTrue(body['success'])
        self.assertEqual(body['data']['image_id'], 'test-uuid-123')
        self.assertEqual(body['data']['user_id'], 'test_user')

        # Verify S3 upload was called
        mock_s3_instance.upload_image.assert_called_once()

        # Verify DynamoDB put was called
        mock_dynamodb_instance.put_image_metadata.assert_called_once()

    def test_upload_missing_required_field(self):
        """Test upload with missing required field"""
        from upload.handler import lambda_handler

        invalid_event = {
            'body': json.dumps({
                'user_id': 'test_user',
                'filename': 'test.jpg'
                # Missing content_type and image_data
            })
        }

        response = lambda_handler(invalid_event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('Missing required fields', body['error']['message'])

    def test_upload_invalid_base64(self):
        """Test upload with invalid base64 data"""
        from upload.handler import lambda_handler

        invalid_event = {
            'body': json.dumps({
                'user_id': 'test_user',
                'filename': 'test.jpg',
                'content_type': 'image/jpeg',
                'image_data': 'invalid_base64!!!'
            })
        }

        response = lambda_handler(invalid_event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('Invalid base64', body['error']['message'])

    def test_upload_invalid_content_type(self):
        """Test upload with invalid content type"""
        from upload.handler import lambda_handler

        invalid_event = {
            'body': json.dumps({
                'user_id': 'test_user',
                'filename': 'test.pdf',
                'content_type': 'application/pdf',
                'image_data': self.test_image_base64
            })
        }

        response = lambda_handler(invalid_event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('Invalid content type', body['error']['message'])

    @patch('upload.handler.S3Client')
    @patch('upload.handler.DynamoDBClient')
    @patch('upload.handler.uuid.uuid4')
    def test_upload_s3_failure(self, mock_uuid, mock_dynamodb, mock_s3):
        """Test upload when S3 upload fails"""
        from upload.handler import lambda_handler

        # Mock UUID
        mock_uuid.return_value = Mock()
        mock_uuid.return_value.__str__ = Mock(return_value='test-uuid-123')

        # Mock S3 client to raise exception
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.upload_image.side_effect = Exception('S3 error')

        response = lambda_handler(self.valid_event, {})

        self.assertEqual(response['statusCode'], 500)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('Failed to upload image to S3', body['error']['message'])

    @patch('upload.handler.S3Client')
    @patch('upload.handler.DynamoDBClient')
    @patch('upload.handler.uuid.uuid4')
    def test_upload_dynamodb_failure_with_rollback(self, mock_uuid, mock_dynamodb, mock_s3):
        """Test upload when DynamoDB fails and S3 is rolled back"""
        from upload.handler import lambda_handler

        # Mock UUID
        mock_uuid.return_value = Mock()
        mock_uuid.return_value.__str__ = Mock(return_value='test-uuid-123')

        # Mock S3 client
        mock_s3_instance = MagicMock()
        mock_s3.return_value = mock_s3_instance
        mock_s3_instance.upload_image.return_value = True

        # Mock DynamoDB client to raise exception
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.put_image_metadata.side_effect = Exception('DynamoDB error')

        response = lambda_handler(self.valid_event, {})

        self.assertEqual(response['statusCode'], 500)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('Failed to store metadata', body['error']['message'])

        # Verify S3 delete was called for rollback
        mock_s3_instance.delete_image.assert_called_once()


if __name__ == '__main__':
    unittest.main()
