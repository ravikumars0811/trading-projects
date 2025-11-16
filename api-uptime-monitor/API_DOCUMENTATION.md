# API Documentation

Base URL: `http://localhost:8000/api/v1`

Interactive documentation available at: `http://localhost:8000/api/docs`

## Authentication

All endpoints except registration and login require JWT authentication.

### Get Token

Include the token in subsequent requests:
```
Authorization: Bearer <your_token>
```

## Endpoints

### Authentication

#### POST /auth/register
Register a new user account.

**Request Body:**
```json
{
  "email": "user@example.com",
  "username": "johndoe",
  "password": "secure_password123",
  "full_name": "John Doe"
}
```

**Response:** `201 Created`
```json
{
  "id": 1,
  "email": "user@example.com",
  "username": "johndoe",
  "full_name": "John Doe",
  "is_active": true,
  "created_at": "2024-01-15T10:30:00Z"
}
```

#### POST /auth/login
Login and receive JWT token.

**Request Body:**
```json
{
  "username": "johndoe",
  "password": "secure_password123"
}
```

**Response:** `200 OK`
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "bearer"
}
```

#### GET /auth/me
Get current user information.

**Headers:** `Authorization: Bearer <token>`

**Response:** `200 OK`
```json
{
  "id": 1,
  "email": "user@example.com",
  "username": "johndoe",
  "full_name": "John Doe",
  "is_active": true,
  "created_at": "2024-01-15T10:30:00Z"
}
```

---

### Endpoints Management

#### GET /endpoints
List all endpoints for the authenticated user.

**Headers:** `Authorization: Bearer <token>`

**Response:** `200 OK`
```json
[
  {
    "id": 1,
    "user_id": 1,
    "name": "Production API",
    "url": "https://api.example.com/health",
    "method": "GET",
    "headers": {},
    "body": null,
    "expected_status_codes": [200],
    "timeout": 30,
    "check_interval": 300,
    "is_active": true,
    "verify_ssl": true,
    "follow_redirects": true,
    "response_time_threshold": 5000,
    "check_ssl_expiry": true,
    "ssl_expiry_alert_days": 7,
    "created_at": "2024-01-15T10:30:00Z",
    "updated_at": "2024-01-15T10:30:00Z",
    "last_checked_at": "2024-01-15T12:45:00Z"
  }
]
```

#### POST /endpoints
Create a new endpoint to monitor.

**Headers:** `Authorization: Bearer <token>`

**Request Body:**
```json
{
  "name": "Production API",
  "url": "https://api.example.com/health",
  "method": "GET",
  "headers": {
    "Authorization": "Bearer api_key_here"
  },
  "expected_status_codes": [200, 201],
  "timeout": 30,
  "check_interval": 300,
  "response_time_threshold": 5000,
  "verify_ssl": true,
  "check_ssl_expiry": true,
  "ssl_expiry_alert_days": 7
}
```

**Response:** `201 Created`
```json
{
  "id": 1,
  "user_id": 1,
  "name": "Production API",
  "url": "https://api.example.com/health",
  "method": "GET",
  "headers": {
    "Authorization": "Bearer api_key_here"
  },
  "body": null,
  "expected_status_codes": [200, 201],
  "timeout": 30,
  "check_interval": 300,
  "is_active": true,
  "verify_ssl": true,
  "follow_redirects": true,
  "response_time_threshold": 5000,
  "check_ssl_expiry": true,
  "ssl_expiry_alert_days": 7,
  "created_at": "2024-01-15T10:30:00Z",
  "updated_at": "2024-01-15T10:30:00Z",
  "last_checked_at": null
}
```

#### GET /endpoints/{endpoint_id}
Get a specific endpoint.

**Headers:** `Authorization: Bearer <token>`

**Response:** `200 OK`
```json
{
  "id": 1,
  "user_id": 1,
  "name": "Production API",
  "url": "https://api.example.com/health",
  "method": "GET",
  "headers": {},
  "body": null,
  "expected_status_codes": [200],
  "timeout": 30,
  "check_interval": 300,
  "is_active": true,
  "verify_ssl": true,
  "follow_redirects": true,
  "response_time_threshold": 5000,
  "check_ssl_expiry": true,
  "ssl_expiry_alert_days": 7,
  "created_at": "2024-01-15T10:30:00Z",
  "updated_at": "2024-01-15T10:30:00Z",
  "last_checked_at": "2024-01-15T12:45:00Z"
}
```

#### PUT /endpoints/{endpoint_id}
Update an endpoint.

**Headers:** `Authorization: Bearer <token>`

**Request Body:**
```json
{
  "name": "Updated API Name",
  "check_interval": 600,
  "is_active": false
}
```

**Response:** `200 OK`

#### DELETE /endpoints/{endpoint_id}
Delete an endpoint.

**Headers:** `Authorization: Bearer <token>`

**Response:** `204 No Content`

#### POST /endpoints/{endpoint_id}/check
Trigger an immediate health check.

**Headers:** `Authorization: Bearer <token>`

**Response:** `202 Accepted`
```json
{
  "message": "Health check initiated"
}
```

#### GET /endpoints/{endpoint_id}/stats
Get statistics for an endpoint.

**Headers:** `Authorization: Bearer <token>`

**Query Parameters:**
- `period` (optional): `24h`, `7d`, or `30d` (default: `24h`)

**Response:** `200 OK`
```json
{
  "endpoint_id": 1,
  "uptime_percentage": 99.5,
  "avg_response_time": 245.5,
  "total_checks": 288,
  "failed_checks": 2,
  "last_check": "2024-01-15T12:45:00Z",
  "current_status": "up"
}
```

---

### Health Checks

#### GET /health-checks/endpoint/{endpoint_id}
Get health check history for an endpoint.

**Headers:** `Authorization: Bearer <token>`

**Query Parameters:**
- `limit` (optional): Number of results (default: 100, max: 1000)

**Response:** `200 OK`
```json
[
  {
    "id": 1,
    "endpoint_id": 1,
    "status_code": 200,
    "response_time": 245.5,
    "is_success": true,
    "error_message": null,
    "ssl_expiry_date": "2025-01-15T00:00:00Z",
    "ssl_days_remaining": 365,
    "is_anomaly": false,
    "anomaly_score": -0.234,
    "checked_at": "2024-01-15T12:45:00Z"
  },
  {
    "id": 2,
    "endpoint_id": 1,
    "status_code": null,
    "response_time": null,
    "is_success": false,
    "error_message": "Connection timeout",
    "ssl_expiry_date": null,
    "ssl_days_remaining": null,
    "is_anomaly": false,
    "anomaly_score": null,
    "checked_at": "2024-01-15T12:40:00Z"
  }
]
```

#### GET /health-checks/endpoint/{endpoint_id}/stats
Get detailed statistics for an endpoint.

**Headers:** `Authorization: Bearer <token>`

**Query Parameters:**
- `period` (optional): `24h`, `7d`, or `30d` (default: `24h`)

**Response:** `200 OK`
```json
{
  "period": "24h",
  "uptime_percentage": 99.5,
  "avg_response_time": 245.5,
  "min_response_time": 180.2,
  "max_response_time": 850.1,
  "total_checks": 288,
  "successful_checks": 286,
  "failed_checks": 2,
  "anomalies_detected": 3
}
```

---

### Alerts

#### GET /alerts
List all alert configurations for the authenticated user.

**Headers:** `Authorization: Bearer <token>`

**Response:** `200 OK`
```json
[
  {
    "id": 1,
    "user_id": 1,
    "channel": "email",
    "alert_types": ["downtime", "slow_response", "ssl_expiry"],
    "is_active": true,
    "email": "alerts@example.com",
    "telegram_chat_id": null,
    "slack_webhook_url": null,
    "cooldown_minutes": 5,
    "created_at": "2024-01-15T10:30:00Z",
    "updated_at": "2024-01-15T10:30:00Z"
  }
]
```

#### POST /alerts
Create a new alert configuration.

**Headers:** `Authorization: Bearer <token>`

**Request Body (Email):**
```json
{
  "channel": "email",
  "alert_types": ["downtime", "slow_response", "ssl_expiry", "anomaly"],
  "is_active": true,
  "email": "alerts@example.com",
  "cooldown_minutes": 5
}
```

**Request Body (Telegram):**
```json
{
  "channel": "telegram",
  "alert_types": ["downtime", "anomaly"],
  "is_active": true,
  "telegram_chat_id": "123456789",
  "cooldown_minutes": 10
}
```

**Request Body (Slack):**
```json
{
  "channel": "slack",
  "alert_types": ["downtime", "slow_response"],
  "is_active": true,
  "slack_webhook_url": "https://hooks.slack.com/services/T00/B00/XXX",
  "cooldown_minutes": 5
}
```

**Response:** `201 Created`

#### GET /alerts/{alert_id}
Get a specific alert configuration.

**Headers:** `Authorization: Bearer <token>`

**Response:** `200 OK`

#### PUT /alerts/{alert_id}
Update an alert configuration.

**Headers:** `Authorization: Bearer <token>`

**Request Body:**
```json
{
  "alert_types": ["downtime"],
  "is_active": false,
  "cooldown_minutes": 15
}
```

**Response:** `200 OK`

#### DELETE /alerts/{alert_id}
Delete an alert configuration.

**Headers:** `Authorization: Bearer <token>`

**Response:** `204 No Content`

#### POST /alerts/{alert_id}/test
Send a test alert.

**Headers:** `Authorization: Bearer <token>`

**Response:** `200 OK`
```json
{
  "message": "Test alert sent successfully"
}
```

---

## Error Responses

### 400 Bad Request
```json
{
  "detail": "Invalid input data"
}
```

### 401 Unauthorized
```json
{
  "detail": "Could not validate credentials"
}
```

### 404 Not Found
```json
{
  "detail": "Endpoint not found"
}
```

### 422 Validation Error
```json
{
  "detail": [
    {
      "loc": ["body", "email"],
      "msg": "value is not a valid email address",
      "type": "value_error.email"
    }
  ]
}
```

### 500 Internal Server Error
```json
{
  "detail": "Internal server error"
}
```

---

## Rate Limiting

Currently, there are no rate limits implemented. For production, consider implementing rate limiting based on:
- User tier
- IP address
- API endpoint

---

## Webhooks (Future)

Coming soon: Webhook support for real-time notifications to your own endpoints.

---

## SDK Examples

### Python

```python
import requests

BASE_URL = "http://localhost:8000/api/v1"

# Login
response = requests.post(f"{BASE_URL}/auth/login", json={
    "username": "johndoe",
    "password": "password123"
})
token = response.json()["access_token"]

# Set authorization header
headers = {"Authorization": f"Bearer {token}"}

# Create endpoint
endpoint = requests.post(f"{BASE_URL}/endpoints", headers=headers, json={
    "name": "My API",
    "url": "https://api.example.com/health",
    "method": "GET",
    "check_interval": 300
})

# Get statistics
stats = requests.get(
    f"{BASE_URL}/endpoints/{endpoint.json()['id']}/stats",
    headers=headers,
    params={"period": "24h"}
)
print(stats.json())
```

### JavaScript/Node.js

```javascript
const axios = require('axios');

const BASE_URL = 'http://localhost:8000/api/v1';

async function main() {
  // Login
  const loginResponse = await axios.post(`${BASE_URL}/auth/login`, {
    username: 'johndoe',
    password: 'password123'
  });

  const token = loginResponse.data.access_token;
  const headers = { Authorization: `Bearer ${token}` };

  // Create endpoint
  const endpoint = await axios.post(`${BASE_URL}/endpoints`, {
    name: 'My API',
    url: 'https://api.example.com/health',
    method: 'GET',
    check_interval: 300
  }, { headers });

  // Get statistics
  const stats = await axios.get(
    `${BASE_URL}/endpoints/${endpoint.data.id}/stats`,
    { headers, params: { period: '24h' } }
  );

  console.log(stats.data);
}

main();
```

### cURL

```bash
# Login
TOKEN=$(curl -X POST http://localhost:8000/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"johndoe","password":"password123"}' \
  | jq -r '.access_token')

# Create endpoint
curl -X POST http://localhost:8000/api/v1/endpoints \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "name": "My API",
    "url": "https://api.example.com/health",
    "method": "GET",
    "check_interval": 300
  }'

# Get statistics
curl -X GET "http://localhost:8000/api/v1/endpoints/1/stats?period=24h" \
  -H "Authorization: Bearer $TOKEN"
```

---

## Best Practices

1. **Authentication**: Always use HTTPS in production to protect JWT tokens
2. **Error Handling**: Implement proper error handling for all API calls
3. **Rate Limiting**: Respect rate limits (when implemented)
4. **Timeout**: Set reasonable timeouts for your HTTP clients
5. **Retries**: Implement exponential backoff for failed requests
6. **Validation**: Validate all input data before sending to API
7. **Monitoring**: Monitor your API usage and performance
