# Architecture Diagrams

This document contains detailed ASCII diagrams explaining the system architecture.

## 1. Overall System Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          USER INTERFACES                                 │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐│
│  │   Web UI     │  │   REST API   │  │  WebSocket   │  │   Mobile    ││
│  │  Dashboard   │  │   Endpoints  │  │  Streaming   │  │     App     ││
│  └──────────────┘  └──────────────┘  └──────────────┘  └─────────────┘│
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                        APPLICATION SERVICES                              │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │                    HFT SERVICES                                   │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │  │
│  │  │   Signal    │  │    Risk     │  │   Order     │             │  │
│  │  │  Generator  │  │  Controller │  │  Executor   │             │  │
│  │  └─────────────┘  └─────────────┘  └─────────────┘             │  │
│  └──────────────────────────────────────────────────────────────────┘  │
│                                                                          │
│  ┌──────────────────────────────────────────────────────────────────┐  │
│  │              INVESTMENT BANKING SERVICES                          │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │  │
│  │  │  Portfolio  │  │   Credit    │  │    Deal     │             │  │
│  │  │  Optimizer  │  │    Risk     │  │  Analysis   │             │  │
│  │  └─────────────┘  └─────────────┘  └─────────────┘             │  │
│  └──────────────────────────────────────────────────────────────────┘  │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                         AI MODEL LAYER                                   │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌────────────┐ │
│  │ Transformers │  │     LLMs     │  │  Generative  │  │  Classical │ │
│  │              │  │              │  │   Models     │  │     ML     │ │
│  │  • TimeGPT   │  │  • GPT-4     │  │   • VAE      │  │  • XGBoost │ │
│  │  • BERT      │  │  • Claude    │  │   • GAN      │  │  • LightGBM│ │
│  │  • LSTM/GRU  │  │  • LLaMA     │  │   • Diffusion│  │  • RF      │ │
│  └──────────────┘  └──────────────┘  └──────────────┘  └────────────┘ │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                      FEATURE ENGINEERING                                 │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌────────────┐ │
│  │  Technical   │  │     NLP      │  │  Fundamental │  │Alternative │ │
│  │  Indicators  │  │   Features   │  │   Features   │  │   Data     │ │
│  │              │  │              │  │              │  │            │ │
│  │  • MA/EMA    │  │  • Sentiment │  │  • Ratios    │  │ • Social   │ │
│  │  • RSI/MACD  │  │  • Entities  │  │  • Metrics   │  │ • Satellite│ │
│  │  • Bollinger │  │  • Topics    │  │  • Growth    │  │ • Web      │ │
│  └──────────────┘  └──────────────┘  └──────────────┘  └────────────┘ │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                         DATA INGESTION                                   │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌────────────┐ │
│  │Market Data   │  │    News      │  │  Financial   │  │Alternative │ │
│  │              │  │              │  │  Documents   │  │   Data     │ │
│  │ • Exchanges  │  │ • Reuters    │  │  • 10-K/Q    │  │ • Twitter  │ │
│  │ • Vendors    │  │ • Bloomberg  │  │  • 8-K       │  │ • Reddit   │ │
│  │ • Direct     │  │ • Benzinga   │  │  • Earnings  │  │ • Satellite│ │
│  └──────────────┘  └──────────────┘  └──────────────┘  └────────────┘ │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                         STORAGE LAYER                                    │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌────────────┐ │
│  │  Time-Series │  │   Document   │  │   Feature    │  │   Model    │ │
│  │   Database   │  │    Store     │  │    Cache     │  │   Store    │ │
│  │              │  │              │  │              │  │            │ │
│  │ TimescaleDB  │  │   MongoDB    │  │    Redis     │  │  MLflow    │ │
│  └──────────────┘  └──────────────┘  └──────────────┘  └────────────┘ │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

## 2. HFT Signal Generation Pipeline

```
Market Event
     │
     ▼
┌─────────────────────────────────────────┐
│  Market Data Handler (1-5 μs)           │
│  • Parse protocol (FIX/FAST)            │
│  • Normalize data                        │
│  • Update order book                     │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Feature Extraction (5-10 μs)           │
│  • Order imbalance                       │
│  • Microprice                            │
│  • Spread                                │
│  • Volume indicators                     │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Model Inference (30-50 μs)             │
│  • Load cached features                  │
│  • Quantized model (INT8)                │
│  • GPU acceleration                      │
│  • Generate prediction                   │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Signal Generation (10-20 μs)           │
│  • Apply business logic                  │
│  • Position sizing                       │
│  • Signal confidence                     │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Risk Controls (10-20 μs)               │
│  • Position limits                       │
│  • Notional limits                       │
│  • Loss limits                           │
│  • Concentration checks                  │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Order Execution (5-10 μs)              │
│  • Create order message                  │
│  • Route to exchange                     │
│  • Confirm execution                     │
└─────────────────────────────────────────┘
     │
     ▼
  Order Sent

Total Latency: 60-100 μs
```

## 3. Portfolio Optimization Workflow

```
Investment Universe
     │
     ▼
┌─────────────────────────────────────────┐
│  Data Collection                         │
│  • Historical prices                     │
│  • Fundamental data                      │
│  • Alternative data                      │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Feature Engineering                     │
│  • Returns calculation                   │
│  • Risk metrics                          │
│  • Factor exposures                      │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  AI-Powered Prediction                   │
│  • Expected returns (LLM + ML)          │
│  • Covariance matrix (ML)               │
│  • Regime detection                      │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Optimization Engine                     │
│  ┌─────────────────────────────────────┐│
│  │ Choose Method:                      ││
│  │  • Mean-Variance                    ││
│  │  • Risk Parity                      ││
│  │  • Minimum Variance                 ││
│  │  • Maximum Sharpe                   ││
│  │  • Black-Litterman                  ││
│  └─────────────────────────────────────┘│
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Constraint Application                  │
│  • Position limits                       │
│  • Sector limits                         │
│  • Risk limits                           │
│  • Regulatory constraints                │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Optimal Allocation                      │
│  • Asset weights                         │
│  • Dollar amounts                        │
│  • Rebalancing trades                    │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Performance Attribution                 │
│  • Risk decomposition                    │
│  • Factor attribution                    │
│  • Contribution analysis                 │
└─────────────────────────────────────────┘
```

## 4. Credit Risk Assessment Flow

```
Company Information
     │
     ▼
┌─────────────────────────────────────────┐
│  Document Collection                     │
│  • Financial statements (10-K/Q)        │
│  • Earnings calls                        │
│  • News articles                         │
│  • Industry reports                      │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  LLM Processing                          │
│  • Document parsing                      │
│  • Entity extraction                     │
│  • Risk factor identification            │
│  • Sentiment analysis                    │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Financial Analysis                      │
│  • Ratio calculation                     │
│    - Profitability (ROA, ROE)           │
│    - Leverage (D/E, Coverage)           │
│    - Liquidity (Current, Quick)         │
│    - Efficiency (Turnover)              │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Feature Engineering                     │
│  • Normalized ratios                     │
│  • Sentiment scores                      │
│  • Industry factors                      │
│  • Macro indicators                      │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Deep Learning Model                     │
│  • Default prediction                    │
│  • Probability of default (PD)          │
│  • Confidence estimation                 │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Credit Score Calculation                │
│  • PD contribution (40%)                 │
│  • Financial ratios (60%)               │
│  • Score: 0-1000                        │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Rating Assignment                       │
│  • Map score to rating (AAA-D)          │
│  • Estimate LGD                         │
│  • Calculate expected loss              │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Report Generation (LLM)                 │
│  • Executive summary                     │
│  • Key risk factors                      │
│  • Strengths & weaknesses               │
│  • Recommendation                        │
└─────────────────────────────────────────┘
```

## 5. Transformer Model Architecture (TimeGPT)

```
Input: Time Series [T-100, T-99, ..., T-1]
     │
     ▼
┌─────────────────────────────────────────┐
│  Embedding Layer                         │
│  • Map features to dense vectors        │
│  • Dimension: 128 → 512                 │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Positional Encoding                     │
│  • Add temporal information             │
│  • Sin/Cos functions                    │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Multi-Head Attention (x6 layers)       │
│  ┌─────────────────────────────────────┐│
│  │  Attention Head 1                   ││
│  │  Query × Key → Attention Weights    ││
│  │  Weights × Value → Output           ││
│  ├─────────────────────────────────────┤│
│  │  Attention Head 2                   ││
│  │  ...                                ││
│  ├─────────────────────────────────────┤│
│  │  Attention Head 8                   ││
│  └─────────────────────────────────────┘│
│                                          │
│  • Identifies important patterns        │
│  • Learns dependencies                  │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Feed-Forward Network                    │
│  • Linear(512 → 2048)                   │
│  • GELU Activation                      │
│  • Linear(2048 → 512)                   │
│  • Dropout(0.1)                         │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Layer Normalization                     │
│  • Stabilize training                   │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Output Heads                            │
│  ┌─────────────────────────────────────┐│
│  │ Direction Head                      ││
│  │ Linear(512 → 3)                     ││
│  │ Output: [DOWN, NEUTRAL, UP]         ││
│  └─────────────────────────────────────┘│
│  ┌─────────────────────────────────────┐│
│  │ Magnitude Head                      ││
│  │ Linear(512 → 1)                     ││
│  │ Output: Expected % change           ││
│  └─────────────────────────────────────┘│
└─────────────────────────────────────────┘
     │
     ▼
Prediction at Time T
```

## 6. Data Flow Diagram

```
External Data Sources
     │
     ├─── Market Data Feeds ────────────┐
     │    • NYSE, NASDAQ, CME           │
     │    • Direct connections          │
     │                                  │
     ├─── News APIs ───────────────────┤
     │    • Reuters, Bloomberg          │
     │    • REST/WebSocket              │
     │                                  │
     ├─── Financial Docs ──────────────┤
     │    • EDGAR (SEC filings)         │
     │    • Company websites            │
     │                                  │
     └─── Alternative Data ────────────┘
          • Twitter, Reddit
          • Web scraping

     ▼           ▼           ▼
┌──────────┐ ┌──────────┐ ┌──────────┐
│ Ingestion│ │ Ingestion│ │ Ingestion│
│ Service  │ │ Service  │ │ Service  │
│    1     │ │    2     │ │    3     │
└──────────┘ └──────────┘ └──────────┘
     │           │           │
     └───────────┴───────────┘
                 │
                 ▼
     ┌───────────────────────┐
     │  Message Queue        │
     │  (Kafka/RabbitMQ)     │
     └───────────────────────┘
                 │
     ┌───────────┴───────────┐
     │                       │
     ▼                       ▼
┌──────────┐           ┌──────────┐
│Raw Data  │           │Raw Data  │
│Storage   │           │Processor │
│(S3/HDFS) │           │          │
└──────────┘           └──────────┘
                            │
                            ▼
                  ┌─────────────────┐
                  │Feature Store    │
                  │(Redis/Feast)    │
                  └─────────────────┘
                            │
     ┌──────────────────────┴──────────────────────┐
     │                                              │
     ▼                                              ▼
┌──────────┐                                  ┌──────────┐
│  Model   │                                  │  Real-   │
│ Training │                                  │  Time    │
│ Pipeline │                                  │ Inference│
└──────────┘                                  └──────────┘
     │                                              │
     ▼                                              ▼
┌──────────┐                                  ┌──────────┐
│  Model   │                                  │  Trading │
│ Registry │                                  │  System  │
└──────────┘                                  └──────────┘
```

## 7. Deployment Architecture (Production)

```
                        Internet
                           │
                           ▼
                    ┌──────────────┐
                    │     WAF      │
                    │ + DDoS Prot  │
                    └──────────────┘
                           │
                           ▼
                    ┌──────────────┐
                    │     CDN      │
                    │  CloudFront  │
                    └──────────────┘
                           │
                           ▼
┌────────────────────────────────────────────────────────┐
│                   Load Balancer                         │
│                  (Application LB)                       │
└────────────────────────────────────────────────────────┘
                           │
         ┌─────────────────┴─────────────────┐
         │                                   │
         ▼                                   ▼
┌─────────────────┐                 ┌─────────────────┐
│  Auto Scaling   │                 │  Auto Scaling   │
│     Group 1     │                 │     Group 2     │
│                 │                 │                 │
│  API Services   │                 │  HFT Services   │
│  (Fargate)      │                 │  (EC2 GPU)      │
│                 │                 │                 │
│  ┌───────────┐  │                 │  ┌───────────┐  │
│  │Container 1│  │                 │  │Container 1│  │
│  ├───────────┤  │                 │  ├───────────┤  │
│  │Container 2│  │                 │  │Container 2│  │
│  ├───────────┤  │                 │  ├───────────┤  │
│  │Container 3│  │                 │  │Container 3│  │
│  └───────────┘  │                 │  └───────────┘  │
└─────────────────┘                 └─────────────────┘
         │                                   │
         └─────────────────┬─────────────────┘
                          │
                          ▼
┌────────────────────────────────────────────────────────┐
│                   Data Layer                            │
├────────────────────────────────────────────────────────┤
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐ │
│  │   RDS        │  │ TimescaleDB  │  │ElastiCache  │ │
│  │  PostgreSQL  │  │              │  │   Redis     │ │
│  │  (Multi-AZ)  │  │  (Replicas)  │  │  (Cluster)  │ │
│  └──────────────┘  └──────────────┘  └─────────────┘ │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐ │
│  │   MongoDB    │  │      S3      │  │   MLflow    │ │
│  │  (Sharded)   │  │  (Documents) │  │   Server    │ │
│  └──────────────┘  └──────────────┘  └─────────────┘ │
│                                                         │
└────────────────────────────────────────────────────────┘
                          │
                          ▼
┌────────────────────────────────────────────────────────┐
│              Monitoring & Observability                 │
├────────────────────────────────────────────────────────┤
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐ │
│  │ Prometheus   │  │   Grafana    │  │  CloudWatch │ │
│  │  (Metrics)   │  │ (Dashboards) │  │   (Logs)    │ │
│  └──────────────┘  └──────────────┘  └─────────────┘ │
│                                                         │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐ │
│  │   Jaeger     │  │  PagerDuty   │  │   MLflow    │ │
│  │  (Tracing)   │  │   (Alerts)   │  │(Experiments)│ │
│  └──────────────┘  └──────────────┘  └─────────────┘ │
│                                                         │
└────────────────────────────────────────────────────────┘
```

## 8. Model Training Pipeline

```
Historical Data
     │
     ▼
┌─────────────────────────────────────────┐
│  Data Validation                         │
│  • Check completeness                   │
│  • Identify outliers                    │
│  • Verify consistency                   │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Feature Engineering                     │
│  • Technical indicators                  │
│  • NLP features                         │
│  • Custom features                      │
└─────────────────────────────────────────┘
     │
     ▼
┌─────────────────────────────────────────┐
│  Train/Val/Test Split                   │
│  • Temporal split (important!)         │
│  • 70% train / 15% val / 15% test      │
└─────────────────────────────────────────┘
     │
     ├─── Training Set
     │         │
     │         ▼
     │    ┌─────────────────────────────┐
     │    │  Model Training             │
     │    │  • Initialize model         │
     │    │  • Forward/backward pass    │
     │    │  • Update weights           │
     │    │  • Track metrics            │
     │    └─────────────────────────────┘
     │         │
     ├─── Validation Set
     │         │
     │         ▼
     │    ┌─────────────────────────────┐
     │    │  Validation                 │
     │    │  • Evaluate performance     │
     │    │  • Prevent overfitting      │
     │    └─────────────────────────────┘
     │         │
     │         ▼
     │    ┌─────────────────────────────┐
     │    │  Hyperparameter Tuning      │
     │    │  • Grid search              │
     │    │  • Random search            │
     │    │  • Bayesian optimization    │
     │    └─────────────────────────────┘
     │         │
     │         ▼
     │    ┌─────────────────────────────┐
     │    │  Model Selection            │
     │    │  • Best validation score    │
     │    └─────────────────────────────┘
     │         │
     └─── Test Set
               │
               ▼
          ┌─────────────────────────────┐
          │  Final Evaluation           │
          │  • Unbiased performance     │
          │  • Production readiness     │
          └─────────────────────────────┘
               │
               ▼
          ┌─────────────────────────────┐
          │  Model Export               │
          │  • SaveTorchScript          │
          │  • ONNX format              │
          │  • Quantization             │
          └─────────────────────────────┘
               │
               ▼
          ┌─────────────────────────────┐
          │  Model Registry             │
          │  • Version control          │
          │  • Metadata                 │
          │  • MLflow                   │
          └─────────────────────────────┘
```

These diagrams provide a comprehensive visual understanding of the system architecture, data flows, and model pipelines for both HFT and Investment Banking applications.
