"""
Module 2.2: Claude API Basics

Learn how to use Anthropic's Claude API for financial applications.
Claude excels at: analysis, reasoning, long context, structured thinking.
"""

import anthropic
import os
import json
from typing import List, Dict, Any, Generator
from dotenv import load_dotenv
import time

load_dotenv()


class ClaudeFinancialAssistant:
    """
    Production-ready Claude integration for financial applications.
    Claude advantages: 200K context, excellent reasoning, strong with structured data.
    """

    def __init__(
        self,
        api_key: str = None,
        model: str = "claude-3-5-sonnet-20241022"
    ):
        """
        Initialize Claude client.

        Args:
            api_key: Anthropic API key
            model: Model to use (claude-3-opus, claude-3-sonnet, claude-3-haiku)
        """
        self.client = anthropic.Anthropic(
            api_key=api_key or os.getenv("ANTHROPIC_API_KEY")
        )
        self.model = model

    def simple_completion(
        self,
        prompt: str,
        system_prompt: str = None,
        temperature: float = 1.0,
        max_tokens: int = 4096
    ) -> str:
        """
        Simple text completion with Claude.

        Args:
            prompt: User prompt
            system_prompt: System instructions
            temperature: Randomness (0-1)
            max_tokens: Maximum response length

        Returns:
            Generated text
        """
        message = self.client.messages.create(
            model=self.model,
            max_tokens=max_tokens,
            temperature=temperature,
            system=system_prompt if system_prompt else "You are a helpful financial analyst assistant.",
            messages=[
                {"role": "user", "content": prompt}
            ]
        )

        return message.content[0].text

    def analyze_10k_filing(self, filing_text: str) -> dict:
        """
        Analyze SEC 10-K filing using Claude's long context.
        Claude can handle 200K tokens - perfect for long documents!

        Args:
            filing_text: 10-K filing text

        Returns:
            Structured analysis
        """
        system_prompt = """You are an expert financial analyst specializing in SEC filings.
        Analyze 10-K filings and extract key information in a structured format."""

        prompt = f"""
        Analyze this 10-K filing and provide a comprehensive analysis in JSON format with:

        1. company_overview: Brief description
        2. financial_highlights:
           - revenue_trend: 3-year trend analysis
           - profitability_metrics: Key margins and ratios
           - cash_flow: Operating, investing, financing cash flows
        3. risk_factors: Top 5 most significant risks
        4. business_segments: Performance by segment
        5. competitive_position: Market position and competitive advantages
        6. management_discussion: Key points from MD&A
        7. red_flags: Any concerning items
        8. investment_thesis: Bull and bear cases

        10-K Filing:
        {filing_text[:50000]}  # Claude can handle much more, but truncating for example
        """

        response = self.simple_completion(
            prompt=prompt,
            system_prompt=system_prompt,
            temperature=0.3,
            max_tokens=4096
        )

        # Try to extract JSON from response
        try:
            # Claude might wrap JSON in markdown code blocks
            if "```json" in response:
                json_str = response.split("```json")[1].split("```")[0].strip()
                return json.loads(json_str)
            elif "```" in response:
                json_str = response.split("```")[1].split("```")[0].strip()
                return json.loads(json_str)
            else:
                return json.loads(response)
        except:
            return {"raw_analysis": response}

    def streaming_response(
        self,
        prompt: str,
        system_prompt: str = None
    ) -> Generator[str, None, None]:
        """
        Stream responses from Claude.
        Use case: Real-time user experience, long-form content.

        Args:
            prompt: User prompt
            system_prompt: System instructions

        Yields:
            Response chunks
        """
        with self.client.messages.stream(
            model=self.model,
            max_tokens=4096,
            system=system_prompt or "You are a helpful financial analyst assistant.",
            messages=[{"role": "user", "content": prompt}]
        ) as stream:
            for text in stream.text_stream:
                yield text

    def multi_turn_conversation(self, conversation_history: List[Dict[str, str]]) -> str:
        """
        Multi-turn conversation with context.
        Use case: Interactive analysis, follow-up questions.

        Args:
            conversation_history: List of {"role": "user"/"assistant", "content": "..."}

        Returns:
            Assistant response
        """
        message = self.client.messages.create(
            model=self.model,
            max_tokens=4096,
            messages=conversation_history
        )

        return message.content[0].text

    def structured_analysis_with_thinking(self, query: str) -> dict:
        """
        Use Claude's extended thinking for complex analysis.
        Claude shows its reasoning process.

        Args:
            query: Analysis query

        Returns:
            Analysis with reasoning
        """
        system_prompt = """You are an expert quantitative analyst.
        Think through problems step-by-step and show your reasoning."""

        response = self.simple_completion(
            prompt=f"""Analyze the following query. Show your thinking process, then provide a structured answer.

Query: {query}

Format your response as:
THINKING: [Your step-by-step reasoning]
ANALYSIS: [Your structured analysis]
""",
            system_prompt=system_prompt,
            temperature=0.5
        )

        # Parse thinking and analysis
        parts = response.split("ANALYSIS:")
        thinking = parts[0].replace("THINKING:", "").strip() if len(parts) > 1 else ""
        analysis = parts[1].strip() if len(parts) > 1 else response

        return {
            "thinking": thinking,
            "analysis": analysis
        }

    def compare_with_examples(
        self,
        task: str,
        examples: List[Dict[str, str]],
        new_input: str
    ) -> str:
        """
        Few-shot learning with examples.
        Use case: Consistent formatting, specific analysis styles.

        Args:
            task: Description of task
            examples: List of {"input": "...", "output": "..."}
            new_input: New input to process

        Returns:
            Generated output
        """
        # Build prompt with examples
        examples_text = "\n\n".join([
            f"Input: {ex['input']}\nOutput: {ex['output']}"
            for ex in examples
        ])

        prompt = f"""Task: {task}

Here are some examples:

{examples_text}

Now, process this new input:
Input: {new_input}
Output:"""

        return self.simple_completion(prompt, temperature=0.3)


class ClaudePortfolioAnalyzer:
    """
    Advanced portfolio analysis using Claude.
    """

    def __init__(self):
        self.assistant = ClaudeFinancialAssistant()

    def analyze_portfolio_risk(self, portfolio: dict) -> dict:
        """
        Comprehensive portfolio risk analysis.

        Args:
            portfolio: Portfolio data

        Returns:
            Risk analysis
        """
        prompt = f"""
        Analyze the risk profile of this investment portfolio:

        {json.dumps(portfolio, indent=2)}

        Provide a comprehensive risk analysis including:
        1. Overall risk level (low/medium/high)
        2. Concentration risks
        3. Market risks
        4. Sector exposure analysis
        5. Correlation risks
        6. Specific recommendations to reduce risk

        Format as JSON with clear structure.
        """

        return self.assistant.structured_analysis_with_thinking(prompt)

    def generate_investment_memo(
        self,
        company: str,
        research_data: dict
    ) -> str:
        """
        Generate professional investment memorandum.

        Args:
            company: Company name
            research_data: Research data

        Returns:
            Investment memo
        """
        system_prompt = """You are a senior investment analyst at a top-tier investment firm.
        Write professional, well-reasoned investment memoranda."""

        prompt = f"""
        Write a comprehensive investment memorandum for {company} based on this research:

        {json.dumps(research_data, indent=2)}

        The memo should include:
        1. Executive Summary
        2. Investment Thesis
        3. Business Overview
        4. Financial Analysis
        5. Valuation
        6. Risks
        7. Recommendation

        Write in a professional, institutional style suitable for presentation to an investment committee.
        """

        return self.assistant.simple_completion(
            prompt=prompt,
            system_prompt=system_prompt,
            temperature=0.7,
            max_tokens=4096
        )


class ClaudeVsOpenAIComparison:
    """
    Compare Claude and OpenAI for different use cases.
    """

    @staticmethod
    def when_to_use_claude():
        """
        Guidance on when to use Claude vs OpenAI.
        """
        return {
            "Use Claude for": [
                "Long documents (10-K, 10-Q, research reports) - 200K context",
                "Complex reasoning and analysis",
                "Structured thinking and step-by-step analysis",
                "Reduced hallucination on factual content",
                "Citation and quote accuracy",
                "Constitutional AI (safer, more ethical responses)",
            ],
            "Use OpenAI for": [
                "Function calling and structured outputs",
                "Broader ecosystem (more tools, integrations)",
                "Faster inference (in some cases)",
                "Code generation",
                "Creative content generation",
                "Lower cost for simple tasks (GPT-3.5)",
            ],
            "Use Both": [
                "A/B testing different models",
                "Ensemble approaches (combine outputs)",
                "Fallback strategies (if one API is down)",
                "Cost optimization (route based on task complexity)",
            ]
        }


def main():
    """Run Claude API demonstrations."""

    if not os.getenv("ANTHROPIC_API_KEY"):
        print("Warning: ANTHROPIC_API_KEY not set. Some examples will be skipped.")
        print("Get your API key at: https://console.anthropic.com/")
        return

    assistant = ClaudeFinancialAssistant()

    # Example 1: Simple completion
    print("Example 1: Market Analysis with Claude")
    print("="*80)

    query = "Explain the current state of the US Treasury yield curve and its implications for equity markets."

    try:
        response = assistant.simple_completion(query)
        print(f"Query: {query}\n")
        print(f"Response:\n{response}")
    except Exception as e:
        print(f"Error: {e}")

    # Example 2: Streaming response
    print("\n\nExample 2: Streaming Response")
    print("="*80)

    stream_query = "What are the key differences between value and growth investing strategies?"

    try:
        print(f"Query: {stream_query}\n\nResponse: ", end="", flush=True)
        for chunk in assistant.streaming_response(stream_query):
            print(chunk, end="", flush=True)
        print("\n")
    except Exception as e:
        print(f"Error: {e}")

    # Example 3: Structured analysis with thinking
    print("\n\nExample 3: Analysis with Reasoning")
    print("="*80)

    complex_query = """
    A company has:
    - P/E ratio of 45
    - Revenue growth of 60% YoY
    - Negative earnings
    - $2B in cash, $500M in debt
    - Operating in a rapidly growing market

    Should we invest? What valuation method is appropriate?
    """

    try:
        result = assistant.structured_analysis_with_thinking(complex_query)
        print("Thinking Process:")
        print(result['thinking'])
        print("\n" + "="*80)
        print("\nAnalysis:")
        print(result['analysis'])
    except Exception as e:
        print(f"Error: {e}")

    # Example 4: Multi-turn conversation
    print("\n\nExample 4: Multi-turn Conversation")
    print("="*80)

    conversation = [
        {
            "role": "user",
            "content": "What are the main drivers of Apple's revenue?"
        }
    ]

    try:
        # First response
        response1 = assistant.multi_turn_conversation(conversation)
        print(f"User: {conversation[0]['content']}")
        print(f"Claude: {response1[:200]}...\n")

        # Continue conversation
        conversation.append({"role": "assistant", "content": response1})
        conversation.append({
            "role": "user",
            "content": "How does this compare to Microsoft?"
        })

        response2 = assistant.multi_turn_conversation(conversation)
        print(f"User: {conversation[2]['content']}")
        print(f"Claude: {response2[:200]}...")
    except Exception as e:
        print(f"Error: {e}")

    # Example 5: Few-shot learning
    print("\n\nExample 5: Few-shot Learning for Consistent Analysis")
    print("="*80)

    examples = [
        {
            "input": "AAPL up 3% on earnings beat",
            "output": "BULLISH | Ticker: AAPL | Catalyst: Earnings beat | Magnitude: Moderate (+3%)"
        },
        {
            "input": "Fed raises rates by 50bps, markets tumble",
            "output": "BEARISH | Ticker: Market-wide | Catalyst: Rate hike | Magnitude: Significant"
        }
    ]

    new_input = "Tesla announces record deliveries, stock surges 7%"

    try:
        result = assistant.compare_with_examples(
            task="Classify market news in a structured format",
            examples=examples,
            new_input=new_input
        )
        print(f"New Input: {new_input}")
        print(f"Formatted Output: {result}")
    except Exception as e:
        print(f"Error: {e}")

    # Example 6: Portfolio risk analysis
    print("\n\nExample 6: Portfolio Risk Analysis")
    print("="*80)

    sample_portfolio = {
        "total_value": 1000000,
        "positions": [
            {"ticker": "AAPL", "value": 300000, "sector": "Technology"},
            {"ticker": "MSFT", "value": 250000, "sector": "Technology"},
            {"ticker": "NVDA", "value": 200000, "sector": "Technology"},
            {"ticker": "JPM", "value": 150000, "sector": "Financials"},
            {"ticker": "JNJ", "value": 100000, "sector": "Healthcare"}
        ],
        "cash": 0
    }

    try:
        analyzer = ClaudePortfolioAnalyzer()
        risk_analysis = analyzer.analyze_portfolio_risk(sample_portfolio)
        print("Portfolio Risk Analysis:")
        print(f"\nThinking: {risk_analysis['thinking'][:300]}...")
        print(f"\nAnalysis: {risk_analysis['analysis'][:300]}...")
    except Exception as e:
        print(f"Error: {e}")

    # Example 7: When to use Claude vs OpenAI
    print("\n\nExample 7: Claude vs OpenAI - Usage Guidelines")
    print("="*80)

    comparison = ClaudeVsOpenAIComparison.when_to_use_claude()
    print("\nUse Claude for:")
    for item in comparison["Use Claude for"]:
        print(f"  • {item}")

    print("\nUse OpenAI for:")
    for item in comparison["Use OpenAI for"]:
        print(f"  • {item}")


if __name__ == "__main__":
    main()
