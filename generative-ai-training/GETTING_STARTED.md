# Getting Started with Generative AI for Financial Applications

A practical guide to start using AI in your HFT and investment banking projects.

## Table of Contents

1. [Quick Start (5 minutes)](#quick-start)
2. [Understanding the Basics](#understanding-the-basics)
3. [Your First AI Integration](#your-first-ai-integration)
4. [Production Considerations](#production-considerations)
5. [Cost Optimization](#cost-optimization)
6. [Performance Tuning](#performance-tuning)

## Quick Start

### Step 1: Installation

```bash
# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt
```

### Step 2: Configure API Keys

```bash
# Copy environment template
cp .env.example .env

# Edit .env and add your API keys
# Get OpenAI key: https://platform.openai.com/api-keys
# Get Claude key: https://console.anthropic.com/
```

### Step 3: Run Your First Example

```bash
# Test basic tokenization
python module1_llm_fundamentals/01_tokenization.py

# Test API integration
python module2_api_integration/01_openai_basics.py
```

## Understanding the Basics

### What are LLMs?

Large Language Models (LLMs) are AI systems trained on massive amounts of text data. They can:

- **Understand context**: Analyze market news in context
- **Generate text**: Create reports, summaries, explanations
- **Extract information**: Pull key data from documents
- **Reason**: Make inferences and recommendations

### Key Concepts

#### 1. Tokens

Text is broken into tokens (words/subwords). Important because:
- APIs charge per token
- Models have token limits (GPT-4: 128K, Claude: 200K)
- More tokens = higher latency

```python
# Example: Estimate tokens
import tiktoken

encoder = tiktoken.encoding_for_model("gpt-4")
text = "The S&P 500 rose 2.3% today"
tokens = encoder.encode(text)
print(f"Text: {text}")
print(f"Tokens: {len(tokens)}")  # ~8 tokens
print(f"Cost (GPT-4): ${len(tokens) * 0.00003:.6f}")  # ~$0.00024
```

#### 2. Embeddings

Vector representations of text that capture meaning:

```python
from sentence_transformers import SentenceTransformer

model = SentenceTransformer('all-MiniLM-L6-v2')

# Similar texts have similar embeddings
text1 = "Stock market rally"
text2 = "Equities surge higher"
text3 = "Weather forecast"

emb1 = model.encode(text1)
emb2 = model.encode(text2)
emb3 = model.encode(text3)

# text1 and text2 are similar, text3 is different
```

#### 3. Prompt Engineering

How you ask matters! Good prompts get better results.

**Bad Prompt:**
```
Tell me about AAPL
```

**Good Prompt:**
```
Analyze Apple Inc. (AAPL) based on these metrics:
- Revenue: $383.9B (↓3% YoY)
- Services: $85.2B (↑9% YoY)
- Gross Margin: 44.1%

Provide:
1. Financial health assessment
2. Key strengths and risks
3. Investment recommendation with reasoning
```

## Your First AI Integration

### Example 1: News Sentiment Analysis for Trading

```python
import openai
import os
from dotenv import load_dotenv

load_dotenv()

client = openai.OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

def analyze_news_for_trading(news_text, ticker):
    """Analyze news and get trading signal."""

    prompt = f"""
    Analyze this news for {ticker} and provide a trading recommendation.

    News: {news_text}

    Respond with JSON:
    {{
        "sentiment": "BULLISH/BEARISH/NEUTRAL",
        "confidence": 0-100,
        "recommendation": "BUY/SELL/HOLD",
        "reasoning": "brief explanation"
    }}
    """

    response = client.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[{"role": "user", "content": prompt}],
        temperature=0.3,
        response_format={"type": "json_object"}
    )

    import json
    return json.loads(response.choices[0].message.content)

# Test it
news = "Tesla Q3 deliveries beat estimates, stock surges 7%"
result = analyze_news_for_trading(news, "TSLA")

print(f"Sentiment: {result['sentiment']}")
print(f"Recommendation: {result['recommendation']}")
print(f"Confidence: {result['confidence']}%")
print(f"Reasoning: {result['reasoning']}")
```

### Example 2: Document Analysis

```python
import anthropic
import os

client = anthropic.Anthropic(api_key=os.getenv("ANTHROPIC_API_KEY"))

def analyze_10k(filing_text):
    """Analyze 10-K filing."""

    response = client.messages.create(
        model="claude-3-5-sonnet-20241022",
        max_tokens=2048,
        messages=[{
            "role": "user",
            "content": f"""
            Analyze this 10-K filing and extract:
            1. Key financial metrics
            2. Main risk factors
            3. Growth opportunities
            4. Investment recommendation

            10-K: {filing_text[:50000]}
            """
        }]
    )

    return response.content[0].text

# Use Claude for long documents (200K context window)
```

### Example 3: Automated Report Generation

```python
def generate_daily_market_report(market_data):
    """Generate professional market commentary."""

    prompt = f"""
    Generate a professional daily market report.

    Market Data:
    - S&P 500: {market_data['sp500']}%
    - NASDAQ: {market_data['nasdaq']}%
    - Top Gainers: {market_data['gainers']}
    - Top Losers: {market_data['losers']}

    Write a 2-paragraph professional summary for institutional clients.
    """

    response = client.chat.completions.create(
        model="gpt-4-turbo-preview",
        messages=[{"role": "user", "content": prompt}],
        temperature=0.7
    )

    return response.choices[0].message.content
```

## Production Considerations

### 1. Error Handling

Always handle API errors:

```python
import time
from openai import RateLimitError, APIError

def call_with_retry(func, max_retries=3):
    """Call API with exponential backoff."""

    for attempt in range(max_retries):
        try:
            return func()
        except RateLimitError:
            if attempt < max_retries - 1:
                wait = 2 ** attempt
                print(f"Rate limit hit. Waiting {wait}s...")
                time.sleep(wait)
            else:
                raise
        except APIError as e:
            print(f"API error: {e}")
            if attempt < max_retries - 1:
                time.sleep(2 ** attempt)
            else:
                raise
```

### 2. Caching

Cache responses to reduce costs and latency:

```python
from functools import lru_cache
import hashlib

@lru_cache(maxsize=1000)
def cached_analysis(text_hash):
    """Cache analysis results."""
    # Your AI call here
    pass

def analyze_with_cache(text):
    """Analyze with caching."""
    text_hash = hashlib.md5(text.encode()).hexdigest()
    return cached_analysis(text_hash)
```

### 3. Async Operations

Use async for high throughput:

```python
import asyncio

async def analyze_multiple_news(news_items):
    """Process multiple news items concurrently."""

    tasks = [analyze_news_async(item) for item in news_items]
    return await asyncio.gather(*tasks)

# Process 100 news items in parallel instead of sequentially
```

## Cost Optimization

### Strategy 1: Use Appropriate Models

| Task | Model | Cost/1M tokens | Use When |
|------|-------|----------------|----------|
| Simple sentiment | GPT-3.5 | $0.50 | Speed critical, simple task |
| Complex analysis | GPT-4 Turbo | $10.00 | Need accuracy, reasoning |
| Long documents | Claude 3 Sonnet | $3.00 | 200K context needed |
| Bulk processing | GPT-3.5 Turbo | $0.50 | Volume over quality |

### Strategy 2: Optimize Prompts

**Inefficient:**
```python
# Uses 500 tokens
long_prompt = "Please analyze this with great detail..." + context
```

**Efficient:**
```python
# Uses 100 tokens
short_prompt = f"Analyze: {context}. Provide JSON: {{sentiment, score}}"
```

### Strategy 3: Batch Processing

```python
# Instead of 100 API calls:
for item in items:
    analyze(item)  # 100 API calls = expensive

# Do this:
batch_prompt = "Analyze these items:\n" + "\n".join(items)
result = analyze(batch_prompt)  # 1 API call = cheap
```

### Strategy 4: Route by Complexity

```python
def smart_analyze(text):
    """Route to appropriate model."""

    # Simple cases: use cheap model
    if len(text) < 200 and is_simple_sentiment(text):
        return analyze_with_gpt35(text)

    # Complex cases: use expensive model
    else:
        return analyze_with_gpt4(text)
```

## Performance Tuning

### 1. Latency Optimization

For HFT systems where every millisecond matters:

```python
# Parallel processing
async def low_latency_analysis(news_items):
    # Process multiple items simultaneously
    results = await asyncio.gather(*[
        analyze_async(item) for item in news_items
    ])
    return results

# Streaming for faster first response
def stream_analysis(text):
    stream = client.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[{"role": "user", "content": text}],
        stream=True
    )

    for chunk in stream:
        # Process as you receive
        yield chunk.choices[0].delta.content
```

### 2. Monitoring

Track performance metrics:

```python
class AIMetrics:
    def __init__(self):
        self.latencies = []
        self.costs = []

    def record(self, latency_ms, cost):
        self.latencies.append(latency_ms)
        self.costs.append(cost)

    def report(self):
        return {
            "avg_latency": sum(self.latencies) / len(self.latencies),
            "p95_latency": sorted(self.latencies)[int(len(self.latencies) * 0.95)],
            "total_cost": sum(self.costs),
            "requests": len(self.latencies)
        }
```

## Next Steps

1. **Module 1**: Learn LLM fundamentals (tokenization, embeddings)
2. **Module 2**: Master API integration (OpenAI, Claude)
3. **Module 3**: Build HFT applications (sentiment, signals)
4. **Module 4**: Implement investment banking use cases (document analysis)
5. **Examples**: Study real-world scenarios

## Common Pitfalls to Avoid

❌ **Don't**: Make API calls in loops without batching
✅ **Do**: Batch requests or use async operations

❌ **Don't**: Use expensive models for simple tasks
✅ **Do**: Match model to task complexity

❌ **Don't**: Ignore rate limits
✅ **Do**: Implement exponential backoff retry logic

❌ **Don't**: Skip caching
✅ **Do**: Cache responses aggressively

❌ **Don't**: Trust AI output blindly for trading
✅ **Do**: Use AI as a signal among many, with proper risk management

## Resources

- [OpenAI Documentation](https://platform.openai.com/docs)
- [Anthropic Documentation](https://docs.anthropic.com)
- [Course Examples](./examples/)
- [API Cost Calculator](https://openai.com/pricing)

## Support

- Issues: [GitHub Issues](https://github.com/your-repo/issues)
- Questions: [Discussions](https://github.com/your-repo/discussions)

---

**Ready to build AI-powered financial applications? Start with Module 1!** 🚀
