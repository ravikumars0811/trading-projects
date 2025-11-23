# Architecture Documentation

## Overview

The Image Upload Service is built using AWS serverless architecture, providing a scalable, cost-effective solution for image management.

## Architecture Diagram

```
                                    ┌──────────────────┐
                                    │                  │
                                    │   API Clients    │
                                    │  (Web/Mobile)    │
                                    │                  │
                                    └────────┬─────────┘
                                             │
                                             │ HTTPS
                                             │
                                    ┌────────▼─────────┐
                                    │                  │
                                    │   API Gateway    │
                                    │  (REST API)      │
                                    │                  │
                                    └────────┬─────────┘
                                             │
                        ┌────────────────────┼────────────────────┐
                        │                    │                    │
                        │                    │                    │
              ┌─────────▼──────┐   ┌────────▼────────┐  ┌────────▼────────┐
              │                │   │                 │  │                 │
              │  Upload Image  │   │  List Images    │  │   Get Image     │
              │    Lambda      │   │    Lambda       │  │    Lambda       │
              │                │   │                 │  │                 │
              └───────┬────────┘   └────────┬────────┘  └────────┬────────┘
                      │                     │                    │
                      │                     │                    │
              ┌───────▼────────┐   ┌────────▼────────┐  ┌────────▼────────┐
              │                │   │                 │  │                 │
              │ Delete Image   │   │                 │  │                 │
              │    Lambda      │   │                 │  │                 │
              │                │   │                 │  │                 │
              └───────┬────────┘   │                 │  │                 │
                      │            │                 │  │                 │
          ┌───────────┴────────────┴─────────────────┘  │                 │
          │                                              │                 │
          │                                              │                 │
    ┌─────▼──────┐                              ┌────────▼────────┐
    │            │                              │                 │
    │  DynamoDB  │                              │       S3        │
    │            │                              │                 │
    │ Metadata   │                              │  Image Storage  │
    │   Table    │                              │                 │
    │            │                              │                 │
    └────────────┘                              └─────────────────┘
         │                                              │
         │                                              │
         │  - User Index (GSI)                          │  - Versioning
         │  - Image Metadata                            │  - Encryption
         │  - Query/Scan                                │  - Lifecycle
         │                                              │
```

## Components

### 1. API Gateway

**Purpose**: HTTP endpoint management and request routing

**Features**:
- RESTful API endpoints
- Request/response transformation
- CORS handling
- Rate limiting (production)
- API key management (production)

**Endpoints**:
- `POST /images` → Upload Lambda
- `GET /images` → List Lambda
- `GET /images/{image_id}` → Get Lambda
- `DELETE /images/{image_id}` → Delete Lambda

### 2. Lambda Functions

#### Upload Image Lambda

**Purpose**: Handle image upload requests

**Responsibilities**:
- Validate image data and metadata
- Generate unique image ID
- Upload image to S3
- Store metadata in DynamoDB
- Implement rollback on failure

**Input**:
```json
{
  "user_id": "string",
  "filename": "string",
  "content_type": "string",
  "image_data": "base64_string",
  "description": "string",
  "tags": ["string"]
}
```

**Output**:
```json
{
  "image_id": "uuid",
  "s3_key": "string",
  "metadata": {...}
}
```

#### List Images Lambda

**Purpose**: Query and filter images

**Responsibilities**:
- Query DynamoDB by user_id
- Apply filters (tags, content_type)
- Implement pagination
- Return sorted results

**Features**:
- User-based queries via GSI
- Tag filtering (OR logic)
- Content type filtering
- Cursor-based pagination

#### Get Image Lambda

**Purpose**: Retrieve image or presigned URL

**Responsibilities**:
- Fetch metadata from DynamoDB
- Generate presigned URL (default)
- Download image data (optional)
- Return base64 encoded data

**Modes**:
1. **Presigned URL Mode**: Returns temporary URL (expires in 1 hour)
2. **Download Mode**: Returns base64 encoded image data

#### Delete Image Lambda

**Purpose**: Remove image and metadata

**Responsibilities**:
- Verify image ownership
- Delete from S3
- Delete metadata from DynamoDB
- Handle partial failures gracefully

### 3. DynamoDB Table

**Table Name**: `ImageMetadata`

**Schema**:

**Primary Key**:
- Partition Key: `image_id` (String)
- Sort Key: `user_id` (String)

**Global Secondary Index**:
- Index Name: `UserIdIndex`
- Partition Key: `user_id` (String)
- Sort Key: `created_at` (String)
- Projection: ALL

**Attributes**:
```
{
  "image_id": "uuid",
  "user_id": "string",
  "filename": "string",
  "content_type": "string",
  "size": number,
  "description": "string",
  "tags": ["string"],
  "created_at": "ISO8601",
  "updated_at": "ISO8601",
  "s3_key": "string"
}
```

**Access Patterns**:
1. Get image by ID and user: `GetItem(image_id, user_id)`
2. List all images for user: `Query(UserIdIndex, user_id)`
3. Filter images: `Scan` with FilterExpression

**Capacity**:
- Development: On-demand or 5 RCU / 5 WCU
- Production: Auto-scaling based on traffic

### 4. S3 Bucket

**Bucket Name**: `image-storage-bucket`

**Structure**:
```
image-storage-bucket/
└── images/
    └── {user_id}/
        └── {image_id}.{extension}
```

**Features**:
- Versioning enabled
- Server-side encryption (production)
- Lifecycle policies (production)
- CORS configuration

**Example S3 Key**:
```
images/user123/abc-123-def-456.jpg
```

## Data Flow

### Upload Image Flow

```
1. Client → API Gateway
   POST /images with base64 image

2. API Gateway → Upload Lambda
   Invoke with event data

3. Upload Lambda
   a. Validate input
   b. Decode base64 image
   c. Generate image_id and s3_key
   d. Upload to S3
   e. Store metadata in DynamoDB
   f. Return success

4. Upload Lambda → Client
   Return image_id and metadata
```

### List Images Flow

```
1. Client → API Gateway
   GET /images?user_id=X&tags=Y

2. API Gateway → List Lambda
   Invoke with query parameters

3. List Lambda
   a. Parse filters
   b. Query DynamoDB (UserIdIndex or Scan)
   c. Apply pagination
   d. Format response

4. List Lambda → Client
   Return images array + pagination token
```

### Get Image Flow

```
1. Client → API Gateway
   GET /images/{id}?user_id=X&download=true

2. API Gateway → Get Lambda
   Invoke with path/query parameters

3. Get Lambda
   a. Fetch metadata from DynamoDB
   b. Verify image exists in S3
   c. Generate presigned URL OR download data
   d. Return response

4. Get Lambda → Client
   Return presigned URL or base64 data
```

### Delete Image Flow

```
1. Client → API Gateway
   DELETE /images/{id}?user_id=X

2. API Gateway → Delete Lambda
   Invoke with path/query parameters

3. Delete Lambda
   a. Fetch metadata
   b. Delete from S3
   c. Delete from DynamoDB
   d. Return success

4. Delete Lambda → Client
   Return 204 No Content
```

## Security Architecture

### Current (Development)

- Basic user_id based authorization
- No encryption at rest
- No encryption in transit (LocalStack)
- No authentication

### Production Recommendations

1. **Authentication**:
   - AWS Cognito user pools
   - OAuth2 / OpenID Connect
   - JWT token validation

2. **Authorization**:
   - IAM roles and policies
   - Resource-based policies
   - Fine-grained access control

3. **Encryption**:
   - S3 server-side encryption (SSE-KMS)
   - DynamoDB encryption at rest
   - TLS 1.2+ for all connections

4. **Network Security**:
   - VPC for Lambda functions
   - VPC endpoints for AWS services
   - Security groups and NACLs

5. **Monitoring**:
   - CloudWatch logs
   - CloudTrail for audit
   - AWS X-Ray for tracing
   - CloudWatch alarms

## Scalability

### Current Limits

| Component | Limit | Notes |
|-----------|-------|-------|
| API Gateway | 10,000 req/sec | Default limit |
| Lambda Concurrency | 1,000 | Default regional limit |
| DynamoDB | 40,000 RCU/WCU | Default table limit |
| S3 | Unlimited | No limits |

### Scaling Strategies

1. **Lambda**:
   - Auto-scales automatically
   - Configure reserved concurrency for critical functions
   - Use provisioned concurrency for consistent performance

2. **DynamoDB**:
   - Use on-demand billing for variable workloads
   - Configure auto-scaling for provisioned capacity
   - Implement caching with DAX

3. **S3**:
   - Automatically scales
   - Use Transfer Acceleration for global users
   - Implement CloudFront for content delivery

4. **API Gateway**:
   - Request limit increase if needed
   - Implement caching
   - Use regional endpoints for better performance

## Cost Optimization

### Cost Breakdown (Estimated)

1. **API Gateway**: $3.50 per million requests
2. **Lambda**:
   - $0.20 per million requests
   - $0.0000166667 per GB-second
3. **DynamoDB**:
   - On-demand: $1.25 per million write requests, $0.25 per million read requests
   - Provisioned: $0.00065 per WCU/hour, $0.00013 per RCU/hour
4. **S3**:
   - Storage: $0.023 per GB/month
   - Requests: $0.005 per 1,000 PUT, $0.0004 per 1,000 GET

### Optimization Tips

1. Use presigned URLs instead of downloading through Lambda
2. Implement image compression
3. Use S3 Intelligent-Tiering
4. Set lifecycle policies to archive old images
5. Use DynamoDB on-demand for unpredictable traffic
6. Implement CloudFront caching

## Monitoring and Observability

### Metrics to Track

1. **API Gateway**:
   - Request count
   - Latency (4XX, 5XX errors)
   - Cache hit rate

2. **Lambda**:
   - Invocations
   - Duration
   - Errors
   - Throttles
   - Concurrent executions

3. **DynamoDB**:
   - Consumed read/write capacity
   - Throttled requests
   - Latency

4. **S3**:
   - Storage size
   - Request count
   - Data transfer

### Logging Strategy

1. Enable CloudWatch Logs for all Lambda functions
2. Log all API Gateway requests
3. Enable CloudTrail for audit
4. Implement structured logging (JSON)
5. Set appropriate log retention policies

## Disaster Recovery

### Backup Strategy

1. **DynamoDB**:
   - Enable point-in-time recovery
   - Create on-demand backups
   - Export to S3 for long-term storage

2. **S3**:
   - Enable versioning
   - Configure cross-region replication
   - Implement lifecycle policies

### Recovery Procedures

1. **Data Loss**:
   - Restore DynamoDB from backup
   - Restore S3 objects from versions

2. **Service Failure**:
   - Multi-region deployment
   - Route 53 health checks
   - Automated failover

## Testing Strategy

### Unit Tests

- Test all Lambda handlers
- Test validators
- Test DynamoDB and S3 clients
- Mock AWS services

### Integration Tests

- Test complete workflows
- Use LocalStack for local testing
- Test error scenarios

### Load Tests

- Use tools like Artillery or JMeter
- Test concurrent uploads
- Test pagination with large datasets
- Monitor performance metrics

## Future Enhancements

1. **Image Processing**:
   - Automatic resizing
   - Thumbnail generation
   - Format conversion
   - Compression

2. **Search**:
   - Full-text search with Elasticsearch
   - AI-powered image tagging
   - Facial recognition

3. **Analytics**:
   - Usage metrics
   - Popular images
   - User activity tracking

4. **Social Features**:
   - Image sharing
   - Comments
   - Likes/reactions

5. **Performance**:
   - CloudFront CDN
   - Image optimization
   - Lazy loading
