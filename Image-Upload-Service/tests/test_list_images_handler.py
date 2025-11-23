"""
Unit tests for list images Lambda handler
"""
import unittest
import json
import base64
from unittest.mock import Mock, patch, MagicMock
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lambda_functions'))


class TestListImagesHandler(unittest.TestCase):
    """Test cases for list images Lambda handler"""

    def setUp(self):
        """Set up test fixtures"""
        self.test_images = [
            {
                'image_id': 'img1',
                'user_id': 'user123',
                'filename': 'test1.jpg',
                'content_type': 'image/jpeg',
                'size': 1024,
                'tags': ['test', 'sample']
            },
            {
                'image_id': 'img2',
                'user_id': 'user123',
                'filename': 'test2.png',
                'content_type': 'image/png',
                'size': 2048,
                'tags': ['test']
            }
        ]

    @patch('list_images.handler.DynamoDBClient')
    def test_list_images_success(self, mock_dynamodb):
        """Test successful image listing"""
        from list_images.handler import lambda_handler

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.query_images_by_user.return_value = {
            'items': self.test_images,
            'last_evaluated_key': None
        }

        event = {
            'queryStringParameters': {
                'user_id': 'user123'
            }
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 200)
        body = json.loads(response['body'])
        self.assertTrue(body['success'])
        self.assertEqual(len(body['data']['images']), 2)
        self.assertFalse(body['data']['has_more'])

        # Verify query was called
        mock_dynamodb_instance.query_images_by_user.assert_called_once()

    @patch('list_images.handler.DynamoDBClient')
    def test_list_images_with_filters(self, mock_dynamodb):
        """Test listing images with filters"""
        from list_images.handler import lambda_handler

        # Mock DynamoDB client
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.scan_images_with_filters.return_value = {
            'items': [self.test_images[0]],
            'last_evaluated_key': None
        }

        event = {
            'queryStringParameters': {
                'user_id': 'user123',
                'tags': 'test,sample',
                'content_type': 'image/jpeg'
            }
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 200)
        body = json.loads(response['body'])
        self.assertTrue(body['success'])
        self.assertEqual(len(body['data']['images']), 1)
        self.assertEqual(body['data']['filters']['tags'], ['test', 'sample'])
        self.assertEqual(body['data']['filters']['content_type'], 'image/jpeg')

        # Verify scan with filters was called
        mock_dynamodb_instance.scan_images_with_filters.assert_called_once()

    @patch('list_images.handler.DynamoDBClient')
    def test_list_images_with_pagination(self, mock_dynamodb):
        """Test listing images with pagination"""
        from list_images.handler import lambda_handler

        # Mock DynamoDB client
        next_key = {'image_id': 'img3', 'user_id': 'user123'}
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.query_images_by_user.return_value = {
            'items': self.test_images,
            'last_evaluated_key': next_key
        }

        event = {
            'queryStringParameters': {
                'user_id': 'user123',
                'limit': '10'
            }
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 200)
        body = json.loads(response['body'])
        self.assertTrue(body['success'])
        self.assertTrue(body['data']['has_more'])
        self.assertIn('next_page_token', body['data'])

        # Decode and verify pagination token
        token = body['data']['next_page_token']
        decoded = json.loads(base64.b64decode(token))
        self.assertEqual(decoded, next_key)

    def test_list_images_missing_user_id(self):
        """Test listing images without user_id"""
        from list_images.handler import lambda_handler

        event = {
            'queryStringParameters': {}
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('user_id is required', body['error']['message'])

    def test_list_images_invalid_user_id(self):
        """Test listing images with invalid user_id"""
        from list_images.handler import lambda_handler

        event = {
            'queryStringParameters': {
                'user_id': 'invalid@user'
            }
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('Invalid user_id format', body['error']['message'])

    def test_list_images_invalid_limit(self):
        """Test listing images with invalid limit"""
        from list_images.handler import lambda_handler

        event = {
            'queryStringParameters': {
                'user_id': 'user123',
                'limit': '1000'
            }
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 400)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('exceeds maximum', body['error']['message'])

    @patch('list_images.handler.DynamoDBClient')
    def test_list_images_dynamodb_error(self, mock_dynamodb):
        """Test listing images when DynamoDB fails"""
        from list_images.handler import lambda_handler

        # Mock DynamoDB client to raise exception
        mock_dynamodb_instance = MagicMock()
        mock_dynamodb.return_value = mock_dynamodb_instance
        mock_dynamodb_instance.query_images_by_user.side_effect = Exception('DynamoDB error')

        event = {
            'queryStringParameters': {
                'user_id': 'user123'
            }
        }

        response = lambda_handler(event, {})

        self.assertEqual(response['statusCode'], 500)
        body = json.loads(response['body'])
        self.assertFalse(body['success'])
        self.assertIn('Failed to query images', body['error']['message'])


if __name__ == '__main__':
    unittest.main()
