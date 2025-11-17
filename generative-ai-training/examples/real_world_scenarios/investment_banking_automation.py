"""
Real-World Example: Investment Banking Workflow Automation

Complete automation of common investment banking tasks:
1. Company screening for M&A targets
2. Financial document analysis (10-K, 10-Q)
3. Comparable company analysis
4. Investment memorandum generation
5. Due diligence automation

Demonstrates:
- Multi-step workflows
- Document processing
- Structured data extraction
- Professional report generation
"""

import os
from typing import List, Dict, Any, Optional
from dataclasses import dataclass, asdict
from datetime import datetime
import json

import anthropic
import openai
from dotenv import load_dotenv

load_dotenv()


@dataclass
class CompanyProfile:
    """Company profile for analysis."""
    ticker: str
    name: str
    sector: str
    market_cap: float
    revenue: float
    ebitda: float
    pe_ratio: float
    debt_to_equity: float
    revenue_growth: float


@dataclass
class InvestmentThesis:
    """Structured investment thesis."""
    company: str
    recommendation: str  # BUY, SELL, HOLD
    target_price: float
    upside: float
    key_drivers: List[str]
    risks: List[str]
    valuation_summary: str
    investment_horizon: str
    confidence_level: float


class MATargetScreener:
    """
    Screen and identify M&A targets using AI.
    """

    def __init__(self):
        """Initialize screener."""
        self.client = anthropic.Anthropic(api_key=os.getenv("ANTHROPIC_API_KEY"))

    def screen_targets(
        self,
        criteria: Dict[str, Any],
        candidates: List[CompanyProfile]
    ) -> List[Dict[str, Any]]:
        """
        Screen companies for M&A fit.

        Args:
            criteria: Screening criteria
            candidates: List of candidate companies

        Returns:
            Ranked list of targets with analysis
        """
        # Prepare candidate data
        candidates_json = json.dumps(
            [asdict(c) for c in candidates],
            indent=2
        )

        prompt = f"""You are an M&A advisor screening acquisition targets.

Screening Criteria:
{json.dumps(criteria, indent=2)}

Candidate Companies:
{candidates_json}

For each candidate, evaluate:
1. Strategic fit (1-10)
2. Financial attractiveness (1-10)
3. Synergy potential (1-10)
4. Integration risk (1-10, lower is better)
5. Overall score (weighted average)

Provide detailed JSON response with:
- Ranked list of targets
- Scoring rationale for each
- Top 3 recommendations with detailed reasoning
- Potential deal structures

Be thorough and analytical."""

        response = self.client.messages.create(
            model="claude-3-5-sonnet-20241022",
            max_tokens=4096,
            temperature=0.3,
            system="You are a senior M&A advisor with 20+ years of experience.",
            messages=[{"role": "user", "content": prompt}]
        )

        result_text = response.content[0].text

        try:
            if "```json" in result_text:
                result_text = result_text.split("```json")[1].split("```")[0].strip()
            return json.loads(result_text)
        except:
            return {"raw_analysis": result_text}


class ComparableCompanyAnalyzer:
    """
    Perform comparable company analysis.
    """

    def __init__(self):
        """Initialize analyzer."""
        self.client = openai.OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

    def analyze_comparables(
        self,
        target: CompanyProfile,
        comparables: List[CompanyProfile]
    ) -> Dict[str, Any]:
        """
        Analyze target vs comparable companies.

        Args:
            target: Target company
            comparables: Comparable companies

        Returns:
            Valuation analysis
        """
        target_json = json.dumps(asdict(target), indent=2)
        comps_json = json.dumps(
            [asdict(c) for c in comparables],
            indent=2
        )

        prompt = f"""Perform comprehensive comparable company analysis.

Target Company:
{target_json}

Comparable Companies:
{comps_json}

Provide detailed analysis in JSON:
{{
    "valuation_metrics": {{
        "target_ev_revenue": float,
        "peer_avg_ev_revenue": float,
        "target_pe": float,
        "peer_avg_pe": float,
        "implied_valuation_range": {{"low": float, "high": float}}
    }},
    "relative_positioning": {{
        "growth_vs_peers": "higher/lower/inline",
        "profitability_vs_peers": "higher/lower/inline",
        "leverage_vs_peers": "higher/lower/inline"
    }},
    "valuation_assessment": "undervalued/fairly valued/overvalued",
    "key_insights": ["insight1", "insight2", "insight3"],
    "recommended_multiple": float,
    "target_valuation": float
}}"""

        response = self.client.chat.completions.create(
            model="gpt-4-turbo-preview",
            messages=[
                {
                    "role": "system",
                    "content": "You are an expert equity research analyst specializing in valuation."
                },
                {"role": "user", "content": prompt}
            ],
            temperature=0.3,
            response_format={"type": "json_object"}
        )

        return json.loads(response.choices[0].message.content)


class InvestmentMemoGenerator:
    """
    Generate professional investment memoranda.
    """

    def __init__(self):
        """Initialize generator."""
        self.client = anthropic.Anthropic(api_key=os.getenv("ANTHROPIC_API_KEY"))

    def generate_memo(
        self,
        company: CompanyProfile,
        analysis_data: Dict[str, Any],
        market_context: str
    ) -> str:
        """
        Generate comprehensive investment memorandum.

        Args:
            company: Company profile
            analysis_data: All analysis data
            market_context: Current market context

        Returns:
            Professional investment memo
        """
        company_json = json.dumps(asdict(company), indent=2)
        analysis_json = json.dumps(analysis_data, indent=2)

        prompt = f"""Generate a comprehensive investment memorandum for presentation to an investment committee.

Company:
{company_json}

Analysis Data:
{analysis_json}

Market Context:
{market_context}

The memorandum should include:

1. EXECUTIVE SUMMARY
   - Investment recommendation
   - Target price and expected return
   - Key investment highlights

2. COMPANY OVERVIEW
   - Business description
   - Market position
   - Competitive advantages

3. INVESTMENT THESIS
   - Key growth drivers
   - Strategic initiatives
   - Market opportunities

4. FINANCIAL ANALYSIS
   - Historical performance
   - Margin analysis
   - Cash flow assessment
   - Balance sheet strength

5. VALUATION
   - Valuation methodology
   - Comparable analysis
   - DCF analysis summary
   - Target price derivation

6. RISK FACTORS
   - Business risks
   - Market risks
   - Execution risks
   - Mitigation strategies

7. RECOMMENDATION
   - Investment rating
   - Position sizing recommendation
   - Key milestones to monitor

Write in a professional, institutional style. Be analytical and balanced.
Use clear section headers. Aim for 1500-2000 words."""

        response = self.client.messages.create(
            model="claude-3-5-sonnet-20241022",
            max_tokens=4096,
            temperature=0.7,
            system="You are a senior equity research analyst at a top-tier investment bank.",
            messages=[{"role": "user", "content": prompt}]
        )

        return response.content[0].text


class DueDiligenceWorkflow:
    """
    Complete due diligence workflow automation.
    """

    def __init__(self):
        """Initialize workflow."""
        self.client = anthropic.Anthropic(api_key=os.getenv("ANTHROPIC_API_KEY"))

    def run_due_diligence(
        self,
        company_name: str,
        documents: Dict[str, str],
        focus_areas: List[str]
    ) -> Dict[str, Any]:
        """
        Run comprehensive due diligence.

        Args:
            company_name: Target company
            documents: Dictionary of documents
            focus_areas: Areas to focus on

        Returns:
            DD report with findings
        """
        # Build comprehensive prompt
        prompt = f"""Conduct thorough buy-side due diligence on {company_name}.

Focus Areas:
{', '.join(focus_areas)}

Available Documents:
"""
        for doc_type, content in documents.items():
            prompt += f"\n\n{doc_type}:\n{content[:30000]}\n"

        prompt += """

Provide comprehensive due diligence report in JSON:
{{
    "executive_summary": "2-3 paragraph overview",
    "financial_due_diligence": {{
        "quality_of_earnings": "assessment",
        "working_capital": "analysis",
        "debt_analysis": "details",
        "recurring_revenue": "percentage and quality",
        "ebitda_adjustments": ["adjustment1", "adjustment2"]
    }},
    "commercial_due_diligence": {{
        "market_position": "assessment",
        "customer_concentration": "risk level",
        "competitive_dynamics": "analysis",
        "growth_sustainability": "assessment"
    }},
    "operational_due_diligence": {{
        "management_quality": "assessment",
        "org_structure": "analysis",
        "technology_infrastructure": "assessment",
        "key_man_risk": "level"
    }},
    "legal_regulatory": {{
        "compliance_status": "assessment",
        "litigation_risk": "level",
        "regulatory_issues": ["issue1", "issue2"],
        "ip_assessment": "analysis"
    }},
    "risk_factors": {{
        "critical_risks": ["risk1", "risk2"],
        "medium_risks": ["risk1", "risk2"],
        "mitigation_required": ["item1", "item2"]
    }},
    "red_flags": ["flag1", "flag2"],
    "value_creation_opportunities": ["opp1", "opp2"],
    "recommendation": {{
        "proceed": true/false,
        "valuation_range": {{"low": float, "high": float}},
        "key_conditions": ["condition1", "condition2"],
        "confidence_level": 1-10
    }}
}}"""

        response = self.client.messages.create(
            model="claude-3-5-sonnet-20241022",
            max_tokens=4096,
            temperature=0.2,  # Lower for factual analysis
            system="You are a senior due diligence professional with expertise across financial, commercial, and operational DD.",
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
    """Run investment banking automation examples."""

    print("="*80)
    print("Investment Banking Workflow Automation with AI")
    print("="*80)

    # Check API keys
    if not (os.getenv("OPENAI_API_KEY") or os.getenv("ANTHROPIC_API_KEY")):
        print("\nWarning: API keys not configured.")
        print("Set OPENAI_API_KEY or ANTHROPIC_API_KEY to run examples.")
        return

    # Example 1: M&A Target Screening
    print("\n\nExample 1: M&A Target Screening")
    print("="*80)

    screening_criteria = {
        "industry": "SaaS",
        "min_revenue": 50_000_000,
        "min_growth_rate": 20,
        "max_debt_to_equity": 1.0,
        "geographic_focus": "North America",
        "strategic_priorities": [
            "Cloud infrastructure",
            "AI/ML capabilities",
            "Enterprise customer base"
        ]
    }

    candidates = [
        CompanyProfile(
            ticker="SAAS1",
            name="CloudTech Inc",
            sector="SaaS",
            market_cap=500_000_000,
            revenue=75_000_000,
            ebitda=15_000_000,
            pe_ratio=33.3,
            debt_to_equity=0.5,
            revenue_growth=35.0
        ),
        CompanyProfile(
            ticker="SAAS2",
            name="DataFlow Systems",
            sector="SaaS",
            market_cap=300_000_000,
            revenue=50_000_000,
            ebitda=10_000_000,
            pe_ratio=30.0,
            debt_to_equity=0.3,
            revenue_growth=28.0
        ),
        CompanyProfile(
            ticker="SAAS3",
            name="Enterprise Solutions Co",
            sector="SaaS",
            market_cap=800_000_000,
            revenue=120_000_000,
            ebitda=24_000_000,
            pe_ratio=33.3,
            debt_to_equity=0.8,
            revenue_growth=22.0
        ),
    ]

    try:
        screener = MATargetScreener()
        screening_results = screener.screen_targets(screening_criteria, candidates)

        print("\nM&A Target Screening Results:")
        print(json.dumps(screening_results, indent=2)[:1500])
        print("\n... (truncated)")
    except Exception as e:
        print(f"Error: {e}")

    # Example 2: Comparable Company Analysis
    print("\n\nExample 2: Comparable Company Analysis")
    print("="*80)

    target = CompanyProfile(
        ticker="TARGET",
        name="Target Company",
        sector="SaaS",
        market_cap=600_000_000,
        revenue=100_000_000,
        ebitda=20_000_000,
        pe_ratio=30.0,
        debt_to_equity=0.6,
        revenue_growth=30.0
    )

    try:
        comp_analyzer = ComparableCompanyAnalyzer()
        comp_analysis = comp_analyzer.analyze_comparables(target, candidates)

        print("\nComparable Company Analysis:")
        print(json.dumps(comp_analysis, indent=2))
    except Exception as e:
        print(f"Error: {e}")

    # Example 3: Investment Memo Generation
    print("\n\nExample 3: Investment Memorandum Generation")
    print("="*80)

    analysis_data = {
        "valuation": {
            "dcf_value": 700_000_000,
            "comparable_value": 650_000_000,
            "target_price": 42.50,
            "current_price": 35.00,
            "upside": 21.4
        },
        "strengths": [
            "Market leader in cloud infrastructure",
            "Strong revenue growth and retention",
            "Expanding into AI/ML segment"
        ],
        "risks": [
            "Competition from larger players",
            "Customer concentration",
            "Technology transition execution"
        ]
    }

    market_context = """
    SaaS sector showing strong fundamentals with cloud adoption accelerating.
    Valuations have moderated from 2021 peaks, creating opportunities.
    AI integration becoming key differentiator.
    """

    try:
        memo_gen = InvestmentMemoGenerator()
        memo = memo_gen.generate_memo(target, analysis_data, market_context)

        print("\nInvestment Memorandum:")
        print(memo[:1500])
        print("\n... (truncated)")

        # Save to file
        with open("investment_memo_example.md", "w") as f:
            f.write(memo)
        print("\nFull memo saved to: investment_memo_example.md")
    except Exception as e:
        print(f"Error: {e}")

    # Example 4: Due Diligence Automation
    print("\n\nExample 4: Due Diligence Workflow")
    print("="*80)

    dd_documents = {
        "Financial Statements": """
        Revenue: $100M (up 30% YoY)
        Gross Margin: 75%
        EBITDA: $20M (20% margin)
        Net Income: $12M
        ARR: $95M
        Net Revenue Retention: 118%
        Customer Count: 450
        Top 10 customers: 35% of revenue
        Cash: $30M
        Debt: $20M
        """,
        "Management Discussion": """
        Strong performance driven by enterprise segment growth.
        Expanded into new vertical markets.
        R&D investment increased to 25% of revenue for AI initiatives.
        Sales team grew 40% to support expansion.
        Renewed focus on large enterprise deals.
        """,
        "Customer Data": """
        NRR: 118%
        Gross Churn: 8%
        Average deal size: $222K
        Sales cycle: 6 months
        Implementation time: 3 months
        Customer satisfaction: 4.5/5
        """
    }

    focus_areas = [
        "Quality of earnings",
        "Revenue sustainability",
        "Customer concentration risk",
        "Technology scalability",
        "Management capability"
    ]

    try:
        dd_workflow = DueDiligenceWorkflow()
        dd_report = dd_workflow.run_due_diligence(
            "Target Company",
            dd_documents,
            focus_areas
        )

        print("\nDue Diligence Report:")
        print(json.dumps(dd_report, indent=2)[:2000])
        print("\n... (truncated)")
    except Exception as e:
        print(f"Error: {e}")

    print("\n" + "="*80)
    print("Automation Complete")
    print("="*80)


if __name__ == "__main__":
    main()
