# Prompt Engineering for Financial Applications

Master the art of prompt engineering to get the best results from LLMs in finance.

## Table of Contents

1. [Core Principles](#core-principles)
2. [Financial-Specific Patterns](#financial-specific-patterns)
3. [Common Mistakes](#common-mistakes)
4. [Advanced Techniques](#advanced-techniques)
5. [Production Tips](#production-tips)

## Core Principles

### 1. Be Specific and Clear

**Bad:**
```
Analyze this stock
```

**Good:**
```
Analyze Apple Inc. (AAPL) stock based on:
- Current P/E ratio: 28.5
- Revenue growth: 8% YoY
- Recent news: Strong iPhone 15 sales

Provide:
1. Valuation assessment (undervalued/fairly valued/overvalued)
2. Key growth drivers
3. Main risks
4. Recommendation (BUY/HOLD/SELL)
```

### 2. Specify Output Format

**Bad:**
```
Tell me about market sentiment
```

**Good:**
```
Analyze market sentiment and respond with JSON:
{
  "sentiment": "BULLISH/BEARISH/NEUTRAL",
  "score": -1.0 to 1.0,
  "confidence": 0.0 to 1.0,
  "key_factors": ["factor1", "factor2"],
  "recommendation": "action to take"
}
```

### 3. Provide Context

**Bad:**
```
Is this a good trade?
TSLA $250
```

**Good:**
```
Evaluate this trade opportunity:

Current Position: Long 500 shares TSLA at $245 (entry)
Current Price: $250
Market Context: Tech stocks rallying on Fed pause signals
News: Tesla Q3 deliveries beat estimates by 4%

Risk Parameters:
- Stop loss: $242 (-1.2%)
- Take profit: $258 (+3.3%)
- Position size: 5% of portfolio

Should I:
1. Take profit now?
2. Hold for higher target?
3. Adjust stop loss?

Provide reasoning for your recommendation.
```

### 4. Use Examples (Few-Shot Learning)

```
Task: Extract trading signals from news

Examples:
Input: "Apple stock surges 5% on earnings beat"
Output: {"ticker": "AAPL", "direction": "BULLISH", "magnitude": "MODERATE", "confidence": 0.85}

Input: "Fed raises rates, markets tumble"
Output: {"ticker": "SPY", "direction": "BEARISH", "magnitude": "HIGH", "confidence": 0.90}

Now extract from:
Input: "Tesla deliveries exceed expectations, stock up 7%"
Output:
```

## Financial-Specific Patterns

### Pattern 1: Sentiment Analysis

```python
def create_sentiment_prompt(text, ticker):
    return f"""
Analyze the financial sentiment of this text for {ticker}.

Text: {text}

Consider:
1. Direct mentions of financial metrics (earnings, revenue, margins)
2. Management tone and forward guidance
3. Analyst opinions and price target changes
4. Market reaction and volume
5. Competitive dynamics mentioned

Respond with JSON:
{{
    "sentiment": "BULLISH/BEARISH/NEUTRAL",
    "score": -1.0 to 1.0 (where -1 is very bearish, +1 is very bullish),
    "confidence": 0.0 to 1.0,
    "price_impact": "percentage estimate",
    "time_horizon": "IMMEDIATE/SHORT_TERM/LONG_TERM",
    "key_factors": ["specific factors driving sentiment"],
    "urgency": "LOW/MEDIUM/HIGH"
}}
"""
```

### Pattern 2: Risk Assessment

```python
def create_risk_assessment_prompt(position_data):
    return f"""
Assess the risk profile of this trading position:

Position Details:
{json.dumps(position_data, indent=2)}

Evaluate:
1. Market risk (volatility, correlation to indices)
2. Liquidity risk (can position be closed quickly?)
3. Concentration risk (position size relative to portfolio)
4. Event risk (upcoming earnings, economic data)
5. Technical risk (support/resistance levels)

Provide risk assessment in JSON:
{{
    "overall_risk": "LOW/MEDIUM/HIGH/EXTREME",
    "risk_score": 0-100,
    "key_risks": [
        {{"type": "risk_type", "severity": "HIGH/MEDIUM/LOW", "description": "details"}}
    ],
    "risk_mitigation": ["recommendation1", "recommendation2"],
    "max_loss_scenario": "worst case analysis",
    "recommended_action": "HOLD/REDUCE/CLOSE"
}}
"""
```

### Pattern 3: Document Analysis

```python
def create_10k_analysis_prompt(filing_text):
    return f"""
You are a senior equity research analyst. Analyze this 10-K filing.

Filing: {filing_text[:50000]}

Extract and analyze:

1. FINANCIAL PERFORMANCE
   - 3-year revenue trend and growth rates
   - Profitability metrics (gross, operating, net margins)
   - Cash flow analysis (operating, free cash flow)
   - Balance sheet strength (debt levels, liquidity ratios)

2. BUSINESS ANALYSIS
   - Segment performance and trends
   - Geographic revenue breakdown
   - Product/service mix changes

3. RISK ASSESSMENT
   - Top 5 material risks (from Risk Factors section)
   - Risk severity and mitigation

4. STRATEGIC POSITIONING
   - Competitive advantages (moats)
   - Market position and share
   - R&D and innovation focus

5. MANAGEMENT ASSESSMENT
   - Strategic priorities from MD&A
   - Capital allocation strategy
   - Management credibility indicators

6. INVESTMENT THESIS
   - Bull case (3-5 key drivers)
   - Bear case (3-5 key concerns)
   - Recommendation (BUY/HOLD/SELL)
   - Confidence level (0-100)

Format response as detailed JSON with clear structure.
"""
```

### Pattern 4: Trade Explanation

```python
def create_trade_explanation_prompt(trade_data):
    return f"""
Generate a clear, professional explanation of this trade for a client.

Trade Details:
{json.dumps(trade_data, indent=2)}

Create a 2-paragraph explanation that:
1. Explains WHY this trade was made (market opportunity, fundamental catalyst)
2. Describes the RISK MANAGEMENT (stop loss, position sizing)
3. States the EXPECTED OUTCOME (price target, timeframe)

Style:
- Professional but accessible
- Avoid jargon where possible
- Focus on key factors, not every detail
- Convey confidence without overpromising

Target audience: Sophisticated retail or institutional investor
"""
```

## Common Mistakes

### Mistake 1: Too Vague

**Don't:**
```
What do you think about the market?
```

**Do:**
```
Analyze current S&P 500 market conditions based on:
- Recent price action: +2.5% this week
- VIX level: 14 (low volatility)
- Economic data: Strong jobs report Friday
- Fed policy: Pause indicated in recent minutes

Assess: Is this a good time to add equity exposure?
```

### Mistake 2: No Output Structure

**Don't:**
```
Analyze these companies and tell me which to invest in
```

**Do:**
```
Compare these companies and rank them for investment.

For each company provide:
{
  "company": "name",
  "score": 0-100,
  "strengths": ["strength1", "strength2"],
  "weaknesses": ["weakness1", "weakness2"],
  "valuation": "cheap/fair/expensive",
  "recommendation": "BUY/HOLD/SELL"
}

Then provide overall ranking with reasoning.
```

### Mistake 3: Ignoring Model Limitations

**Don't:**
```
What will AAPL stock price be tomorrow?
```

**Do:**
```
Based on these factors, what are possible scenarios for AAPL?

Current: $175
Recent news: Earnings beat, China sales weak
Technical: Above 50-day MA, approaching resistance at $180

Provide 3 scenarios (bullish/base/bearish) with:
- Price target
- Probability estimate
- Key assumption
```

### Mistake 4: No Context for Numbers

**Don't:**
```
Company has P/E of 30. Good or bad?
```

**Do:**
```
Evaluate this valuation in context:

Company: SaaS startup
P/E: 30
Growth rate: 50% YoY
Sector avg P/E: 25
Sector avg growth: 20%
Gross margin: 80%

Is 30 P/E justified? Compare to growth and margins.
```

## Advanced Techniques

### Technique 1: Chain-of-Thought Reasoning

```python
prompt = """
Analyze whether to buy this stock. Think step-by-step:

Company: TechCo
Price: $50
P/E: 45
Growth: 60% YoY
Market: Growing 30% annually
Competition: Moderate

Step 1: Assess valuation relative to growth (PEG ratio)
Step 2: Compare to market growth rate
Step 3: Evaluate competitive position
Step 4: Consider risk/reward
Step 5: Make recommendation

Show your reasoning for each step, then conclude.
"""
```

### Technique 2: Role-Playing for Better Responses

```python
system_prompt = """
You are a portfolio manager at a $10B hedge fund.
You have 20 years of experience in technology investing.
You are known for rigorous analysis and risk management.
You communicate clearly and back claims with data.
"""

user_prompt = "Should we invest in this AI startup?"
```

### Technique 3: Constraint-Based Generation

```python
prompt = f"""
Generate a market commentary with these constraints:

MUST include:
- Exactly 3 paragraphs
- At least 2 specific data points
- One forward-looking statement

MUST NOT include:
- Predictions of specific price levels
- Recommendations without reasoning
- Jargon without explanation

Topic: Fed policy impact on tech stocks
"""
```

### Technique 4: Multi-Turn Analysis

```python
# Turn 1: Initial analysis
response1 = analyze("What are risks in this portfolio?")

# Turn 2: Deeper dive
response2 = analyze(
    f"You identified '{specific_risk}' as a risk. "
    f"How can we hedge this? Provide 3 specific strategies."
)

# Turn 3: Implementation
response3 = analyze(
    f"For strategy '{chosen_strategy}', provide step-by-step implementation."
)
```

## Production Tips

### Tip 1: Version Your Prompts

```python
PROMPTS = {
    "sentiment_v1": "Analyze sentiment: {text}",
    "sentiment_v2": "Analyze financial sentiment with score -1 to +1: {text}",
    "sentiment_v3": """Analyze sentiment. Respond JSON:
        {{"sentiment": "BULLISH/BEARISH", "score": float}}
        Text: {text}"""
}

# Use versions to A/B test and track improvements
```

### Tip 2: Template Library

```python
class PromptTemplates:
    @staticmethod
    def sentiment(text, ticker):
        return f"Analyze {ticker}: {text}. Return JSON sentiment."

    @staticmethod
    def risk_assessment(position):
        return f"Assess risk for: {position}. Score 0-100."

    @staticmethod
    def trade_explanation(trade):
        return f"Explain trade to client: {trade}. 2 paragraphs."

# Centralized, reusable, testable
```

### Tip 3: Response Validation

```python
def validate_sentiment_response(response):
    required_fields = ["sentiment", "score", "confidence"]

    # Check structure
    if not all(field in response for field in required_fields):
        return False

    # Check values
    if response["sentiment"] not in ["BULLISH", "BEARISH", "NEUTRAL"]:
        return False

    if not -1 <= response["score"] <= 1:
        return False

    return True
```

### Tip 4: Fallback Strategies

```python
def analyze_with_fallback(text):
    try:
        # Try detailed analysis
        return detailed_analysis(text)
    except Exception as e:
        # Fallback to simple analysis
        return simple_analysis(text)
```

## Quick Reference

### Do's
✅ Be specific about what you want
✅ Request structured output (JSON)
✅ Provide relevant context
✅ Use examples when possible
✅ Specify constraints and requirements
✅ Ask for reasoning, not just answers
✅ Test and iterate on prompts

### Don'ts
❌ Ask vague questions
❌ Expect unstructured responses to be consistent
❌ Omit critical context
❌ Ask for impossible predictions
❌ Ignore model limitations
❌ Use in production without testing
❌ Trust responses without validation

## Testing Your Prompts

```python
def test_prompt(prompt_func, test_cases):
    """Test prompt across multiple scenarios."""

    results = []
    for test_case in test_cases:
        prompt = prompt_func(**test_case["input"])
        response = call_llm(prompt)

        results.append({
            "input": test_case["input"],
            "expected": test_case["expected"],
            "actual": response,
            "passed": validate(response, test_case["expected"])
        })

    return results

# Always test before deploying!
```

---

**Master prompt engineering and unlock the full potential of AI in your financial applications!**
