# Payment System - Class Diagram

```mermaid
classDiagram
    %% User Service Classes
    class User {
        +String id
        +String email
        +String hashed_password
        +String first_name
        +String last_name
        +String phone
        +UserRole role
        +Boolean is_active
        +Boolean is_verified
        +DateTime created_at
        +DateTime updated_at
        +to_dict()
    }

    class UserRepository {
        -Session db
        +create_user(UserCreate) User
        +get_user_by_id(str) User
        +get_user_by_email(str) User
        +update_user(str, UserUpdate) User
        +delete_user(str) bool
        +verify_user(str) bool
    }

    class UserService {
        -UserRepository repository
        -RedisClient redis
        -KafkaProducer kafka
        +register_user(UserCreate) UserResponse
        +authenticate_user(UserLogin) dict
        +get_user(str) UserResponse
        +update_user(str, UserUpdate) UserResponse
        +verify_user(str) bool
        +logout_user(str) bool
    }

    %% Wallet Service Classes
    class Wallet {
        +String id
        +String user_id
        +Decimal balance
        +Currency currency
        +Boolean is_active
        +DateTime created_at
        +DateTime updated_at
        +to_dict()
    }

    class WalletRepository {
        -Session db
        +create_wallet(str, Currency) Wallet
        +get_wallet_by_id(str) Wallet
        +get_wallet_by_user_id(str, Currency) Wallet
        +update_balance(str, Decimal, str) Wallet
        +deactivate_wallet(str) bool
    }

    class WalletService {
        -WalletRepository repository
        -RedisClient redis
        -KafkaProducer kafka
        +create_wallet(str, Currency) WalletResponse
        +get_wallet(str) WalletResponse
        +add_funds(str, Decimal) WalletResponse
        +deduct_funds(str, Decimal) WalletResponse
        +get_balance(str) dict
    }

    %% Transaction Service Classes
    class Transaction {
        +String id
        +String reference_id
        +TransactionType transaction_type
        +TransactionStatus status
        +Decimal amount
        +Currency currency
        +String from_wallet_id
        +String to_wallet_id
        +PaymentMethod payment_method
        +String description
        +JSON metadata
        +DateTime created_at
        +DateTime updated_at
        +DateTime completed_at
        +to_dict()
    }

    class TransactionRepository {
        -Session db
        +create_transaction(TransactionCreate) Transaction
        +get_transaction_by_id(str) Transaction
        +get_transaction_by_reference(str) Transaction
        +get_transactions_by_wallet(str) List~Transaction~
        +update_transaction_status(str, TransactionStatus) Transaction
    }

    class TransactionService {
        -TransactionRepository repository
        -RedisClient redis
        -KafkaProducer kafka
        -MongoDB mongodb
        +create_transaction(TransactionCreate) TransactionResponse
        +get_transaction(str) TransactionResponse
        +process_transfer(Transaction) void
        +process_deposit(Transaction) void
        +process_withdrawal(Transaction) void
    }

    %% Shared Components
    class RedisClient {
        -Redis client
        +get(str) Any
        +set(str, Any, int) bool
        +delete(str) bool
        +increment(str, int) int
    }

    class KafkaProducerClient {
        -KafkaProducer producer
        +send_event(str, str, dict) bool
        +close() void
    }

    class KafkaConsumerClient {
        -KafkaConsumer consumer
        -list topics
        +consume_events(Callable) void
        +close() void
    }

    class MongoDB {
        -MongoClient client
        -Database db
        +connect() void
        +get_collection(str) Collection
        +disconnect() void
    }

    %% API Gateway
    class APIGateway {
        -RateLimiter rate_limiter
        -RedisClient redis
        +proxy_request(str, str, Request) Response
        +verify_token(Request) dict
        +rate_limit_middleware(Request) Response
    }

    class RateLimiter {
        -RedisClient redis
        +check_rate_limit(str, int, int) bool
    }

    %% Relationships
    UserService --> UserRepository : uses
    UserService --> RedisClient : uses
    UserService --> KafkaProducerClient : uses
    UserRepository --> User : manages

    WalletService --> WalletRepository : uses
    WalletService --> RedisClient : uses
    WalletService --> KafkaProducerClient : uses
    WalletRepository --> Wallet : manages

    TransactionService --> TransactionRepository : uses
    TransactionService --> RedisClient : uses
    TransactionService --> KafkaProducerClient : uses
    TransactionService --> MongoDB : uses
    TransactionRepository --> Transaction : manages

    APIGateway --> RateLimiter : uses
    APIGateway --> RedisClient : uses
    RateLimiter --> RedisClient : uses

    User "1" --> "*" Wallet : owns
    Transaction "*" --> "1" Wallet : from/to
```

## Entity Relationships

### User ↔ Wallet
- One user can have multiple wallets (different currencies)
- Each wallet belongs to exactly one user

### Wallet ↔ Transaction
- Transactions can involve one or two wallets
- Each transaction references source and/or destination wallet

### Service Dependencies
- All services use RedisClient for caching
- All services use KafkaProducerClient for event streaming
- Transaction Service additionally uses MongoDB for audit logging
