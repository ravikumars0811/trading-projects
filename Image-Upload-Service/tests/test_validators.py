"""
Unit tests for validators
"""
import unittest
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lambda_functions'))

from common.validators import (
    validate_image_upload,
    validate_metadata,
    validate_image_id,
    validate_user_id,
    validate_pagination_params
)
from common.constants import MAX_IMAGE_SIZE


class TestValidators(unittest.TestCase):
    """Test cases for validator functions"""

    def test_validate_image_upload_valid(self):
        """Test valid image upload"""
        is_valid, error = validate_image_upload(
            content_type='image/jpeg',
            image_size=1024 * 1024,  # 1MB
            filename='test.jpg'
        )
        self.assertTrue(is_valid)
        self.assertEqual(error, '')

    def test_validate_image_upload_invalid_content_type(self):
        """Test invalid content type"""
        is_valid, error = validate_image_upload(
            content_type='application/pdf',
            image_size=1024,
            filename='test.pdf'
        )
        self.assertFalse(is_valid)
        self.assertIn('Invalid content type', error)

    def test_validate_image_upload_size_exceeded(self):
        """Test image size exceeds limit"""
        is_valid, error = validate_image_upload(
            content_type='image/jpeg',
            image_size=MAX_IMAGE_SIZE + 1,
            filename='large.jpg'
        )
        self.assertFalse(is_valid)
        self.assertIn('exceeds maximum limit', error)

    def test_validate_image_upload_empty_filename(self):
        """Test empty filename"""
        is_valid, error = validate_image_upload(
            content_type='image/jpeg',
            image_size=1024,
            filename=''
        )
        self.assertFalse(is_valid)
        self.assertIn('Invalid filename', error)

    def test_validate_image_upload_long_filename(self):
        """Test filename too long"""
        is_valid, error = validate_image_upload(
            content_type='image/jpeg',
            image_size=1024,
            filename='a' * 256
        )
        self.assertFalse(is_valid)
        self.assertIn('Invalid filename', error)

    def test_validate_metadata_valid(self):
        """Test valid metadata"""
        metadata = {
            'user_id': 'user123',
            'filename': 'test.jpg',
            'content_type': 'image/jpeg',
            'description': 'A test image',
            'tags': ['test', 'sample']
        }
        is_valid, error = validate_metadata(metadata)
        self.assertTrue(is_valid)
        self.assertEqual(error, '')

    def test_validate_metadata_missing_required_field(self):
        """Test missing required field"""
        metadata = {
            'filename': 'test.jpg',
            'content_type': 'image/jpeg'
        }
        is_valid, error = validate_metadata(metadata)
        self.assertFalse(is_valid)
        self.assertIn('Missing required field', error)

    def test_validate_metadata_invalid_user_id(self):
        """Test invalid user_id format"""
        metadata = {
            'user_id': 'user@invalid',
            'filename': 'test.jpg',
            'content_type': 'image/jpeg'
        }
        is_valid, error = validate_metadata(metadata)
        self.assertFalse(is_valid)
        self.assertIn('Invalid user_id format', error)

    def test_validate_metadata_description_too_long(self):
        """Test description exceeds length"""
        metadata = {
            'user_id': 'user123',
            'filename': 'test.jpg',
            'content_type': 'image/jpeg',
            'description': 'a' * 1001
        }
        is_valid, error = validate_metadata(metadata)
        self.assertFalse(is_valid)
        self.assertIn('Description exceeds maximum length', error)

    def test_validate_metadata_invalid_tags_type(self):
        """Test invalid tags type"""
        metadata = {
            'user_id': 'user123',
            'filename': 'test.jpg',
            'content_type': 'image/jpeg',
            'tags': 'not-a-list'
        }
        is_valid, error = validate_metadata(metadata)
        self.assertFalse(is_valid)
        self.assertIn('Tags must be a list', error)

    def test_validate_metadata_too_many_tags(self):
        """Test too many tags"""
        metadata = {
            'user_id': 'user123',
            'filename': 'test.jpg',
            'content_type': 'image/jpeg',
            'tags': [f'tag{i}' for i in range(21)]
        }
        is_valid, error = validate_metadata(metadata)
        self.assertFalse(is_valid)
        self.assertIn('Maximum 20 tags allowed', error)

    def test_validate_metadata_tag_too_long(self):
        """Test tag exceeds length"""
        metadata = {
            'user_id': 'user123',
            'filename': 'test.jpg',
            'content_type': 'image/jpeg',
            'tags': ['a' * 51]
        }
        is_valid, error = validate_metadata(metadata)
        self.assertFalse(is_valid)
        self.assertIn('max 50 characters', error)

    def test_validate_image_id_valid(self):
        """Test valid image ID"""
        self.assertTrue(validate_image_id('abc123-def456'))
        self.assertTrue(validate_image_id('12345678-1234-5678-1234-567812345678'))

    def test_validate_image_id_invalid(self):
        """Test invalid image ID"""
        self.assertFalse(validate_image_id(''))
        self.assertFalse(validate_image_id('abc@123'))
        self.assertFalse(validate_image_id('a' * 129))

    def test_validate_user_id_valid(self):
        """Test valid user ID"""
        self.assertTrue(validate_user_id('user123'))
        self.assertTrue(validate_user_id('user-test_123'))

    def test_validate_user_id_invalid(self):
        """Test invalid user ID"""
        self.assertFalse(validate_user_id(''))
        self.assertFalse(validate_user_id('user@test'))
        self.assertFalse(validate_user_id('a' * 129))

    def test_validate_pagination_params_valid(self):
        """Test valid pagination parameters"""
        is_valid, error = validate_pagination_params(20)
        self.assertTrue(is_valid)
        self.assertEqual(error, '')

    def test_validate_pagination_params_invalid_limit(self):
        """Test invalid limit"""
        is_valid, error = validate_pagination_params(0)
        self.assertFalse(is_valid)
        self.assertIn('positive integer', error)

        is_valid, error = validate_pagination_params(101)
        self.assertFalse(is_valid)
        self.assertIn('exceeds maximum', error)


if __name__ == '__main__':
    unittest.main()
