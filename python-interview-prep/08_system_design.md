# System Design Guide for Senior Tech Leads

## 🎯 Overview

System design is critical for senior tech lead interviews. This guide covers:
- Fundamental concepts
- Common design patterns
- Popular system design questions
- Real-world examples

---

## 📚 Table of Contents

1. [Fundamental Concepts](#fundamental-concepts)
2. [Scalability Principles](#scalability-principles)
3. [Database Design](#database-design)
4. [Caching Strategies](#caching-strategies)
5. [Load Balancing](#load-balancing)
6. [Microservices vs Monolith](#microservices-vs-monolith)
7. [CAP Theorem](#cap-theorem)
8. [Common Interview Questions](#common-interview-questions)

---

## 1. Fundamental Concepts

### Key Metrics

#### Latency
- **Definition**: Time to perform an action
- **Examples**:
  - Memory reference: 100 ns
  - SSD read: 16 μs
  - Network within datacenter: 500 μs
  - Disk seek: 10 ms
  - Network between continents: 150 ms

#### Throughput
- **Definition**: Number of operations per unit time
- **Example**: Requests per second (RPS), Queries per second (QPS)

#### Availability
- **Definition**: Percentage of time system is operational
- **Calculation**: `Uptime / (Uptime + Downtime)`
- **Standards**:
  - 99% = 3.65 days downtime/year
  - 99.9% = 8.76 hours downtime/year
  - 99.99% = 52.56 minutes downtime/year
  - 99.999% = 5.26 minutes downtime/year

### Back-of-the-Envelope Calculations

**Important Numbers:**
```
1 million requests/day = ~12 requests/second
1 billion requests/day = ~12,000 requests/second

1 TB = 1,000 GB = 1,000,000 MB
1 GB memory = ~$1/month (cloud)
1 TB storage = ~$10/month (cloud)
```

**Example Calculation:**
```
Twitter-like service:
- 300M users
- 50M daily active users
- Each user posts 2 tweets/day
- Each user reads 200 tweets/day

Write Load:
50M * 2 tweets = 100M tweets/day = 1,157 tweets/sec

Read Load:
50M * 200 reads = 10B reads/day = 115,740 reads/sec

Storage (per year):
100M tweets/day * 365 days * 200 bytes = 7.3 TB/year
```

---

## 2. Scalability Principles

### Vertical Scaling (Scale Up)
```
┌─────────────┐         ┌─────────────┐
│   Server    │         │   Server    │
│  2 CPU      │   →     │  16 CPU     │
│  4 GB RAM   │         │  64 GB RAM  │
│  100 GB SSD │         │  1 TB SSD   │
└─────────────┘         └─────────────┘
```

**Pros:**
- Simple to implement
- No code changes needed
- Lower latency

**Cons:**
- Hardware limits
- Single point of failure
- Expensive at scale

### Horizontal Scaling (Scale Out)
```
    ┌─────────────┐
    │Load Balancer│
    └─────────────┘
         │
    ┌────┴────┬────────┬────────┐
    │         │        │        │
┌───▼───┐ ┌───▼───┐ ┌──▼───┐ ┌──▼───┐
│Server1│ │Server2│ │Server3│ │Server4│
└───────┘ └───────┘ └───────┘ └───────┘
```

**Pros:**
- No single point of failure
- Cost-effective
- Unlimited scaling potential

**Cons:**
- Complexity in data consistency
- Network overhead
- Deployment complexity

---

## 3. Database Design

### SQL vs NoSQL

#### When to Use SQL:
- ACID transactions required
- Complex queries with JOINs
- Structured data with relationships
- Examples: PostgreSQL, MySQL

```sql
-- Example: E-commerce orders
SELECT
    o.order_id,
    u.name,
    SUM(oi.quantity * p.price) as total
FROM orders o
JOIN users u ON o.user_id = u.id
JOIN order_items oi ON o.order_id = oi.order_id
JOIN products p ON oi.product_id = p.id
WHERE o.created_at > '2024-01-01'
GROUP BY o.order_id, u.name;
```

#### When to Use NoSQL:
- High write throughput
- Flexible schema
- Horizontal scaling
- Simple queries

**Types of NoSQL:**

1. **Document Store** (MongoDB, CouchDB)
```json
{
  "user_id": "123",
  "name": "John Doe",
  "posts": [
    {"title": "Post 1", "likes": 100},
    {"title": "Post 2", "likes": 50}
  ]
}
```

2. **Key-Value Store** (Redis, DynamoDB)
```
Key: "user:123:profile"
Value: {"name": "John", "email": "john@example.com"}
```

3. **Column-Family** (Cassandra, HBase)
```
Row Key: user123
Column Family: profile
  - name: John
  - email: john@example.com
```

4. **Graph Database** (Neo4j)
```
(User)-[:FOLLOWS]->(User)
(User)-[:POSTED]->(Tweet)
(User)-[:LIKES]->(Tweet)
```

### Database Sharding

**Horizontal Partitioning by User ID:**
```
Shard 1: Users 0-999,999
Shard 2: Users 1M-1.999M
Shard 3: Users 2M-2.999M
```

**Sharding Strategies:**

1. **Hash-based:**
```python
shard_id = hash(user_id) % num_shards
```

2. **Range-based:**
```python
if user_id < 1_000_000:
    shard = "shard_1"
elif user_id < 2_000_000:
    shard = "shard_2"
```

3. **Geographic:**
```python
if user.country in ["US", "CA", "MX"]:
    shard = "americas_shard"
elif user.country in ["UK", "FR", "DE"]:
    shard = "europe_shard"
```

---

## 4. Caching Strategies

### Cache Hierarchy
```
Client → CDN → Application Cache → Database Cache → Database
```

### Common Caching Patterns

#### 1. Cache-Aside (Lazy Loading)
```python
def get_user(user_id):
    # Check cache first
    user = cache.get(f"user:{user_id}")
    if user:
        return user

    # Cache miss - fetch from DB
    user = db.query("SELECT * FROM users WHERE id = ?", user_id)

    # Update cache
    cache.set(f"user:{user_id}", user, ttl=3600)
    return user
```

#### 2. Write-Through Cache
```python
def update_user(user_id, data):
    # Update cache
    cache.set(f"user:{user_id}", data, ttl=3600)

    # Update database
    db.update("users", user_id, data)
```

#### 3. Write-Behind (Write-Back)
```python
def update_user(user_id, data):
    # Update cache immediately
    cache.set(f"user:{user_id}", data, ttl=3600)

    # Queue database update (asynchronous)
    queue.add_task("update_db", user_id, data)
```

### Cache Eviction Policies

1. **LRU (Least Recently Used)**
   - Evict items not accessed recently
   - Good for general purpose

2. **LFU (Least Frequently Used)**
   - Evict items accessed least often
   - Good for stable access patterns

3. **FIFO (First In First Out)**
   - Evict oldest items
   - Simple but less effective

4. **TTL (Time To Live)**
   - Items expire after time period
   - Good for time-sensitive data

---

## 5. Load Balancing

### Load Balancing Algorithms

#### 1. Round Robin
```
Request 1 → Server A
Request 2 → Server B
Request 3 → Server C
Request 4 → Server A (repeat)
```

#### 2. Least Connections
```
Server A: 5 connections
Server B: 3 connections ← Route here
Server C: 7 connections
```

#### 3. Weighted Round Robin
```
Server A (weight: 3): Gets 3/6 requests
Server B (weight: 2): Gets 2/6 requests
Server C (weight: 1): Gets 1/6 requests
```

#### 4. IP Hash
```python
server = hash(client_ip) % num_servers
# Same client always goes to same server
# Good for session persistence
```

### Load Balancer Layers

```
Layer 4 (Transport):
- Fast (works at TCP/UDP level)
- Can't inspect application data
- Examples: AWS NLB, HAProxy

Layer 7 (Application):
- Slower but smarter
- Can route based on URL, headers, cookies
- Examples: NGINX, AWS ALB
```

---

## 6. Microservices vs Monolith

### Monolithic Architecture
```
┌─────────────────────────────────┐
│      Single Application         │
│  ┌───────┬───────┬──────────┐  │
│  │  UI   │Service│ Database │  │
│  │ Layer │ Logic │  Layer   │  │
│  └───────┴───────┴──────────┘  │
└─────────────────────────────────┘
```

**Pros:**
- Simple to develop and deploy
- Easy to test
- Single codebase
- Better performance (no network calls)

**Cons:**
- Tight coupling
- Difficult to scale specific features
- Large codebase becomes unwieldy
- Single point of failure

### Microservices Architecture
```
┌──────────┐  ┌──────────┐  ┌──────────┐
│  User    │  │ Product  │  │ Order    │
│ Service  │  │ Service  │  │ Service  │
└────┬─────┘  └────┬─────┘  └────┬─────┘
     │             │             │
┌────▼─────┐  ┌───▼──────┐  ┌───▼──────┐
│ User DB  │  │Product DB│  │ Order DB │
└──────────┘  └──────────┘  └──────────┘
```

**Pros:**
- Independent deployment
- Technology flexibility
- Easy to scale specific services
- Fault isolation

**Cons:**
- Complex deployment
- Distributed system challenges
- Network latency
- Data consistency issues

---

## 7. CAP Theorem

**You can only guarantee 2 out of 3:**

### Consistency (C)
All nodes see the same data at the same time

### Availability (A)
Every request receives a response (success or failure)

### Partition Tolerance (P)
System continues to operate despite network partitions

```
         CAP
        /   \
       /     \
      /       \
   CA          CP
   (RDBMS)   (MongoDB)

     AP
   (Cassandra, DynamoDB)
```

**In distributed systems, P is mandatory, so choose:**
- **CP**: Sacrifice availability for consistency
- **AP**: Sacrifice consistency for availability

---

## 8. Common Interview Questions

### Question 1: Design Twitter

**Requirements:**
- Post tweets (280 chars)
- Follow users
- Home timeline (tweets from people you follow)
- 300M users, 50M DAU
- Read-heavy (100:1 read/write ratio)

**High-Level Design:**
```
┌─────────┐
│  Client │
└────┬────┘
     │
┌────▼────────┐
│Load Balancer│
└────┬────────┘
     │
┌────▼───────────────────────┐
│   Application Servers      │
└────┬───────────────────────┘
     │
┌────▼──────┬──────────┬──────────┐
│           │          │          │
│   Tweet   │  User    │Timeline  │
│  Service  │ Service  │ Service  │
└────┬──────┴──────┬───┴────┬─────┘
     │             │        │
┌────▼──────┐ ┌────▼────┐ ┌▼──────┐
│ Tweet DB  │ │ User DB │ │Redis  │
│(Cassandra)│ │(MySQL)  │ │Cache  │
└───────────┘ └─────────┘ └───────┘
```

**Key Components:**

1. **Tweet Service:**
   - Write tweets to Cassandra
   - Tweet ID generation (Snowflake)

2. **User Service:**
   - User profiles in MySQL
   - Follow relationships (graph structure)

3. **Timeline Service:**
   - Fan-out on write for small followers
   - Fan-out on read for celebrities
   - Cache timelines in Redis

**Data Models:**

```python
# Tweet
{
    "tweet_id": "1234567890",
    "user_id": "user123",
    "content": "Hello World!",
    "created_at": "2024-01-01T12:00:00Z",
    "media_urls": ["url1", "url2"]
}

# Timeline Cache (Redis)
Key: "timeline:user123"
Value: [tweet_id1, tweet_id2, tweet_id3, ...]
```

**Scalability:**
- Shard tweets by tweet_id
- Cache hot timelines
- Use CDN for media
- Async processing for fan-out

---

### Question 2: Design URL Shortener

**Requirements:**
- Shorten long URLs
- Redirect to original URL
- Analytics (click count)
- 100M URLs/month
- 100:1 read/write ratio

**High-Level Design:**
```
┌────────┐
│ Client │
└───┬────┘
    │
┌───▼────────────┐
│ Load Balancer  │
└───┬────────────┘
    │
┌───▼──────────┐
│  API Server  │
└───┬──────────┘
    │
┌───▼────┬────────┐
│        │        │
│ Redis  │  MySQL │
│ Cache  │   DB   │
└────────┴────────┘
```

**URL Shortening Algorithm:**

```python
# Base62 encoding (a-zA-Z0-9)
def encode(num: int) -> str:
    chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    result = []

    while num > 0:
        result.append(chars[num % 62])
        num //= 62

    return ''.join(reversed(result))

# Example: 125 → "cb"
```

**Data Model:**

```sql
CREATE TABLE urls (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    short_url VARCHAR(10) UNIQUE,
    long_url TEXT,
    user_id BIGINT,
    created_at TIMESTAMP,
    clicks BIGINT DEFAULT 0
);

CREATE INDEX idx_short_url ON urls(short_url);
```

**Scalability:**
- Cache popular URLs in Redis
- Use DB auto-increment for ID generation
- Shard by short_url hash

---

### Question 3: Design Instagram

**Requirements:**
- Upload/view photos
- Follow users
- Feed generation
- 500M users, 100M DAU
- 95M photos/day

**High-Level Design:**
```
┌──────────────────────────────────┐
│         Client Apps              │
└────────────┬─────────────────────┘
             │
┌────────────▼─────────────────────┐
│         API Gateway              │
└────┬────────────────────┬────────┘
     │                    │
┌────▼─────┐      ┌───────▼───────┐
│  Photo   │      │     User      │
│ Service  │      │   Service     │
└────┬─────┘      └───────┬───────┘
     │                    │
┌────▼──────┐      ┌──────▼───────┐
│   CDN     │      │  User DB     │
│(S3+CF)    │      │  (MySQL)     │
└───────────┘      └──────────────┘
```

**Photo Upload Flow:**

1. Client uploads photo to API
2. API generates unique photo ID
3. Photo stored in S3
4. Metadata stored in database
5. CDN URL returned to client

**Feed Generation:**

```python
def generate_feed(user_id, page=1, size=20):
    # Get users followed by this user
    following = get_following(user_id)

    # Get recent photos from followed users
    photos = []
    for followed_user in following:
        user_photos = get_recent_photos(followed_user, limit=10)
        photos.extend(user_photos)

    # Sort by timestamp
    photos.sort(key=lambda x: x.timestamp, reverse=True)

    # Paginate
    start = (page - 1) * size
    return photos[start:start + size]
```

**Optimizations:**
- Pre-generate feeds for active users
- Store feeds in Redis
- Use ranking algorithm (ML-based)
- Lazy load images

---

### Question 4: Design Rate Limiter

**Requirements:**
- Limit API calls per user
- Multiple rate limit tiers
- Distributed system
- Low latency (<10ms)

**Algorithms:**

#### 1. Token Bucket
```python
class TokenBucket:
    def __init__(self, capacity, refill_rate):
        self.capacity = capacity
        self.tokens = capacity
        self.refill_rate = refill_rate  # tokens/second
        self.last_refill = time.time()

    def allow_request(self):
        self._refill()

        if self.tokens >= 1:
            self.tokens -= 1
            return True
        return False

    def _refill(self):
        now = time.time()
        elapsed = now - self.last_refill
        tokens_to_add = elapsed * self.refill_rate

        self.tokens = min(self.capacity, self.tokens + tokens_to_add)
        self.last_refill = now
```

#### 2. Sliding Window Log
```python
# Redis-based implementation
def is_allowed(user_id, limit, window):
    now = time.time()
    key = f"ratelimit:{user_id}"

    # Remove old entries
    redis.zremrangebyscore(key, 0, now - window)

    # Count requests in window
    count = redis.zcard(key)

    if count < limit:
        redis.zadd(key, {now: now})
        redis.expire(key, window)
        return True

    return False
```

**Implementation:**
```
┌────────┐
│ Client │
└───┬────┘
    │
┌───▼──────────────┐
│  Rate Limiter    │
│  (Redis-based)   │
└───┬──────────────┘
    │
┌───▼──────────┐
│  API Server  │
└──────────────┘
```

---

## 9. Key Takeaways for Interviews

### 1. **Clarify Requirements**
Always ask:
- Functional requirements
- Non-functional requirements (scale, latency, consistency)
- Constraints and assumptions

### 2. **Start with High-Level Design**
- Draw boxes and arrows
- Identify major components
- Discuss data flow

### 3. **Dive into Components**
- Database schema
- API design
- Algorithms used

### 4. **Discuss Trade-offs**
- SQL vs NoSQL
- Consistency vs Availability
- Latency vs Throughput
- Cost vs Performance

### 5. **Address Scalability**
- How to scale each component
- Bottlenecks and solutions
- Monitoring and alerts

### 6. **Common Patterns to Know**
- Load balancing
- Caching
- Sharding
- Replication
- Message queues
- CDN
- Rate limiting

---

## 10. Resources

### Books
- "Designing Data-Intensive Applications" - Martin Kleppmann
- "System Design Interview" - Alex Xu

### Online Resources
- https://github.com/donnemartin/system-design-primer
- https://www.educative.io/courses/grokking-the-system-design-interview

### Practice Platforms
- LeetCode System Design
- Pramp
- interviewing.io

---

## Conclusion

System design interviews test:
1. **Technical Knowledge**: Understanding of distributed systems
2. **Problem Solving**: Breaking down complex problems
3. **Communication**: Explaining trade-offs clearly
4. **Experience**: Real-world engineering judgment

**Success Formula:**
```
Practice + Fundamentals + Communication = Success
```

Good luck! 🚀
