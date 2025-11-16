"""
AI-Powered Credit Risk Assessment for Investment Banking

This module provides comprehensive credit risk analysis using:
- LLMs for document analysis
- ML models for default prediction
- Sentiment analysis from news and filings
- Financial statement analysis
- Industry and macro factor assessment

Designed for professional credit analysis and underwriting.
"""

import numpy as np
import torch
import torch.nn as nn
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
from datetime import datetime
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class CreditAssessment:
    """Credit risk assessment result"""
    company_name: str
    credit_score: float  # 0-1000
    rating: str  # AAA, AA, A, BBB, BB, B, CCC, CC, C, D
    probability_of_default: float  # 0.0 to 1.0
    loss_given_default: float  # 0.0 to 1.0
    expected_loss: float
    risk_factors: List[Dict[str, any]]
    strengths: List[str]
    weaknesses: List[str]
    recommendation: str
    confidence: float
    timestamp: datetime


@dataclass
class FinancialRatios:
    """Key financial ratios for credit analysis"""
    # Profitability
    roa: float  # Return on Assets
    roe: float  # Return on Equity
    profit_margin: float
    operating_margin: float

    # Leverage
    debt_to_equity: float
    debt_to_assets: float
    interest_coverage: float
    debt_service_coverage: float

    # Liquidity
    current_ratio: float
    quick_ratio: float
    cash_ratio: float

    # Efficiency
    asset_turnover: float
    inventory_turnover: float
    receivables_turnover: float

    # Growth
    revenue_growth: float
    earnings_growth: float
    cash_flow_growth: float


class FinancialStatementAnalyzer:
    """
    Analyzes financial statements to extract key metrics

    Processes:
    - Balance sheets
    - Income statements
    - Cash flow statements
    """

    @staticmethod
    def calculate_ratios(
        balance_sheet: Dict,
        income_statement: Dict,
        cash_flow: Dict
    ) -> FinancialRatios:
        """
        Calculate comprehensive financial ratios

        Args:
            balance_sheet: Balance sheet data
            income_statement: Income statement data
            cash_flow: Cash flow statement data

        Returns:
            FinancialRatios object
        """
        # Extract key values
        total_assets = balance_sheet.get('total_assets', 1)
        total_equity = balance_sheet.get('total_equity', 1)
        total_debt = balance_sheet.get('total_debt', 0)
        current_assets = balance_sheet.get('current_assets', 0)
        current_liabilities = balance_sheet.get('current_liabilities', 1)
        cash = balance_sheet.get('cash', 0)
        inventory = balance_sheet.get('inventory', 0)

        revenue = income_statement.get('revenue', 1)
        net_income = income_statement.get('net_income', 0)
        operating_income = income_statement.get('operating_income', 0)
        interest_expense = income_statement.get('interest_expense', 1)

        operating_cash_flow = cash_flow.get('operating_cash_flow', 0)

        # Calculate ratios
        ratios = FinancialRatios(
            # Profitability
            roa=net_income / total_assets if total_assets > 0 else 0,
            roe=net_income / total_equity if total_equity > 0 else 0,
            profit_margin=net_income / revenue if revenue > 0 else 0,
            operating_margin=operating_income / revenue if revenue > 0 else 0,

            # Leverage
            debt_to_equity=total_debt / total_equity if total_equity > 0 else 0,
            debt_to_assets=total_debt / total_assets if total_assets > 0 else 0,
            interest_coverage=operating_income / interest_expense if interest_expense > 0 else 0,
            debt_service_coverage=operating_cash_flow / (total_debt * 0.1) if total_debt > 0 else 0,

            # Liquidity
            current_ratio=current_assets / current_liabilities if current_liabilities > 0 else 0,
            quick_ratio=(current_assets - inventory) / current_liabilities if current_liabilities > 0 else 0,
            cash_ratio=cash / current_liabilities if current_liabilities > 0 else 0,

            # Efficiency
            asset_turnover=revenue / total_assets if total_assets > 0 else 0,
            inventory_turnover=revenue / inventory if inventory > 0 else 0,
            receivables_turnover=revenue / (total_assets * 0.2) if total_assets > 0 else 0,

            # Growth (placeholder - would need historical data)
            revenue_growth=0.10,
            earnings_growth=0.08,
            cash_flow_growth=0.12
        )

        return ratios


class DefaultPredictionModel(nn.Module):
    """
    Deep learning model for default prediction

    Inputs:
    - Financial ratios
    - Industry factors
    - Macroeconomic indicators
    - Sentiment scores
    """

    def __init__(
        self,
        input_dim: int = 64,
        hidden_dim: int = 256
    ):
        super().__init__()

        self.network = nn.Sequential(
            nn.Linear(input_dim, hidden_dim),
            nn.BatchNorm1d(hidden_dim),
            nn.ReLU(),
            nn.Dropout(0.3),

            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.BatchNorm1d(hidden_dim // 2),
            nn.ReLU(),
            nn.Dropout(0.3),

            nn.Linear(hidden_dim // 2, hidden_dim // 4),
            nn.BatchNorm1d(hidden_dim // 4),
            nn.ReLU(),
            nn.Dropout(0.2),

            nn.Linear(hidden_dim // 4, 1),
            nn.Sigmoid()  # Output probability of default
        )

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """
        Forward pass

        Args:
            x: Input features [batch_size, input_dim]

        Returns:
            probability_of_default: [batch_size, 1]
        """
        return self.network(x)


class CreditRatingMapper:
    """
    Maps credit scores to letter ratings

    Based on S&P / Moody's rating scale
    """

    @staticmethod
    def score_to_rating(score: float) -> str:
        """
        Convert credit score (0-1000) to letter rating

        Args:
            score: Credit score (0-1000)

        Returns:
            Letter rating (AAA to D)
        """
        if score >= 900:
            return 'AAA'
        elif score >= 850:
            return 'AA+'
        elif score >= 800:
            return 'AA'
        elif score >= 750:
            return 'AA-'
        elif score >= 700:
            return 'A+'
        elif score >= 650:
            return 'A'
        elif score >= 600:
            return 'A-'
        elif score >= 550:
            return 'BBB+'
        elif score >= 500:
            return 'BBB'
        elif score >= 450:
            return 'BBB-'
        elif score >= 400:
            return 'BB+'
        elif score >= 350:
            return 'BB'
        elif score >= 300:
            return 'BB-'
        elif score >= 250:
            return 'B+'
        elif score >= 200:
            return 'B'
        elif score >= 150:
            return 'B-'
        elif score >= 100:
            return 'CCC'
        elif score >= 50:
            return 'CC'
        else:
            return 'C'

    @staticmethod
    def rating_to_pd(rating: str) -> float:
        """
        Map rating to typical probability of default

        Based on historical default rates
        """
        pd_map = {
            'AAA': 0.0001, 'AA+': 0.0002, 'AA': 0.0003, 'AA-': 0.0005,
            'A+': 0.0008, 'A': 0.0012, 'A-': 0.0020,
            'BBB+': 0.0035, 'BBB': 0.0055, 'BBB-': 0.0090,
            'BB+': 0.0150, 'BB': 0.0250, 'BB-': 0.0400,
            'B+': 0.0650, 'B': 0.1000, 'B-': 0.1500,
            'CCC': 0.2500, 'CC': 0.4000, 'C': 0.6000, 'D': 1.0000
        }
        return pd_map.get(rating, 0.05)


class CreditRiskAnalyzer:
    """
    Comprehensive Credit Risk Analyzer

    Analyzes:
    - Financial statements
    - Industry trends
    - Management quality
    - Market sentiment
    - Macroeconomic factors

    Uses:
    - Deep learning for default prediction
    - LLMs for qualitative analysis
    - Traditional credit metrics

    Example:
        >>> analyzer = CreditRiskAnalyzer()
        >>> assessment = analyzer.analyze(
        ...     company_name='Example Corp',
        ...     financial_statements=statements,
        ...     market_data=data
        ... )
        >>> print(f"Rating: {assessment.rating}")
        >>> print(f"PD: {assessment.probability_of_default:.2%}")
    """

    def __init__(
        self,
        model_path: Optional[str] = None,
        device: str = 'cuda' if torch.cuda.is_available() else 'cpu'
    ):
        self.device = device

        # Load default prediction model
        self.default_model = DefaultPredictionModel().to(device)
        if model_path:
            self.default_model.load_state_dict(torch.load(model_path))
        self.default_model.eval()

        # Financial analyzer
        self.financial_analyzer = FinancialStatementAnalyzer()

        # Rating mapper
        self.rating_mapper = CreditRatingMapper()

        logger.info("Credit Risk Analyzer initialized")

    def analyze(
        self,
        company_name: str,
        financial_statements: Dict,
        market_data: Optional[Dict] = None,
        news_sentiment: Optional[float] = None,
        industry: Optional[str] = None
    ) -> CreditAssessment:
        """
        Perform comprehensive credit risk analysis

        Args:
            company_name: Company name
            financial_statements: {
                'balance_sheet': {...},
                'income_statement': {...},
                'cash_flow': {...}
            }
            market_data: Market data (stock price, volatility, etc.)
            news_sentiment: Sentiment score (-1 to 1)
            industry: Industry sector

        Returns:
            CreditAssessment with complete analysis
        """
        # Calculate financial ratios
        ratios = self.financial_analyzer.calculate_ratios(
            balance_sheet=financial_statements.get('balance_sheet', {}),
            income_statement=financial_statements.get('income_statement', {}),
            cash_flow=financial_statements.get('cash_flow', {})
        )

        # Prepare features for ML model
        features = self._prepare_features(
            ratios=ratios,
            market_data=market_data,
            news_sentiment=news_sentiment,
            industry=industry
        )

        # Predict probability of default
        with torch.no_grad():
            features_tensor = torch.FloatTensor(features).unsqueeze(0).to(self.device)
            pd = self.default_model(features_tensor).item()

        # Calculate credit score
        credit_score = self._calculate_credit_score(ratios, pd)

        # Map to rating
        rating = self.rating_mapper.score_to_rating(credit_score)

        # Estimate loss given default
        lgd = self._estimate_lgd(
            financial_statements.get('balance_sheet', {}),
            industry
        )

        # Calculate expected loss
        expected_loss = pd * lgd

        # Identify risk factors
        risk_factors = self._identify_risk_factors(ratios, market_data)

        # Identify strengths and weaknesses
        strengths = self._identify_strengths(ratios)
        weaknesses = self._identify_weaknesses(ratios)

        # Generate recommendation
        recommendation = self._generate_recommendation(
            rating, pd, expected_loss
        )

        # Calculate confidence
        confidence = self._calculate_confidence(ratios, market_data)

        # Create assessment
        assessment = CreditAssessment(
            company_name=company_name,
            credit_score=credit_score,
            rating=rating,
            probability_of_default=pd,
            loss_given_default=lgd,
            expected_loss=expected_loss,
            risk_factors=risk_factors,
            strengths=strengths,
            weaknesses=weaknesses,
            recommendation=recommendation,
            confidence=confidence,
            timestamp=datetime.now()
        )

        return assessment

    def _prepare_features(
        self,
        ratios: FinancialRatios,
        market_data: Optional[Dict],
        news_sentiment: Optional[float],
        industry: Optional[str]
    ) -> np.ndarray:
        """Prepare feature vector for ML model"""
        features = [
            # Financial ratios
            ratios.roa,
            ratios.roe,
            ratios.profit_margin,
            ratios.operating_margin,
            ratios.debt_to_equity,
            ratios.debt_to_assets,
            ratios.interest_coverage,
            ratios.debt_service_coverage,
            ratios.current_ratio,
            ratios.quick_ratio,
            ratios.cash_ratio,
            ratios.asset_turnover,
            ratios.inventory_turnover,
            ratios.receivables_turnover,
            ratios.revenue_growth,
            ratios.earnings_growth,
            ratios.cash_flow_growth,

            # Market data
            market_data.get('volatility', 0.2) if market_data else 0.2,
            market_data.get('beta', 1.0) if market_data else 1.0,
            market_data.get('market_cap', 1e9) / 1e9 if market_data else 1.0,

            # Sentiment
            news_sentiment if news_sentiment is not None else 0.0,
        ]

        # Pad to input_dim
        while len(features) < 64:
            features.append(0.0)

        return np.array(features[:64], dtype=np.float32)

    def _calculate_credit_score(
        self,
        ratios: FinancialRatios,
        pd: float
    ) -> float:
        """
        Calculate credit score (0-1000)

        Combines:
        - Probability of default (40%)
        - Financial ratios (60%)
        """
        # Score from PD (inverted - lower PD = higher score)
        pd_score = (1 - min(pd, 1.0)) * 400

        # Score from financial ratios
        ratio_scores = []

        # Profitability (0-100)
        profit_score = min(max(ratios.roa * 1000, 0), 100)
        ratio_scores.append(profit_score)

        # Leverage (0-100, lower leverage = higher score)
        leverage_score = min(max(100 - ratios.debt_to_equity * 20, 0), 100)
        ratio_scores.append(leverage_score)

        # Liquidity (0-100)
        liquidity_score = min(max(ratios.current_ratio * 50, 0), 100)
        ratio_scores.append(liquidity_score)

        # Interest coverage (0-100)
        coverage_score = min(max(ratios.interest_coverage * 10, 0), 100)
        ratio_scores.append(coverage_score)

        # Growth (0-100)
        growth_score = min(max(ratios.revenue_growth * 500, 0), 100)
        ratio_scores.append(growth_score)

        # Average ratio score
        avg_ratio_score = np.mean(ratio_scores) * 6  # Scale to 600

        # Combined score
        total_score = pd_score + avg_ratio_score

        return min(max(total_score, 0), 1000)

    def _estimate_lgd(
        self,
        balance_sheet: Dict,
        industry: Optional[str]
    ) -> float:
        """
        Estimate Loss Given Default

        Considers:
        - Asset tangibility
        - Industry
        - Capital structure
        """
        # Base LGD by industry
        industry_lgd = {
            'banking': 0.45,
            'real_estate': 0.30,
            'utilities': 0.35,
            'technology': 0.60,
            'retail': 0.55,
            'manufacturing': 0.40
        }

        base_lgd = industry_lgd.get(industry, 0.45)

        # Adjust for collateral
        total_assets = balance_sheet.get('total_assets', 1)
        intangible_assets = balance_sheet.get('intangible_assets', 0)
        tangible_ratio = (total_assets - intangible_assets) / total_assets

        # Higher tangible assets = lower LGD
        adjusted_lgd = base_lgd * (1 - tangible_ratio * 0.3)

        return min(max(adjusted_lgd, 0.1), 0.9)

    def _identify_risk_factors(
        self,
        ratios: FinancialRatios,
        market_data: Optional[Dict]
    ) -> List[Dict]:
        """Identify key risk factors"""
        risk_factors = []

        # High leverage
        if ratios.debt_to_equity > 2.0:
            risk_factors.append({
                'factor': 'High Leverage',
                'severity': 'High',
                'value': ratios.debt_to_equity,
                'description': f'Debt-to-equity ratio of {ratios.debt_to_equity:.2f} indicates high financial leverage'
            })

        # Low liquidity
        if ratios.current_ratio < 1.0:
            risk_factors.append({
                'factor': 'Low Liquidity',
                'severity': 'High',
                'value': ratios.current_ratio,
                'description': f'Current ratio of {ratios.current_ratio:.2f} may indicate liquidity stress'
            })

        # Poor profitability
        if ratios.roa < 0:
            risk_factors.append({
                'factor': 'Negative Profitability',
                'severity': 'High',
                'value': ratios.roa,
                'description': 'Company is currently unprofitable'
            })

        # Weak interest coverage
        if ratios.interest_coverage < 2.0:
            risk_factors.append({
                'factor': 'Weak Interest Coverage',
                'severity': 'Medium',
                'value': ratios.interest_coverage,
                'description': f'Interest coverage of {ratios.interest_coverage:.2f}x may limit financial flexibility'
            })

        return risk_factors

    def _identify_strengths(self, ratios: FinancialRatios) -> List[str]:
        """Identify company strengths"""
        strengths = []

        if ratios.roa > 0.10:
            strengths.append('Strong profitability with ROA > 10%')

        if ratios.debt_to_equity < 0.5:
            strengths.append('Conservative leverage profile')

        if ratios.current_ratio > 2.0:
            strengths.append('Strong liquidity position')

        if ratios.interest_coverage > 5.0:
            strengths.append('Excellent interest coverage')

        if ratios.revenue_growth > 0.15:
            strengths.append('Robust revenue growth')

        return strengths

    def _identify_weaknesses(self, ratios: FinancialRatios) -> List[str]:
        """Identify company weaknesses"""
        weaknesses = []

        if ratios.roa < 0.02:
            weaknesses.append('Low profitability')

        if ratios.debt_to_equity > 1.5:
            weaknesses.append('High financial leverage')

        if ratios.current_ratio < 1.2:
            weaknesses.append('Tight liquidity')

        if ratios.interest_coverage < 3.0:
            weaknesses.append('Limited interest coverage')

        if ratios.revenue_growth < 0:
            weaknesses.append('Declining revenues')

        return weaknesses

    def _generate_recommendation(
        self,
        rating: str,
        pd: float,
        expected_loss: float
    ) -> str:
        """Generate investment recommendation"""
        if rating in ['AAA', 'AA+', 'AA', 'AA-', 'A+', 'A']:
            return 'STRONG BUY - Investment grade with low default risk'
        elif rating in ['A-', 'BBB+', 'BBB']:
            return 'BUY - Investment grade, acceptable risk'
        elif rating in ['BBB-', 'BB+', 'BB']:
            return 'HOLD - Moderate risk, monitor closely'
        elif rating in ['BB-', 'B+', 'B']:
            return 'CAUTIOUS - High risk, only for risk-tolerant investors'
        else:
            return 'AVOID - Very high default risk'

    def _calculate_confidence(
        self,
        ratios: FinancialRatios,
        market_data: Optional[Dict]
    ) -> float:
        """Calculate confidence in assessment"""
        # Higher confidence if:
        # - More data available
        # - Ratios are stable
        # - Clear credit profile

        confidence = 0.7  # Base confidence

        # Boost if market data available
        if market_data:
            confidence += 0.1

        # Boost if ratios are reasonable (not extreme)
        if 0.5 < ratios.debt_to_equity < 2.0:
            confidence += 0.1

        if ratios.current_ratio > 1.0:
            confidence += 0.1

        return min(confidence, 1.0)


if __name__ == "__main__":
    # Example usage
    print("Credit Risk Analyzer - Example")
    print("=" * 50)

    # Create analyzer
    analyzer = CreditRiskAnalyzer()

    # Sample financial statements
    financial_statements = {
        'balance_sheet': {
            'total_assets': 1_000_000_000,
            'total_equity': 400_000_000,
            'total_debt': 300_000_000,
            'current_assets': 250_000_000,
            'current_liabilities': 150_000_000,
            'cash': 80_000_000,
            'inventory': 50_000_000,
            'intangible_assets': 100_000_000
        },
        'income_statement': {
            'revenue': 800_000_000,
            'net_income': 60_000_000,
            'operating_income': 100_000_000,
            'interest_expense': 15_000_000
        },
        'cash_flow': {
            'operating_cash_flow': 90_000_000
        }
    }

    # Perform analysis
    assessment = analyzer.analyze(
        company_name='Example Corporation',
        financial_statements=financial_statements,
        news_sentiment=0.3,
        industry='technology'
    )

    print(f"\nCredit Assessment for {assessment.company_name}")
    print(f"Credit Score: {assessment.credit_score:.0f}")
    print(f"Rating: {assessment.rating}")
    print(f"Probability of Default: {assessment.probability_of_default:.2%}")
    print(f"Loss Given Default: {assessment.loss_given_default:.2%}")
    print(f"Expected Loss: {assessment.expected_loss:.2%}")
    print(f"Recommendation: {assessment.recommendation}")
    print(f"Confidence: {assessment.confidence:.0%}")

    print(f"\nStrengths:")
    for strength in assessment.strengths:
        print(f"  + {strength}")

    print(f"\nWeaknesses:")
    for weakness in assessment.weaknesses:
        print(f"  - {weakness}")

    print(f"\nKey Risk Factors:")
    for risk in assessment.risk_factors:
        print(f"  [{risk['severity']}] {risk['factor']}: {risk['description']}")
