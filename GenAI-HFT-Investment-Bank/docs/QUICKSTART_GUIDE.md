# Quick Start Guide

Get up and running with GenAI HFT & Investment Banking in 10 minutes!

## Prerequisites

- Python 3.9 or higher
- 8GB+ RAM (16GB+ recommended)
- Optional: NVIDIA GPU with CUDA for faster training

## Installation

### Step 1: Clone and Setup

```bash
# Navigate to the project directory
cd GenAI-HFT-Investment-Bank

# Create virtual environment
python -m venv venv

# Activate virtual environment
# On Linux/Mac:
source venv/bin/activate
# On Windows:
# venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Install the package
pip install -e .
```

### Step 2: Verify Installation

```bash
# Test import
python -c "import src; print('Installation successful!')"
```

## Running Your First Example

### Example 1: Market Prediction

```bash
# Run the basic market prediction example
python examples/01_market_prediction_basic.py
```

**What it does:**
- Generates synthetic market data
- Trains a transformer model
- Makes predictions
- Evaluates performance

**Expected output:**
```
Example 1: Basic Market Prediction with Generative AI
======================================================================

[Step 1] Generating market data...
  ✓ Generated 500 days of data
  ✓ Price range: $95.23 - $154.67

[Step 2] Calculating technical features...
  ✓ Created 400 feature vectors
  ✓ Feature dimension: 128

...

RESULTS
======================================================================
  Accuracy: 58.33%
  Correct predictions: 35/60

  Sample Predictions:
  Actual     Predicted  Confidence
  -----------------------------------
  UP         UP         68.3%
  DOWN       DOWN       71.2%
  NEUTRAL    HOLD       54.8%
```

### Example 2: HFT Signal Generation

```bash
# Run the HFT signal generation example
python examples/02_hft_signal_generation.py
```

**What it does:**
- Simulates order book data
- Extracts HFT features
- Generates trading signals
- Analyzes performance

**Expected output:**
```
Example 2: High-Frequency Trading Signal Generation
======================================================================

[Step 1] Understanding Order Book Features...

  Sample Order Book:
  Top 5 Bids:
    Level 1: $100.00 x 1,234
    Level 2: $99.99 x 1,856
    ...

  Extracted Features:
    Spread              : 0.0010
    Microprice          : 100.0050
    Order Imbalance     : 0.0523
```

### Example 3: Portfolio Optimization

```bash
# Run the portfolio optimization example
python examples/03_portfolio_optimization.py
```

**What it does:**
- Creates investment universe
- Optimizes portfolio allocation
- Compares different strategies
- Generates recommendations

**Expected output:**
```
Example 3: AI-Powered Portfolio Optimization
======================================================================

STRATEGY COMPARISON
======================================================================

  Strategy              Sharpe     Return       Risk
  ------------------------------------------------------------
  Equal Weight          1.23       N/A          N/A
  Maximum Sharpe        1.85       14.23        10.52
  Minimum Variance      1.42       8.67         7.21
  Risk Parity          1.56       11.34        9.12

  🏆 Best Strategy: Maximum Sharpe (Sharpe: 1.85)
```

## Understanding the Output

### Model Accuracy
- **55-60%**: Good for financial markets (better than random)
- **60-65%**: Excellent performance
- **65%+**: Outstanding (verify for overfitting)

### Sharpe Ratio
- **< 1.0**: Below market performance
- **1.0-2.0**: Good risk-adjusted returns
- **> 2.0**: Excellent risk-adjusted returns

### Latency (HFT)
- **< 100μs**: Excellent for HFT
- **100-200μs**: Acceptable
- **> 200μs**: Needs optimization

## Next Steps

### 1. Learn the Fundamentals
Read the documentation in order:
```
docs/
├── 00_INTRODUCTION.md          ← Start here
├── 01_ARCHITECTURE.md          ← System design
├── 02_MODELS.md                ← AI models
├── 03_HFT_GUIDE.md            ← HFT deep dive
└── 04_INVESTMENT_BANKING.md   ← IB applications
```

### 2. Customize for Your Use Case

#### For HFT:
1. Edit `configs/hft_config.yaml`
2. Add your trading symbols
3. Connect real market data
4. Train models on historical data
5. Run backtests

#### For Investment Banking:
1. Edit `configs/investment_banking_config.yaml`
2. Define your investment universe
3. Set risk parameters
4. Run portfolio optimization
5. Generate client reports

### 3. Work with Real Data

Replace sample data with real data:

```python
# Using yfinance
import yfinance as yf

# Download data
data = yf.download('AAPL', start='2020-01-01', end='2023-12-31')

# Or use Alpha Vantage
from alpha_vantage.timeseries import TimeSeries

ts = TimeSeries(key='YOUR_API_KEY')
data, meta = ts.get_daily('AAPL', outputsize='full')
```

### 4. Train Custom Models

```python
from src.models.market_predictor import MarketPredictor

# Create predictor
predictor = MarketPredictor(model_type='transformer')

# Prepare your data
# train_features: shape [num_samples, sequence_length, num_features]
# train_labels: shape [num_samples, 2] (direction, magnitude)

# Train
predictor.train(
    train_data=train_features,
    train_labels=train_labels,
    val_data=val_features,
    val_labels=val_labels,
    epochs=100,
    batch_size=64,
    learning_rate=0.001
)

# Save model
predictor.save_model('my_model.pth')
```

### 5. Deploy to Production

See `docs/07_DEPLOYMENT.md` for detailed deployment guide.

Quick deployment checklist:
- [ ] Train models on sufficient data (2+ years)
- [ ] Backtest thoroughly
- [ ] Set up monitoring
- [ ] Configure alerts
- [ ] Implement risk controls
- [ ] Start with paper trading
- [ ] Gradually increase capital

## Common Issues

### Issue: CUDA not available
**Solution:** Install with CPU support or install CUDA toolkit
```bash
# Check CUDA availability
python -c "import torch; print(torch.cuda.is_available())"

# For CPU-only installation
pip install torch torchvision --index-url https://download.pytorch.org/whl/cpu
```

### Issue: TA-Lib installation fails
**Solution:** Install TA-Lib C library first
```bash
# On Ubuntu/Debian
sudo apt-get install ta-lib

# On Mac
brew install ta-lib

# On Windows
# Download from: https://github.com/mrjbq7/ta-lib
```

### Issue: Out of memory during training
**Solution:** Reduce batch size
```python
predictor.train(
    train_data=train_features,
    train_labels=train_labels,
    epochs=100,
    batch_size=16,  # Reduced from 32 or 64
    learning_rate=0.001
)
```

### Issue: Model accuracy is low
**Solution:**
1. Train longer (more epochs)
2. Add more features
3. Use more historical data
4. Try different model architectures

## Getting Help

### Documentation
- Full documentation in `docs/` directory
- API reference: `docs/06_API_REFERENCE.md`
- Troubleshooting: `docs/08_TROUBLESHOOTING.md`

### Community
- GitHub Issues: Report bugs and request features
- Discussions: Ask questions and share ideas

### Professional Support
For enterprise deployments and custom development, contact the team.

## Example Workflows

### Workflow 1: Train a Trading Model

```python
# 1. Load data
from src.data.data_loader import MarketDataLoader
loader = MarketDataLoader()
data = loader.load('AAPL', start='2020-01-01', end='2023-12-31')

# 2. Create features
from src.features.technical import TechnicalFeatures
features = TechnicalFeatures.calculate(data)

# 3. Train model
from src.models.market_predictor import MarketPredictor
predictor = MarketPredictor(model_type='transformer')
predictor.train(features['train'], labels['train'])

# 4. Backtest
from src.backtesting.engine import BacktestEngine
engine = BacktestEngine()
results = engine.run(predictor, data['test'])

# 5. Deploy
predictor.save_model('production_model.pth')
```

### Workflow 2: Optimize a Portfolio

```python
# 1. Define universe
symbols = ['AAPL', 'GOOGL', 'MSFT', 'AMZN', 'TSLA']

# 2. Load data
from src.data.data_loader import PortfolioDataLoader
loader = PortfolioDataLoader()
data = loader.load_multiple(symbols)

# 3. Optimize
from src.investment_banking.portfolio_optimizer import AIPortfolioOptimizer
optimizer = AIPortfolioOptimizer(universe=symbols, method='maximum_sharpe')
allocation = optimizer.optimize(data, capital=1_000_000)

# 4. Generate report
from src.reporting.portfolio_report import PortfolioReport
report = PortfolioReport()
report.generate(allocation, output='portfolio_report.pdf')
```

### Workflow 3: Assess Credit Risk

```python
# 1. Collect company data
from src.data.financial_data import FinancialDataCollector
collector = FinancialDataCollector()
financials = collector.get_financials('AAPL')

# 2. Analyze
from src.investment_banking.credit_risk import CreditRiskAnalyzer
analyzer = CreditRiskAnalyzer()
assessment = analyzer.analyze(
    company_name='Apple Inc.',
    financial_statements=financials,
    industry='technology'
)

# 3. Generate report
print(f"Credit Score: {assessment.credit_score}")
print(f"Rating: {assessment.rating}")
print(f"Recommendation: {assessment.recommendation}")
```

## Success Metrics

Track these metrics to measure success:

### For HFT:
- Latency: < 100μs
- Sharpe Ratio: > 2.0
- Win Rate: > 55%
- Maximum Drawdown: < 10%

### For Investment Banking:
- Portfolio Return: > Benchmark + 3%
- Sharpe Ratio: > 1.5
- Maximum Drawdown: < 15%
- Prediction Accuracy: > 65%

## What's Next?

1. **Week 1**: Run all examples, understand the basics
2. **Week 2**: Customize configurations, add your data
3. **Week 3**: Train models, run backtests
4. **Week 4**: Deploy to paper trading
5. **Week 5+**: Monitor, optimize, scale

---

**Ready to build AI-powered trading systems? Start with `examples/01_market_prediction_basic.py`!**
