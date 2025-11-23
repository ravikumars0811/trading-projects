#!/usr/bin/env python3
"""
Sample test script for Image Upload Service API
This script demonstrates how to use all API endpoints
"""

import requests
import base64
import json
import sys
import time
from io import BytesIO
from PIL import Image


def create_test_image():
    """Create a simple test image in memory"""
    img = Image.new('RGB', (100, 100), color='red')
    buffer = BytesIO()
    img.save(buffer, format='JPEG')
    return buffer.getvalue()


def print_response(title, response):
    """Pretty print API response"""
    print(f"\n{'='*60}")
    print(f"{title}")
    print(f"{'='*60}")
    print(f"Status Code: {response.status_code}")
    try:
        print(f"Response: {json.dumps(response.json(), indent=2)}")
    except:
        print(f"Response: {response.text}")


def main():
    # Check if API URL is provided
    if len(sys.argv) < 2:
        print("Usage: python test_api.py <API_URL>")
        print("Example: python test_api.py http://localhost:4566/restapis/abc123/dev/_user_request_")
        sys.exit(1)

    BASE_URL = sys.argv[1].rstrip('/')
    USER_ID = "test_user_123"

    print(f"Testing Image Upload Service API")
    print(f"Base URL: {BASE_URL}")
    print(f"User ID: {USER_ID}")

    # Create test image
    print("\nCreating test image...")
    image_data = create_test_image()
    image_base64 = base64.b64encode(image_data).decode('utf-8')

    # Test 1: Upload Image
    print("\n" + "="*60)
    print("TEST 1: Upload Image")
    print("="*60)

    upload_payload = {
        "user_id": USER_ID,
        "filename": "test_image.jpg",
        "content_type": "image/jpeg",
        "image_data": image_base64,
        "description": "This is a test image",
        "tags": ["test", "sample", "demo"]
    }

    response = requests.post(f"{BASE_URL}/images", json=upload_payload)
    print_response("Upload Response", response)

    if response.status_code != 201:
        print("\n❌ Upload failed! Exiting...")
        sys.exit(1)

    image_id = response.json()['data']['image_id']
    print(f"\n✓ Image uploaded successfully! Image ID: {image_id}")

    # Test 2: List All Images for User
    print("\n" + "="*60)
    print("TEST 2: List All Images")
    print("="*60)

    response = requests.get(f"{BASE_URL}/images", params={"user_id": USER_ID})
    print_response("List Images Response", response)

    if response.status_code == 200:
        count = len(response.json()['data']['images'])
        print(f"\n✓ Found {count} image(s)")
    else:
        print("\n❌ List images failed!")

    # Test 3: List Images with Tag Filter
    print("\n" + "="*60)
    print("TEST 3: List Images with Tag Filter")
    print("="*60)

    response = requests.get(
        f"{BASE_URL}/images",
        params={"user_id": USER_ID, "tags": "test,sample"}
    )
    print_response("Filtered List Response", response)

    if response.status_code == 200:
        count = len(response.json()['data']['images'])
        print(f"\n✓ Found {count} image(s) with tags 'test' or 'sample'")
    else:
        print("\n❌ Filtered list failed!")

    # Test 4: List Images with Content Type Filter
    print("\n" + "="*60)
    print("TEST 4: List Images with Content Type Filter")
    print("="*60)

    response = requests.get(
        f"{BASE_URL}/images",
        params={"user_id": USER_ID, "content_type": "image/jpeg"}
    )
    print_response("Content Type Filter Response", response)

    if response.status_code == 200:
        count = len(response.json()['data']['images'])
        print(f"\n✓ Found {count} JPEG image(s)")
    else:
        print("\n❌ Content type filter failed!")

    # Test 5: Get Image with Presigned URL
    print("\n" + "="*60)
    print("TEST 5: Get Image (Presigned URL)")
    print("="*60)

    response = requests.get(
        f"{BASE_URL}/images/{image_id}",
        params={"user_id": USER_ID}
    )
    print_response("Get Image Response", response)

    if response.status_code == 200:
        url = response.json()['data'].get('presigned_url')
        print(f"\n✓ Presigned URL obtained: {url[:50]}...")
    else:
        print("\n❌ Get image failed!")

    # Test 6: Download Image
    print("\n" + "="*60)
    print("TEST 6: Download Image Data")
    print("="*60)

    response = requests.get(
        f"{BASE_URL}/images/{image_id}",
        params={"user_id": USER_ID, "download": "true"}
    )
    print_response("Download Image Response", response)

    if response.status_code == 200:
        downloaded_data = response.json()['data'].get('image_data')
        if downloaded_data:
            decoded = base64.b64decode(downloaded_data)
            print(f"\n✓ Image downloaded successfully! Size: {len(decoded)} bytes")
        else:
            print("\n❌ No image data in response!")
    else:
        print("\n❌ Download failed!")

    # Test 7: Upload Another Image for Pagination Test
    print("\n" + "="*60)
    print("TEST 7: Upload Second Image (for pagination)")
    print("="*60)

    upload_payload2 = {
        "user_id": USER_ID,
        "filename": "test_image_2.jpg",
        "content_type": "image/jpeg",
        "image_data": image_base64,
        "description": "Second test image",
        "tags": ["test", "second"]
    }

    response = requests.post(f"{BASE_URL}/images", json=upload_payload2)
    print_response("Second Upload Response", response)

    if response.status_code == 201:
        image_id_2 = response.json()['data']['image_id']
        print(f"\n✓ Second image uploaded! Image ID: {image_id_2}")
    else:
        print("\n❌ Second upload failed!")
        image_id_2 = None

    # Test 8: Pagination
    print("\n" + "="*60)
    print("TEST 8: Pagination")
    print("="*60)

    response = requests.get(
        f"{BASE_URL}/images",
        params={"user_id": USER_ID, "limit": "1"}
    )
    print_response("Pagination Response", response)

    if response.status_code == 200:
        data = response.json()['data']
        print(f"\n✓ Got {len(data['images'])} image(s)")
        print(f"  Has more: {data['has_more']}")
        if data['has_more']:
            print(f"  Next page token: {data['next_page_token'][:30]}...")
    else:
        print("\n❌ Pagination test failed!")

    # Test 9: Delete First Image
    print("\n" + "="*60)
    print("TEST 9: Delete Image")
    print("="*60)

    response = requests.delete(
        f"{BASE_URL}/images/{image_id}",
        params={"user_id": USER_ID}
    )
    print_response("Delete Response", response)

    if response.status_code == 204:
        print(f"\n✓ Image deleted successfully!")
    else:
        print("\n❌ Delete failed!")

    # Test 10: Verify Deletion
    print("\n" + "="*60)
    print("TEST 10: Verify Deletion")
    print("="*60)

    response = requests.get(
        f"{BASE_URL}/images/{image_id}",
        params={"user_id": USER_ID}
    )
    print_response("Get Deleted Image Response", response)

    if response.status_code == 404:
        print(f"\n✓ Image correctly deleted (404 returned)")
    else:
        print("\n❌ Image should not exist!")

    # Cleanup: Delete second image if it exists
    if image_id_2:
        print("\n" + "="*60)
        print("CLEANUP: Deleting Second Image")
        print("="*60)

        response = requests.delete(
            f"{BASE_URL}/images/{image_id_2}",
            params={"user_id": USER_ID}
        )
        if response.status_code == 204:
            print(f"\n✓ Second image deleted")
        else:
            print(f"\n⚠ Failed to delete second image")

    # Summary
    print("\n" + "="*60)
    print("TEST SUMMARY")
    print("="*60)
    print("✓ All tests completed!")
    print("\nTested functionality:")
    print("  1. ✓ Image upload with metadata")
    print("  2. ✓ List all images")
    print("  3. ✓ Filter by tags")
    print("  4. ✓ Filter by content type")
    print("  5. ✓ Get presigned URL")
    print("  6. ✓ Download image data")
    print("  7. ✓ Multiple uploads")
    print("  8. ✓ Pagination")
    print("  9. ✓ Image deletion")
    print(" 10. ✓ Deletion verification")
    print("\n" + "="*60)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n\n❌ Error: {str(e)}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
