"""
Validation utilities for image upload service
"""
import re
from typing import Dict, Any, List, Tuple
from .constants import MAX_IMAGE_SIZE, ALLOWED_IMAGE_TYPES


def validate_image_upload(
    content_type: str,
    image_size: int,
    filename: str
) -> Tuple[bool, str]:
    """
    Validate image upload parameters

    Args:
        content_type: Image MIME type
        image_size: Image size in bytes
        filename: Image filename

    Returns:
        Tuple of (is_valid, error_message)
    """
    # Check content type
    if content_type not in ALLOWED_IMAGE_TYPES:
        allowed = ', '.join(ALLOWED_IMAGE_TYPES.keys())
        return False, f"Invalid content type. Allowed types: {allowed}"

    # Check file size
    if image_size > MAX_IMAGE_SIZE:
        max_mb = MAX_IMAGE_SIZE / (1024 * 1024)
        return False, f"Image size exceeds maximum limit of {max_mb}MB"

    # Check filename
    if not filename or len(filename) > 255:
        return False, "Invalid filename"

    return True, ""


def validate_metadata(metadata: Dict[str, Any]) -> Tuple[bool, str]:
    """
    Validate image metadata

    Args:
        metadata: Metadata dictionary

    Returns:
        Tuple of (is_valid, error_message)
    """
    required_fields = ['user_id', 'filename', 'content_type']

    for field in required_fields:
        if field not in metadata or not metadata[field]:
            return False, f"Missing required field: {field}"

    # Validate user_id format
    user_id = metadata.get('user_id', '')
    if not re.match(r'^[a-zA-Z0-9_-]+$', user_id):
        return False, "Invalid user_id format"

    # Validate description length
    description = metadata.get('description', '')
    if description and len(description) > 1000:
        return False, "Description exceeds maximum length of 1000 characters"

    # Validate tags
    tags = metadata.get('tags', [])
    if tags:
        if not isinstance(tags, list):
            return False, "Tags must be a list"
        if len(tags) > 20:
            return False, "Maximum 20 tags allowed"
        for tag in tags:
            if not isinstance(tag, str) or len(tag) > 50:
                return False, "Each tag must be a string with max 50 characters"

    return True, ""


def validate_image_id(image_id: str) -> bool:
    """
    Validate image ID format

    Args:
        image_id: Image ID

    Returns:
        True if valid
    """
    if not image_id:
        return False

    # UUID format or alphanumeric with hyphens/underscores
    pattern = r'^[a-zA-Z0-9_-]+$'
    return bool(re.match(pattern, image_id)) and len(image_id) <= 128


def validate_user_id(user_id: str) -> bool:
    """
    Validate user ID format

    Args:
        user_id: User ID

    Returns:
        True if valid
    """
    if not user_id:
        return False

    pattern = r'^[a-zA-Z0-9_-]+$'
    return bool(re.match(pattern, user_id)) and len(user_id) <= 128


def validate_pagination_params(limit: int, last_key: str = None) -> Tuple[bool, str]:
    """
    Validate pagination parameters

    Args:
        limit: Page size limit
        last_key: Last evaluated key

    Returns:
        Tuple of (is_valid, error_message)
    """
    from .constants import MAX_PAGE_SIZE

    if not isinstance(limit, int) or limit < 1:
        return False, "Limit must be a positive integer"

    if limit > MAX_PAGE_SIZE:
        return False, f"Limit exceeds maximum of {MAX_PAGE_SIZE}"

    return True, ""
