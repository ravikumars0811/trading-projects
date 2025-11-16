# System Architecture

This document provides a comprehensive overview of the GenAI HFT & Investment Banking system architecture.

## Table of Contents
1. [High-Level Architecture](#high-level-architecture)
2. [Component Details](#component-details)
3. [Data Flow](#data-flow)
4. [Model Pipeline](#model-pipeline)
5. [Deployment Architecture](#deployment-architecture)

## High-Level Architecture

The system is built on a modular, layered architecture:

```
┌─────────────────────────────────────────────────────────────────────┐
│                         PRESENTATION LAYER                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐             │
│  │   Web UI     │  │   REST API   │  │   WebSocket  │             │
│  │  Dashboard   │  │   Endpoints  │  │   Real-time  │             │
│  └──────────────┘  └──────────────┘  └──────────────┘             │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        APPLICATION LAYER                             │
│  ┌──────────────────────────┐  ┌──────────────────────────┐        │
│  │   HFT Applications       │  │  Investment Banking Apps │        │
│  │  ┌────────────────────┐  │  │  ┌────────────────────┐ │        │
│  │  │ Signal Generator   │  │  │  │ Portfolio Optimizer│ │        │
│  │  │ Risk Manager       │  │  │  │ Credit Analyzer    │ │        │
│  │  │ Order Flow Predict │  │  │  │ Deal Analyzer      │ │        │
│  │  │ Regime Detector    │  │  │  │ Report Generator   │ │        │
│  │  └────────────────────┘  │  │  └────────────────────┘ │        │
│  └──────────────────────────┘  └──────────────────────────┘        │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                         AI MODEL LAYER                               │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐               │
│  │ Transformers │ │     LLMs     │ │   Gen Models │               │
│  │  ┌────────┐  │ │  ┌────────┐  │ │  ┌────────┐  │               │
│  │  │TimeGPT │  │ │  │  GPT   │  │ │  │  VAE   │  │               │
│  │  │ BERT   │  │ │  │ Claude │  │ │  │  GAN   │  │               │
│  │  │LSTM/GRU│  │ │  │ LLaMA  │  │ │  │ Diffusion│ │               │
│  │  └────────┘  │ │  └────────┘  │ │  └────────┘  │               │
│  └──────────────┘ └──────────────┘ └──────────────┘               │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      FEATURE ENGINEERING LAYER                       │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐               │
│  │  Technical   │ │     NLP      │ │ Alternative  │               │
│  │  Indicators  │ │   Features   │ │   Features   │               │
│  │  - RSI       │ │ - Sentiment  │ │ - Social     │               │
│  │  - MACD      │ │ - Entities   │ │ - Satellite  │               │
│  │  - Bollinger │ │ - Topics     │ │ - Web scrape │               │
│  └──────────────┘ └──────────────┘ └──────────────┘               │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        DATA INGESTION LAYER                          │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐     │
│  │ Market  │ │  News   │ │Financial│ │Alternate│ │ Internal│     │
│  │  Data   │ │  Feed   │ │   Docs  │ │  Data   │ │  Data   │     │
│  │         │ │         │ │         │ │         │ │         │     │
│  │- Prices │ │- Reuters│ │- 10-K   │ │- Twitter│ │- Trades │     │
│  │- Volume │ │- Bloomberg│- 10-Q  │ │- Reddit │ │- Orders │     │
│  │- Orderbook│-Benzinga│ │- 8-K   │ │- Satellite│- Positions│    │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘     │
└─────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                         STORAGE LAYER                                │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐               │
│  │  Time-Series │ │   Document   │ │    Feature   │               │
│  │     DB       │ │     Store    │ │     Store    │               │
│  │ (TimescaleDB)│ │  (MongoDB)   │ │   (Redis)    │               │
│  └──────────────┘ └──────────────┘ └──────────────┘               │
└─────────────────────────────────────────────────────────────────────┘
```

## Component Details

### 1. Data Ingestion Layer

**Purpose:** Collect data from multiple sources in real-time

**Components:**

#### Market Data Collector
```python
class MarketDataCollector:
    """
    Ingests real-time and historical market data

    Sources:
    - Exchange feeds (Direct market access)
    - Vendor APIs (Bloomberg, Refinitiv)
    - Websockets (Binance, Coinbase for crypto)

    Output: Normalized market data stream
    """

    def __init__(self, sources=['NYSE', 'NASDAQ', 'CME']):
        self.sources = sources
        self.buffer = CircularBuffer(size=1000000)

    async def stream_data(self):
        # Collects tick-by-tick data
        # Latency: < 1 microsecond
        pass
```

**Key Features:**
- Multi-source aggregation
- Real-time normalization
- Quality checks
- Gap detection and handling

#### News Feed Processor
```python
class NewsFeedProcessor:
    """
    Processes news from multiple sources

    Capabilities:
    - Real-time news ingestion
    - Deduplication
    - Entity extraction
    - Timestamp normalization
    """

    def process_news(self, article):
        # Extract entities (companies, people, events)
        entities = self.ner_model.extract(article)

        # Determine relevance
        relevance = self.classifier.score(article)

        # Extract sentiment
        sentiment = self.sentiment_model.analyze(article)

        return {
            'entities': entities,
            'relevance': relevance,
            'sentiment': sentiment,
            'timestamp': article.timestamp
        }
```

#### Document Processor
```python
class FinancialDocumentProcessor:
    """
    Processes financial documents (10-K, 10-Q, earnings calls)

    Capabilities:
    - PDF/HTML parsing
    - Table extraction
    - Section identification
    - Key metric extraction
    """

    def process_10k(self, document):
        # Extract sections
        sections = self.extract_sections(document)

        # Parse financials
        financials = self.table_parser.parse(sections['financials'])

        # Extract risk factors
        risks = self.llm.extract_risks(sections['risk_factors'])

        return structured_data
```

### 2. Feature Engineering Layer

**Purpose:** Transform raw data into features that AI models can learn from

**Components:**

#### Technical Indicators
```python
class TechnicalFeatures:
    """
    Calculates technical indicators

    Indicators:
    - Trend: MA, EMA, MACD
    - Momentum: RSI, Stochastic, ROC
    - Volatility: Bollinger Bands, ATR
    - Volume: OBV, VWAP
    """

    def calculate_features(self, price_data):
        features = {
            # Trend
            'sma_20': price_data.rolling(20).mean(),
            'ema_12': price_data.ewm(span=12).mean(),
            'macd': self.calculate_macd(price_data),

            # Momentum
            'rsi': self.calculate_rsi(price_data, period=14),
            'stochastic': self.calculate_stochastic(price_data),

            # Volatility
            'bollinger': self.calculate_bollinger_bands(price_data),
            'atr': self.calculate_atr(price_data),

            # Volume
            'obv': self.calculate_obv(price_data, volume),
            'vwap': self.calculate_vwap(price_data, volume)
        }
        return features
```

#### NLP Features
```python
class NLPFeatures:
    """
    Extracts features from text data

    Features:
    - Sentiment scores
    - Entity mentions
    - Topic modeling
    - Embeddings
    """

    def extract_nlp_features(self, text_data):
        # Sentiment analysis
        sentiment = self.sentiment_model.predict(text_data)

        # Named entity recognition
        entities = self.ner_model.extract(text_data)

        # Topic modeling
        topics = self.topic_model.transform(text_data)

        # Generate embeddings
        embeddings = self.embedding_model.encode(text_data)

        return {
            'sentiment': sentiment,
            'entities': entities,
            'topics': topics,
            'embeddings': embeddings
        }
```

### 3. AI Model Layer

**Purpose:** Core intelligence of the system

**Model Categories:**

#### Transformer Models
```
Architecture: TimeGPT for Financial Time Series

Input Sequence (T-100 to T-1)
         │
         ▼
┌──────────────────┐
│ Embedding Layer  │  ← Convert to dense vectors
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Positional       │  ← Add time information
│ Encoding         │
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Multi-Head       │  ← Attention mechanism
│ Attention (x6)   │    (What patterns matter?)
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Feed-Forward     │  ← Process patterns
│ Network          │
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Output Layer     │  ← Generate prediction
└──────────────────┘
         │
         ▼
  Prediction (T)
```

**Why Transformers for Finance?**
- Capture long-range dependencies
- Parallel processing (fast!)
- Handle multiple time scales
- Attention mechanism highlights important patterns

#### Large Language Models
```
LLM Pipeline for Financial Analysis

Financial Document (10-K Report)
         │
         ▼
┌──────────────────┐
│ Document         │  ← Split into chunks
│ Chunking         │
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Embedding        │  ← Convert to vectors
│ Generation       │
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Vector Store     │  ← Store for retrieval
│ (FAISS)          │
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ LLM Processing   │  ← Analyze with Claude/GPT
│ (RAG)            │
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Structured       │  ← Extract insights
│ Output           │
└──────────────────┘
```

**Applications:**
- Earnings call analysis
- Credit risk assessment
- Investment thesis generation
- Regulatory compliance checking

### 4. Application Layer

**Purpose:** Domain-specific business logic

#### HFT Signal Generator

```
Signal Generation Pipeline

Market Data Stream
         │
         ▼
┌──────────────────┐
│ Feature          │  ← Real-time feature calculation
│ Calculation      │    (latency: <10μs)
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Model Inference  │  ← Run prediction
│ (Quantized)      │    (latency: <50μs)
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Risk Checks      │  ← Validate signal
└──────────────────┘    (latency: <20μs)
         │
         ▼
┌──────────────────┐
│ Order Generation │  ← Create order
└──────────────────┘    (latency: <10μs)
         │
         ▼
    Total Latency: <100μs
```

**Optimization Techniques:**
- Model quantization (INT8)
- GPU inference
- Feature caching
- Batch processing

#### Portfolio Optimizer

```
Portfolio Optimization Flow

Investment Universe
         │
         ▼
┌──────────────────┐
│ Return           │  ← AI predicts expected returns
│ Prediction       │
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Risk Modeling    │  ← AI estimates covariance
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Constraint       │  ← Apply constraints
│ Definition       │    - Max position size
└──────────────────┘    - Sector limits
         │              - Liquidity
         ▼
┌──────────────────┐
│ Optimization     │  ← Solve for optimal weights
│ (Quadratic Prog) │
└──────────────────┘
         │
         ▼
┌──────────────────┐
│ Portfolio        │  ← Output allocation
│ Weights          │
└──────────────────┘
```

## Data Flow

### Real-Time Data Flow (HFT)

```
Exchange → Market Data Handler → Feature Engine → Model → Signal → Order
  (1μs)         (5μs)              (10μs)        (50μs)  (20μs)   (10μs)

Total Latency: ~100 microseconds
```

**Detailed Breakdown:**

1. **Exchange** (1μs)
   - Market data published
   - Transmitted via direct connection

2. **Market Data Handler** (5μs)
   - Parse FIX/FAST protocol
   - Normalize data format
   - Update order book

3. **Feature Engine** (10μs)
   - Calculate technical indicators
   - Update feature vector
   - Cache for model

4. **Model Inference** (50μs)
   - Load cached features
   - Run quantized model
   - Generate prediction

5. **Signal Generation** (20μs)
   - Apply business logic
   - Risk checks
   - Position sizing

6. **Order Generation** (10μs)
   - Create order message
   - Route to exchange

### Batch Processing Flow (Investment Banking)

```
Day 1: Data Collection
  ↓
Day 2: Feature Engineering
  ↓
Day 3: Model Training
  ↓
Day 4: Backtesting
  ↓
Day 5: Deployment

Then: Daily predictions
```

## Model Pipeline

### Training Pipeline

```
Historical Data
      │
      ▼
┌─────────────────┐
│ Data Cleaning   │  ← Remove outliers, handle missing data
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Feature         │  ← Calculate all features
│ Engineering     │
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Train/Val Split │  ← Time-based split (important!)
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Model Training  │  ← Train on training set
│ (GPU Cluster)   │
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Validation      │  ← Evaluate on validation set
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Hyperparameter  │  ← Optimize hyperparameters
│ Tuning          │
└─────────────────┘
      │
      ▼
┌─────────────────┐
│ Model Export    │  ← Save for production
└─────────────────┘
```

### Inference Pipeline

```
Real-time Data → Feature Calculation → Model Inference → Post-processing → Output
```

## Deployment Architecture

### Cloud Deployment (AWS Example)

```
┌─────────────────────────────────────────────────────────────┐
│                         CloudFront CDN                       │
│                    (Web UI Distribution)                     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      Application Load Balancer               │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                         ECS Cluster                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │  API Service │  │ HFT Service  │  │  IB Service  │     │
│  │  (Fargate)   │  │  (EC2 GPU)   │  │  (Fargate)   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                        Data Layer                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ RDS Postgres │  │  TimescaleDB │  │ ElastiCache  │     │
│  │  (Metadata)  │  │ (Time-series)│  │   (Redis)    │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

### On-Premise Deployment (Co-location)

```
┌─────────────────────────────────────────────────────────────┐
│                      Trading Floor                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │ Trading      │  │ Risk         │  │ Monitoring   │     │
│  │ Terminals    │  │ Dashboard    │  │ Dashboard    │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Co-location Facility                      │
│  ┌────────────────────────────────────────────────┐         │
│  │           HFT Servers (FPGA/GPU)               │         │
│  │  ┌──────────────┐  ┌──────────────┐           │         │
│  │  │ Market Data  │  │ Trading      │           │         │
│  │  │ Handler      │  │ Engine       │           │         │
│  │  └──────────────┘  └──────────────┘           │         │
│  │         Direct Exchange Connection             │         │
│  └────────────────────────────────────────────────┘         │
└─────────────────────────────────────────────────────────────┘
```

## Performance Characteristics

### Latency Targets

| Component | Latency | Notes |
|-----------|---------|-------|
| Market Data Parsing | <5μs | Critical path |
| Feature Calculation | <10μs | Pre-computed when possible |
| Model Inference | <50μs | Quantized models |
| Risk Checks | <20μs | Rule-based |
| Order Generation | <10μs | Template-based |
| **Total (HFT)** | **<100μs** | End-to-end |
| Investment Banking | ~1s | Not latency-critical |

### Throughput

| System | Throughput | Notes |
|--------|------------|-------|
| HFT Signal Generation | 100K signals/sec | Per symbol |
| Market Data Processing | 10M ticks/sec | Aggregated |
| Model Inference | 50K predictions/sec | Batched |
| Order Processing | 20K orders/sec | Including risk checks |

### Resource Requirements

| Component | CPU | RAM | GPU | Storage |
|-----------|-----|-----|-----|---------|
| HFT Server | 64 cores | 256GB | V100 | 2TB NVMe |
| IB Server | 32 cores | 128GB | Optional | 1TB SSD |
| Database | 16 cores | 64GB | N/A | 10TB HDD |
| ML Training | 96 cores | 512GB | 8xA100 | 50TB |

## Security Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      External Access                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   Traders    │  │   Analysts   │  │  Executives  │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      WAF + DDoS Protection                   │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    API Gateway + Auth                        │
│                  (OAuth 2.0 + JWT Tokens)                    │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
│                    (Encrypted at Rest)                       │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                        Data Layer                            │
│              (Encrypted + Access Logs)                       │
└─────────────────────────────────────────────────────────────┘
```

**Security Features:**
- TLS 1.3 for all communications
- End-to-end encryption
- Role-based access control (RBAC)
- Audit logging
- Secrets management (AWS Secrets Manager)
- Network isolation (VPC)

## Monitoring & Observability

```
Application Metrics → Prometheus → Grafana Dashboard
                                         ↓
                                    Alerting
                                         ↓
                                   PagerDuty

Model Performance → MLflow → Model Registry
                                  ↓
                             Drift Detection
                                  ↓
                            Auto-retraining
```

## Next Steps

Now that you understand the architecture, explore:

1. **Model Details** → `02_MODELS.md`
2. **HFT Implementation** → `03_HFT_GUIDE.md`
3. **IB Implementation** → `04_INVESTMENT_BANKING.md`

---

**Previous:** [Introduction](00_INTRODUCTION.md) | **Next:** [Models Deep Dive](02_MODELS.md)
