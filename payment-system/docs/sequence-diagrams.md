# Payment System - Sequence Diagrams

## 1. User Registration Flow

```mermaid
sequenceDiagram
    actor User
    participant APIGateway
    participant UserService
    participant PostgreSQL
    participant Redis
    participant Kafka
    participant NotificationService

    User->>APIGateway: POST /users/register
    APIGateway->>APIGateway: Rate Limit Check
    APIGateway->>UserService: Forward Request
    UserService->>PostgreSQL: Check if email exists
    PostgreSQL-->>UserService: Email available
    UserService->>UserService: Hash password
    UserService->>PostgreSQL: Create user record
    PostgreSQL-->>UserService: User created
    UserService->>Kafka: Publish user.registered event
    UserService-->>APIGateway: Return user data
    APIGateway-->>User: 201 Created + User Info

    Kafka->>NotificationService: Consume user.registered
    NotificationService->>NotificationService: Send welcome email
    NotificationService->>MongoDB: Log notification
```

## 2. User Authentication Flow

```mermaid
sequenceDiagram
    actor User
    participant APIGateway
    participant UserService
    participant PostgreSQL
    participant Redis

    User->>APIGateway: POST /users/login
    APIGateway->>UserService: Forward credentials
    UserService->>PostgreSQL: Get user by email
    PostgreSQL-->>UserService: User record
    UserService->>UserService: Verify password
    UserService->>UserService: Generate JWT tokens
    UserService->>Redis: Cache user session
    Redis-->>UserService: Session cached
    UserService-->>APIGateway: Return tokens + user info
    APIGateway-->>User: 200 OK + Access/Refresh tokens
```

## 3. Wallet Creation Flow

```mermaid
sequenceDiagram
    actor User
    participant APIGateway
    participant WalletService
    participant PostgreSQL
    participant Redis
    participant Kafka

    User->>APIGateway: POST /wallets/create
    APIGateway->>APIGateway: Verify JWT token
    APIGateway->>WalletService: Forward request
    WalletService->>PostgreSQL: Check wallet exists
    PostgreSQL-->>WalletService: No existing wallet
    WalletService->>PostgreSQL: Create wallet with 0 balance
    PostgreSQL-->>WalletService: Wallet created
    WalletService->>Kafka: Publish wallet.created event
    WalletService->>Redis: Cache wallet data
    WalletService-->>APIGateway: Return wallet info
    APIGateway-->>User: 201 Created + Wallet Info
```

## 4. Fund Deposit Flow

```mermaid
sequenceDiagram
    actor User
    participant APIGateway
    participant TransactionService
    participant PaymentGateway
    participant WalletService
    participant PostgreSQL
    participant Kafka
    participant NotificationService

    User->>APIGateway: POST /transactions/create (deposit)
    APIGateway->>APIGateway: Verify JWT token
    APIGateway->>TransactionService: Forward request
    TransactionService->>PostgreSQL: Create transaction (PENDING)
    PostgreSQL-->>TransactionService: Transaction created

    TransactionService->>PaymentGateway: Process external payment
    PaymentGateway->>PaymentGateway: Simulate card processing
    PaymentGateway->>Kafka: Publish payment.processed
    PaymentGateway-->>TransactionService: Payment successful

    TransactionService->>WalletService: POST /wallets/{id}/add-funds
    WalletService->>PostgreSQL: Update wallet balance
    PostgreSQL-->>WalletService: Balance updated
    WalletService->>Kafka: Publish wallet.funds_added
    WalletService-->>TransactionService: Funds added

    TransactionService->>PostgreSQL: Update transaction (COMPLETED)
    TransactionService->>MongoDB: Log transaction
    TransactionService->>Kafka: Publish transaction.completed
    TransactionService-->>APIGateway: Transaction successful
    APIGateway-->>User: 201 Created + Transaction Info

    Kafka->>NotificationService: Consume transaction.completed
    NotificationService->>NotificationService: Send confirmation email
```

## 5. Wallet-to-Wallet Transfer Flow

```mermaid
sequenceDiagram
    actor Sender
    participant APIGateway
    participant TransactionService
    participant WalletService
    participant PostgreSQL
    participant Redis
    participant Kafka
    participant NotificationService

    Sender->>APIGateway: POST /transactions/create (transfer)
    APIGateway->>APIGateway: Verify JWT + Rate limit
    APIGateway->>TransactionService: Forward request

    TransactionService->>PostgreSQL: Create transaction (PENDING)
    PostgreSQL-->>TransactionService: Transaction created

    TransactionService->>WalletService: Check source wallet balance
    WalletService->>PostgreSQL: Get wallet
    PostgreSQL-->>WalletService: Wallet data
    WalletService-->>TransactionService: Balance sufficient

    TransactionService->>WalletService: POST /wallets/{from_id}/deduct-funds
    WalletService->>PostgreSQL: Deduct from source
    PostgreSQL-->>WalletService: Balance updated
    WalletService->>Kafka: Publish wallet.funds_deducted
    WalletService->>Redis: Invalidate cache
    WalletService-->>TransactionService: Funds deducted

    TransactionService->>WalletService: POST /wallets/{to_id}/add-funds
    WalletService->>PostgreSQL: Add to destination
    PostgreSQL-->>WalletService: Balance updated
    WalletService->>Kafka: Publish wallet.funds_added
    WalletService->>Redis: Invalidate cache
    WalletService-->>TransactionService: Funds added

    TransactionService->>PostgreSQL: Update transaction (COMPLETED)
    TransactionService->>MongoDB: Log transaction
    TransactionService->>Kafka: Publish transaction.completed
    TransactionService-->>APIGateway: Transfer successful
    APIGateway-->>Sender: 201 Created + Transaction Info

    Kafka->>NotificationService: Consume events
    NotificationService->>NotificationService: Send notifications to both users
```

## 6. Payment Processing Flow

```mermaid
sequenceDiagram
    actor Customer
    participant APIGateway
    participant TransactionService
    participant WalletService
    participant PaymentGateway
    participant PostgreSQL
    participant Kafka
    participant MongoDB

    Customer->>APIGateway: POST /transactions/create (payment)
    APIGateway->>TransactionService: Forward request

    TransactionService->>PostgreSQL: Create transaction (PENDING)
    TransactionService->>WalletService: Verify balance
    WalletService-->>TransactionService: Balance OK

    TransactionService->>PostgreSQL: Update status (PROCESSING)

    TransactionService->>WalletService: Deduct from customer wallet
    WalletService->>PostgreSQL: Update balance
    WalletService->>Kafka: Publish wallet.funds_deducted
    WalletService-->>TransactionService: Deducted successfully

    TransactionService->>PostgreSQL: Update status (COMPLETED)
    TransactionService->>MongoDB: Log transaction details
    TransactionService->>Kafka: Publish transaction.completed

    TransactionService-->>APIGateway: Payment successful
    APIGateway-->>Customer: Transaction completed
```

## 7. Transaction Failure and Rollback Flow

```mermaid
sequenceDiagram
    actor User
    participant APIGateway
    participant TransactionService
    participant WalletService
    participant PostgreSQL
    participant Kafka

    User->>APIGateway: POST /transactions/create
    APIGateway->>TransactionService: Forward request
    TransactionService->>PostgreSQL: Create transaction (PENDING)

    TransactionService->>WalletService: Deduct funds
    WalletService->>PostgreSQL: Update balance
    WalletService-->>TransactionService: Funds deducted

    TransactionService->>WalletService: Add funds to destination
    WalletService->>PostgreSQL: Update balance
    WalletService-->>TransactionService: ERROR: Wallet inactive

    Note over TransactionService: Rollback initiated
    TransactionService->>WalletService: Refund to source wallet
    WalletService->>PostgreSQL: Restore balance
    WalletService-->>TransactionService: Refunded

    TransactionService->>PostgreSQL: Update transaction (FAILED)
    TransactionService->>Kafka: Publish transaction.failed
    TransactionService-->>APIGateway: Transaction failed
    APIGateway-->>User: 400 Bad Request + Error details
```

## 8. Monitoring and Metrics Collection Flow

```mermaid
sequenceDiagram
    participant Service
    participant Prometheus
    participant Grafana
    participant User

    loop Every 15 seconds
        Prometheus->>Service: Scrape /metrics endpoint
        Service-->>Prometheus: Return metrics (requests, latency, errors)
        Prometheus->>Prometheus: Store time-series data
    end

    User->>Grafana: Access dashboard
    Grafana->>Prometheus: Query metrics
    Prometheus-->>Grafana: Return aggregated data
    Grafana-->>User: Display visualizations

    Note over Grafana: Alerts triggered on anomalies
    Grafana->>User: Send alert notifications
```

## Key Flow Characteristics

### Reliability
- All critical operations are logged to MongoDB for audit trails
- Failed transactions trigger rollback mechanisms
- Events are published to Kafka for asynchronous processing

### Performance
- Redis caching reduces database load
- Asynchronous event processing via Kafka
- Rate limiting prevents system overload

### Security
- JWT token verification on all protected endpoints
- Rate limiting per client IP
- Sensitive data encrypted in transit and at rest

### Observability
- Prometheus metrics collection from all services
- Grafana dashboards for real-time monitoring
- Structured logging for troubleshooting
