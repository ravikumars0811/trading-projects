"""
Helper functions for building API Gateway responses
"""
import json
from typing import Dict, Any, Optional
from .constants import (
    HTTP_200_OK,
    HTTP_201_CREATED,
    HTTP_400_BAD_REQUEST,
    HTTP_404_NOT_FOUND,
    HTTP_500_INTERNAL_ERROR
)


def build_response(
    status_code: int,
    body: Optional[Dict[str, Any]] = None,
    headers: Optional[Dict[str, str]] = None
) -> Dict[str, Any]:
    """
    Build API Gateway response

    Args:
        status_code: HTTP status code
        body: Response body dictionary
        headers: Additional headers

    Returns:
        API Gateway response dictionary
    """
    default_headers = {
        'Content-Type': 'application/json',
        'Access-Control-Allow-Origin': '*',
        'Access-Control-Allow-Methods': 'GET,POST,PUT,DELETE,OPTIONS',
        'Access-Control-Allow-Headers': 'Content-Type,Authorization'
    }

    if headers:
        default_headers.update(headers)

    response = {
        'statusCode': status_code,
        'headers': default_headers
    }

    if body is not None:
        response['body'] = json.dumps(body)

    return response


def success_response(data: Any, status_code: int = HTTP_200_OK) -> Dict[str, Any]:
    """
    Build success response

    Args:
        data: Response data
        status_code: HTTP status code

    Returns:
        API Gateway response
    """
    return build_response(
        status_code=status_code,
        body={
            'success': True,
            'data': data
        }
    )


def error_response(
    message: str,
    status_code: int = HTTP_400_BAD_REQUEST,
    error_code: Optional[str] = None
) -> Dict[str, Any]:
    """
    Build error response

    Args:
        message: Error message
        status_code: HTTP status code
        error_code: Optional error code

    Returns:
        API Gateway response
    """
    body = {
        'success': False,
        'error': {
            'message': message
        }
    }

    if error_code:
        body['error']['code'] = error_code

    return build_response(status_code=status_code, body=body)


def created_response(data: Any) -> Dict[str, Any]:
    """
    Build created (201) response

    Args:
        data: Created resource data

    Returns:
        API Gateway response
    """
    return success_response(data, status_code=HTTP_201_CREATED)


def not_found_response(message: str = "Resource not found") -> Dict[str, Any]:
    """
    Build not found (404) response

    Args:
        message: Error message

    Returns:
        API Gateway response
    """
    return error_response(message, status_code=HTTP_404_NOT_FOUND)


def internal_error_response(message: str = "Internal server error") -> Dict[str, Any]:
    """
    Build internal server error (500) response

    Args:
        message: Error message

    Returns:
        API Gateway response
    """
    return error_response(message, status_code=HTTP_500_INTERNAL_ERROR)


def validation_error_response(message: str) -> Dict[str, Any]:
    """
    Build validation error response

    Args:
        message: Validation error message

    Returns:
        API Gateway response
    """
    return error_response(message, status_code=HTTP_400_BAD_REQUEST, error_code="VALIDATION_ERROR")
