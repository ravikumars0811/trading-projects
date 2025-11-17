"""
Module 4.1: AI-Powered Financial Document Analysis

Automate analysis of 10-Ks, 10-Qs, earnings reports, and other financial documents.
Use Claude for long documents (200K context) or GPT-4 for structured extraction.
"""

import anthropic
import openai
import os
from typing import Dict, List, Any, Optional
import json
from dataclasses import dataclass
from datetime import datetime
from dotenv import load_dotenv
import re

load_dotenv()


@dataclass
class DocumentAnalysis:
    """Structured document analysis results."""
    document_type: str
    company: str
    period: str
    key_metrics: Dict[str, Any]
    risk_factors: List[str]
    opportunities: List[str]
    red_flags: List[str]
    management_assessment: str
    competitive_position: str
    financial_health_score: float  # 0-100
    investment_recommendation: str
    summary: str
    timestamp: str


class FinancialDocumentAnalyzer:
    """
    Analyze financial documents using AI.
    Supports: 10-K, 10-Q, earnings calls, research reports.
    """

    def __init__(self, provider: str = "claude"):
        """
        Initialize analyzer.

        Args:
            provider: 'claude' for long docs, 'openai' for structured extraction
        """
        self.provider = provider

        if provider == "claude":
            self.client = anthropic.Anthropic(
                api_key=os.getenv("ANTHROPIC_API_KEY")
            )
            self.model = "claude-3-5-sonnet-20241022"
        else:
            self.client = openai.OpenAI(api_key=os.getenv("OPENAI_API_KEY"))
            self.model = "gpt-4-turbo-preview"

    def analyze_10k(self, document_text: str, company: str) -> DocumentAnalysis:
        """
        Comprehensive 10-K analysis.
        Claude can handle the full document (200K tokens).

        Args:
            document_text: 10-K filing text
            company: Company name

        Returns:
            Structured analysis
        """
        prompt = f"""Analyze this 10-K filing for {company} and provide a comprehensive investment analysis.

10-K Document:
{document_text[:100000]}  # Claude handles more, truncating for example

Provide analysis in this JSON structure:
{{
    "key_metrics": {{
        "revenue_trend": "analysis of 3-year revenue trend",
        "profit_margins": "gross, operating, net margin analysis",
        "cash_flow": "operating, investing, financing CF analysis",
        "balance_sheet_strength": "debt levels, liquidity, working capital",
        "growth_metrics": "revenue growth, earnings growth rates"
    }},
    "risk_factors": ["top 5 most significant risks"],
    "opportunities": ["top 5 growth opportunities"],
    "red_flags": ["concerning items that require attention"],
    "management_assessment": "evaluation of management discussion and strategy",
    "competitive_position": "market position and competitive advantages",
    "financial_health_score": 0-100,
    "investment_recommendation": "BUY/HOLD/SELL with rationale",
    "summary": "2-3 paragraph executive summary"
}}"""

        if self.provider == "claude":
            response = self.client.messages.create(
                model=self.model,
                max_tokens=4096,
                temperature=0.3,
                system="You are an expert financial analyst with deep experience in SEC filings and investment research.",
                messages=[{"role": "user", "content": prompt}]
            )
            result_text = response.content[0].text
        else:
            response = self.client.chat.completions.create(
                model=self.model,
                messages=[
                    {
                        "role": "system",
                        "content": "You are an expert financial analyst. Provide detailed, structured analysis."
                    },
                    {"role": "user", "content": prompt}
                ],
                temperature=0.3,
                response_format={"type": "json_object"}
            )
            result_text = response.choices[0].message.content

        # Parse JSON response
        try:
            if "```json" in result_text:
                result_text = result_text.split("```json")[1].split("```")[0].strip()
            elif "```" in result_text:
                result_text = result_text.split("```")[1].split("```")[0].strip()

            analysis_data = json.loads(result_text)

            return DocumentAnalysis(
                document_type="10-K",
                company=company,
                period="Annual",
                key_metrics=analysis_data.get("key_metrics", {}),
                risk_factors=analysis_data.get("risk_factors", []),
                opportunities=analysis_data.get("opportunities", []),
                red_flags=analysis_data.get("red_flags", []),
                management_assessment=analysis_data.get("management_assessment", ""),
                competitive_position=analysis_data.get("competitive_position", ""),
                financial_health_score=analysis_data.get("financial_health_score", 50),
                investment_recommendation=analysis_data.get("investment_recommendation", "HOLD"),
                summary=analysis_data.get("summary", ""),
                timestamp=datetime.now().isoformat()
            )
        except Exception as e:
            # Return raw analysis if JSON parsing fails
            return DocumentAnalysis(
                document_type="10-K",
                company=company,
                period="Annual",
                key_metrics={"raw_analysis": result_text},
                risk_factors=[],
                opportunities=[],
                red_flags=[],
                management_assessment="",
                competitive_position="",
                financial_health_score=50,
                investment_recommendation="HOLD",
                summary=result_text[:500],
                timestamp=datetime.now().isoformat()
            )

    def analyze_earnings_call(
        self,
        transcript: str,
        company: str,
        quarter: str
    ) -> Dict[str, Any]:
        """
        Analyze earnings call transcript.

        Args:
            transcript: Call transcript
            company: Company name
            quarter: Quarter (e.g., "Q3 2024")

        Returns:
            Analysis results
        """
        prompt = f"""Analyze this earnings call transcript for {company} ({quarter}).

Transcript:
{transcript[:50000]}

Extract and analyze:
1. Key announcements and highlights
2. Financial performance vs expectations
3. Forward guidance and outlook
4. Management tone and confidence
5. Key questions from analysts
6. Concerns or challenges discussed
7. Strategic initiatives mentioned
8. Market sentiment (bullish/bearish/neutral)

Provide structured JSON response."""

        if self.provider == "claude":
            response = self.client.messages.create(
                model=self.model,
                max_tokens=3000,
                temperature=0.3,
                messages=[{"role": "user", "content": prompt}]
            )
            result_text = response.content[0].text
        else:
            response = self.client.chat.completions.create(
                model=self.model,
                messages=[{"role": "user", "content": prompt}],
                temperature=0.3,
                response_format={"type": "json_object"}
            )
            result_text = response.choices[0].message.content

        try:
            if "```json" in result_text:
                result_text = result_text.split("```json")[1].split("```")[0].strip()
            return json.loads(result_text)
        except:
            return {"raw_analysis": result_text}

    def extract_financial_metrics(self, document: str) -> Dict[str, float]:
        """
        Extract specific financial metrics from document.
        Use function calling for structured extraction.

        Args:
            document: Financial document text

        Returns:
            Extracted metrics
        """
        if self.provider != "openai":
            return {}

        # Define extraction schema
        functions = [{
            "name": "extract_financials",
            "description": "Extract financial metrics from document",
            "parameters": {
                "type": "object",
                "properties": {
                    "revenue": {"type": "number", "description": "Total revenue"},
                    "revenue_growth": {"type": "number", "description": "Revenue growth %"},
                    "net_income": {"type": "number", "description": "Net income"},
                    "eps": {"type": "number", "description": "Earnings per share"},
                    "gross_margin": {"type": "number", "description": "Gross margin %"},
                    "operating_margin": {"type": "number", "description": "Operating margin %"},
                    "free_cash_flow": {"type": "number", "description": "Free cash flow"},
                    "total_debt": {"type": "number", "description": "Total debt"},
                    "cash": {"type": "number", "description": "Cash and equivalents"}
                }
            }
        }]

        response = self.client.chat.completions.create(
            model=self.model,
            messages=[{
                "role": "user",
                "content": f"Extract financial metrics from: {document[:10000]}"
            }],
            functions=functions,
            function_call={"name": "extract_financials"}
        )

        if response.choices[0].message.function_call:
            return json.loads(response.choices[0].message.function_call.arguments)
        return {}

    def compare_documents(
        self,
        doc1: str,
        doc2: str,
        comparison_focus: str
    ) -> Dict[str, Any]:
        """
        Compare two financial documents (e.g., consecutive quarters).

        Args:
            doc1: First document
            doc2: Second document
            comparison_focus: What to focus on

        Returns:
            Comparative analysis
        """
        prompt = f"""Compare these two financial documents focusing on: {comparison_focus}

Document 1:
{doc1[:30000]}

Document 2:
{doc2[:30000]}

Provide:
1. Key changes and trends
2. Improvements or deteriorations
3. Notable differences
4. Strategic shifts
5. Overall trajectory (improving/declining/stable)

Format as JSON."""

        if self.provider == "claude":
            response = self.client.messages.create(
                model=self.model,
                max_tokens=3000,
                temperature=0.3,
                messages=[{"role": "user", "content": prompt}]
            )
            result_text = response.content[0].text
        else:
            response = self.client.chat.completions.create(
                model=self.model,
                messages=[{"role": "user", "content": prompt}],
                temperature=0.3
            )
            result_text = response.choices[0].message.content

        try:
            if "```json" in result_text:
                result_text = result_text.split("```json")[1].split("```")[0].strip()
            return json.loads(result_text)
        except:
            return {"analysis": result_text}


class DueDiligenceAutomation:
    """
    Automate due diligence document review for M&A, investments.
    """

    def __init__(self):
        """Initialize due diligence automation."""
        self.analyzer = FinancialDocumentAnalyzer(provider="claude")

    def comprehensive_due_diligence(
        self,
        company: str,
        documents: Dict[str, str],
        focus_areas: List[str]
    ) -> Dict[str, Any]:
        """
        Comprehensive due diligence across multiple documents.

        Args:
            company: Target company
            documents: Dict of {doc_type: doc_text}
            focus_areas: Areas to focus on

        Returns:
            DD report
        """
        prompt = f"""Conduct comprehensive due diligence on {company}.

Documents provided:
{', '.join(documents.keys())}

Focus areas:
{', '.join(focus_areas)}

Documents:
"""
        for doc_type, doc_text in documents.items():
            prompt += f"\n\n{doc_type}:\n{doc_text[:20000]}\n"

        prompt += """

Provide comprehensive due diligence report covering:
1. Financial health and performance
2. Risk assessment (categorized by type)
3. Legal and compliance issues
4. Operational assessment
5. Market position and competition
6. Growth potential
7. Valuation considerations
8. Red flags and concerns
9. Key strengths and opportunities
10. Recommendation with confidence level

Format as detailed JSON."""

        response = self.analyzer.client.messages.create(
            model=self.analyzer.model,
            max_tokens=4096,
            temperature=0.2,  # Lower for factual analysis
            system="You are a senior M&A advisor conducting thorough due diligence.",
            messages=[{"role": "user", "content": prompt}]
        )

        result_text = response.content[0].text

        try:
            if "```json" in result_text:
                result_text = result_text.split("```json")[1].split("```")[0].strip()
            return json.loads(result_text)
        except:
            return {"report": result_text}


def main():
    """Run document analysis demonstrations."""

    print("AI-Powered Financial Document Analysis")
    print("="*80)

    if not (os.getenv("ANTHROPIC_API_KEY") or os.getenv("OPENAI_API_KEY")):
        print("Warning: No API keys set. Examples will be limited.")
        return

    # Example 1: 10-K Analysis
    print("\nExample 1: 10-K Filing Analysis")
    print("="*80)

    # Sample 10-K excerpt
    sample_10k = """
    Apple Inc. - Annual Report (Form 10-K)

    Item 1. Business
    Apple Inc. designs, manufactures, and markets smartphones, personal computers,
    tablets, wearables, and accessories worldwide. The Company's products include
    iPhone, Mac, iPad, and Wearables, Home and Accessories.

    Item 7. Management's Discussion and Analysis

    Net sales for fiscal 2023 totaled $383.3 billion, a decrease of 3% compared
    to fiscal 2022, driven by foreign exchange headwinds and macroeconomic challenges.

    Products revenue: $298.1 billion (down 4% YoY)
    Services revenue: $85.2 billion (up 9% YoY)

    Gross margin: 44.1% compared to 43.3% in prior year
    Operating expenses: $55.0 billion
    Net income: $97.0 billion
    Diluted EPS: $6.16

    Cash and marketable securities: $166.5 billion
    Total debt: $109.3 billion

    The Company returned $93 billion to shareholders through dividends and
    share repurchases during fiscal 2023.

    Item 1A. Risk Factors

    - Dependence on third-party suppliers and manufacturers
    - Intense competition in global markets
    - Macroeconomic conditions affecting consumer spending
    - Geopolitical tensions impacting supply chain
    - Rapid technological change requiring significant R&D investment
    - Cybersecurity and privacy concerns
    - Regulatory changes in multiple jurisdictions
    """

    try:
        analyzer = FinancialDocumentAnalyzer(provider="claude")
        analysis = analyzer.analyze_10k(sample_10k, "Apple Inc.")

        print(f"\n10-K Analysis for {analysis.company}:")
        print(f"\nFinancial Health Score: {analysis.financial_health_score}/100")
        print(f"Recommendation: {analysis.investment_recommendation}")

        print(f"\nKey Metrics:")
        for key, value in analysis.key_metrics.items():
            if isinstance(value, str):
                print(f"  {key}: {value[:100]}...")

        print(f"\nTop Risk Factors:")
        for i, risk in enumerate(analysis.risk_factors[:3], 1):
            print(f"  {i}. {risk}")

        print(f"\nTop Opportunities:")
        for i, opp in enumerate(analysis.opportunities[:3], 1):
            print(f"  {i}. {opp}")

        if analysis.red_flags:
            print(f"\nRed Flags:")
            for flag in analysis.red_flags:
                print(f"  ⚠ {flag}")

        print(f"\nSummary:")
        print(f"  {analysis.summary[:300]}...")

    except Exception as e:
        print(f"Error: {e}")

    # Example 2: Earnings Call Analysis
    print("\n\nExample 2: Earnings Call Transcript Analysis")
    print("="*80)

    sample_transcript = """
    Apple Inc. Q4 2023 Earnings Call Transcript

    Tim Cook, CEO:
    Thank you for joining us. We're pleased to report another strong quarter.
    Despite macroeconomic headwinds, we achieved revenue of $89.5 billion,
    up 8% year-over-year. iPhone revenue grew 3%, and Services set an all-time
    record at $22.3 billion, up 16%.

    Our installed base of active devices reached a new all-time high across
    all products and geographic segments. We're excited about the momentum
    heading into the holiday season with our strongest product lineup ever.

    Luca Maestri, CFO:
    Products revenue was $67.2 billion, up 5%. Services revenue was $22.3 billion.
    Gross margin was 45.2%, up 220 basis points. Operating expenses were well
    controlled at 25% of revenue. We generated $26 billion in operating cash flow.

    We returned $27 billion to shareholders through dividends and buybacks.
    Given our strong cash position and confidence in the business, we're
    increasing our dividend by 4%.

    Q&A Session:

    Analyst: Can you talk about iPhone demand trends in China?
    Tim Cook: We saw sequential improvement in Greater China. We're optimistic
    about the long-term opportunity there despite near-term volatility.

    Analyst: Services growth outlook?
    Luca Maestri: We expect Services to continue strong double-digit growth,
    driven by App Store, iCloud, and new offerings.
    """

    try:
        call_analysis = analyzer.analyze_earnings_call(
            sample_transcript,
            "Apple Inc.",
            "Q4 2023"
        )

        print("\nEarnings Call Analysis:")
        print(json.dumps(call_analysis, indent=2)[:1000])

    except Exception as e:
        print(f"Error: {e}")

    # Example 3: Due Diligence Automation
    print("\n\nExample 3: Automated Due Diligence")
    print("="*80)

    documents = {
        "10-K": sample_10k,
        "Earnings Call": sample_transcript
    }

    focus_areas = [
        "Financial stability",
        "Growth prospects",
        "Risk factors",
        "Competitive position"
    ]

    try:
        dd_automation = DueDiligenceAutomation()
        dd_report = dd_automation.comprehensive_due_diligence(
            "Apple Inc.",
            documents,
            focus_areas
        )

        print("\nDue Diligence Report:")
        print(json.dumps(dd_report, indent=2)[:1500])
        print("\n... (truncated)")

    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()
