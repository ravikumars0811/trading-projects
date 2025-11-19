"""
Scikit-Learn: Credit Risk Prediction
=====================================
Real-world example: Predicting loan default risk for banking institutions

Industry Use Case: Banks and financial institutions use ML models to assess
creditworthiness, reduce default rates, and comply with Basel III regulations.
"""

import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split, cross_val_score, GridSearchCV
from sklearn.preprocessing import StandardScaler, LabelEncoder
from sklearn.ensemble import RandomForestClassifier, GradientBoostingClassifier
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import (
    classification_report, confusion_matrix, roc_auc_score,
    roc_curve, precision_recall_curve, f1_score
)
from sklearn.pipeline import Pipeline
import pickle
from datetime import datetime
import warnings
warnings.filterwarnings('ignore')


class CreditRiskModel:
    """
    Production-grade credit risk assessment model
    Compliant with fair lending practices and model governance
    """

    def __init__(self, model_type='random_forest'):
        self.model_type = model_type
        self.model = None
        self.scaler = StandardScaler()
        self.feature_names = None
        self.model_metadata = {
            'created_at': datetime.now().isoformat(),
            'model_type': model_type,
            'version': '1.0.0'
        }

    def create_synthetic_data(self, n_samples=10000):
        """
        Generate synthetic loan application data
        In production, this would come from credit bureaus and internal databases
        """
        np.random.seed(42)

        # Customer demographics and financial features
        data = {
            'age': np.random.randint(18, 75, n_samples),
            'income': np.random.lognormal(10.5, 0.8, n_samples),  # Log-normal distribution
            'employment_years': np.random.randint(0, 40, n_samples),
            'debt_to_income': np.random.uniform(0, 1.5, n_samples),
            'credit_score': np.random.normal(650, 100, n_samples).clip(300, 850),
            'loan_amount': np.random.lognormal(10, 0.7, n_samples),
            'loan_term_months': np.random.choice([12, 24, 36, 48, 60], n_samples),
            'num_credit_lines': np.random.randint(0, 20, n_samples),
            'num_delinquencies': np.random.poisson(0.5, n_samples),
            'revolving_balance': np.random.lognormal(8, 1, n_samples),
            'utilization_rate': np.random.uniform(0, 1, n_samples),
            'home_ownership': np.random.choice(['RENT', 'OWN', 'MORTGAGE'], n_samples),
            'purpose': np.random.choice([
                'debt_consolidation', 'credit_card', 'home_improvement',
                'major_purchase', 'business', 'medical'
            ], n_samples),
            'employment_status': np.random.choice([
                'employed', 'self_employed', 'unemployed'
            ], n_samples, p=[0.7, 0.2, 0.1])
        }

        df = pd.DataFrame(data)

        # Engineer features
        df['loan_to_income'] = df['loan_amount'] / df['income']
        df['monthly_payment'] = df['loan_amount'] / df['loan_term_months']
        df['payment_to_income'] = (df['monthly_payment'] * 12) / df['income']

        # Create target variable (default) based on risk factors
        default_probability = self._calculate_default_probability(df)
        df['default'] = (np.random.random(n_samples) < default_probability).astype(int)

        return df

    def _calculate_default_probability(self, df):
        """Calculate default probability based on risk factors"""
        prob = 0.1  # Base default rate

        # Risk factors that increase default probability
        prob += (df['credit_score'] < 580) * 0.3
        prob += (df['debt_to_income'] > 0.8) * 0.2
        prob += (df['num_delinquencies'] > 2) * 0.25
        prob += (df['employment_status'] == 'unemployed') * 0.3
        prob += (df['utilization_rate'] > 0.8) * 0.15
        prob += (df['payment_to_income'] > 0.4) * 0.2

        # Protective factors that decrease default probability
        prob -= (df['credit_score'] > 750) * 0.15
        prob -= (df['employment_years'] > 10) * 0.1
        prob -= (df['home_ownership'] == 'OWN') * 0.1

        return prob.clip(0, 0.9)

    def preprocess_data(self, df, fit=True):
        """Preprocess data for modeling"""
        df_processed = df.copy()

        # Encode categorical variables
        categorical_cols = ['home_ownership', 'purpose', 'employment_status']

        for col in categorical_cols:
            le = LabelEncoder()
            if fit:
                df_processed[col] = le.fit_transform(df_processed[col])
            else:
                df_processed[col] = le.transform(df_processed[col])

        return df_processed

    def train(self, X_train, y_train):
        """Train the credit risk model"""
        print(f"Training {self.model_type} model...")

        if self.model_type == 'random_forest':
            self.model = RandomForestClassifier(
                n_estimators=100,
                max_depth=15,
                min_samples_split=100,
                min_samples_leaf=50,
                class_weight='balanced',  # Handle imbalanced data
                random_state=42,
                n_jobs=-1
            )
        elif self.model_type == 'gradient_boosting':
            self.model = GradientBoostingClassifier(
                n_estimators=100,
                learning_rate=0.1,
                max_depth=5,
                random_state=42
            )
        else:  # logistic_regression
            self.model = LogisticRegression(
                class_weight='balanced',
                max_iter=1000,
                random_state=42
            )

        # Scale features
        X_train_scaled = self.scaler.fit_transform(X_train)

        # Train model
        self.model.fit(X_train_scaled, y_train)

        # Store feature names
        self.feature_names = X_train.columns.tolist()

        print("✓ Model training completed")

    def evaluate(self, X_test, y_test):
        """Comprehensive model evaluation"""
        X_test_scaled = self.scaler.transform(X_test)
        y_pred = self.model.predict(X_test_scaled)
        y_pred_proba = self.model.predict_proba(X_test_scaled)[:, 1]

        print("\n" + "=" * 70)
        print("MODEL EVALUATION REPORT")
        print("=" * 70)

        # Classification metrics
        print("\nClassification Report:")
        print(classification_report(y_test, y_pred, target_names=['No Default', 'Default']))

        # Confusion Matrix
        cm = confusion_matrix(y_test, y_pred)
        print("\nConfusion Matrix:")
        print(f"  True Negatives:  {cm[0][0]:,}")
        print(f"  False Positives: {cm[0][1]:,}")
        print(f"  False Negatives: {cm[1][0]:,}")
        print(f"  True Positives:  {cm[1][1]:,}")

        # ROC AUC Score
        roc_auc = roc_auc_score(y_test, y_pred_proba)
        print(f"\nROC AUC Score: {roc_auc:.4f}")

        # F1 Score
        f1 = f1_score(y_test, y_pred)
        print(f"F1 Score: {f1:.4f}")

        # Business metrics
        self._calculate_business_metrics(y_test, y_pred, cm)

        return {
            'roc_auc': roc_auc,
            'f1_score': f1,
            'confusion_matrix': cm
        }

    def _calculate_business_metrics(self, y_test, y_pred, cm):
        """Calculate business-relevant metrics"""
        print("\n" + "-" * 70)
        print("BUSINESS IMPACT METRICS")
        print("-" * 70)

        # Assuming average loan amount of $50,000
        avg_loan = 50000
        default_loss_rate = 0.6  # Banks typically recover 40% from defaults

        # False Negatives = Approved loans that defaulted
        expected_loss = cm[1][0] * avg_loan * default_loss_rate
        print(f"Expected Loss from Missed Defaults: ${expected_loss:,.2f}")

        # False Positives = Good customers rejected
        opportunity_cost = cm[0][1] * avg_loan * 0.05  # 5% profit margin
        print(f"Opportunity Cost from False Rejections: ${opportunity_cost:,.2f}")

        # Default rate in approved loans
        approved_defaults = cm[1][1] / (cm[0][1] + cm[1][1]) if (cm[0][1] + cm[1][1]) > 0 else 0
        print(f"Default Rate in Approved Loans: {approved_defaults:.2%}")

    def get_feature_importance(self):
        """Get and display feature importance"""
        if hasattr(self.model, 'feature_importances_'):
            importance = pd.DataFrame({
                'feature': self.feature_names,
                'importance': self.model.feature_importances_
            }).sort_values('importance', ascending=False)

            print("\n" + "-" * 70)
            print("FEATURE IMPORTANCE (Top 10)")
            print("-" * 70)
            for idx, row in importance.head(10).iterrows():
                print(f"{row['feature']:30s} {row['importance']:.4f}")

            return importance
        else:
            print("Feature importance not available for this model type")
            return None

    def predict_risk(self, customer_data):
        """Predict default risk for a new customer"""
        customer_scaled = self.scaler.transform(customer_data)
        default_prob = self.model.predict_proba(customer_scaled)[:, 1][0]
        prediction = self.model.predict(customer_scaled)[0]

        # Risk categorization
        if default_prob < 0.2:
            risk_level = "LOW"
        elif default_prob < 0.5:
            risk_level = "MEDIUM"
        else:
            risk_level = "HIGH"

        return {
            'default_probability': default_prob,
            'predicted_default': bool(prediction),
            'risk_level': risk_level,
            'decision': 'APPROVE' if default_prob < 0.5 else 'REJECT'
        }

    def save_model(self, filepath='credit_risk_model.pkl'):
        """Save model for production deployment"""
        model_package = {
            'model': self.model,
            'scaler': self.scaler,
            'feature_names': self.feature_names,
            'metadata': self.model_metadata
        }
        with open(filepath, 'wb') as f:
            pickle.dump(model_package, f)
        print(f"\n✓ Model saved to {filepath}")


def main():
    """Demo: Complete credit risk modeling pipeline"""
    print("=" * 70)
    print("CREDIT RISK PREDICTION MODEL")
    print("=" * 70)

    # Initialize model
    credit_model = CreditRiskModel(model_type='random_forest')

    # Generate synthetic data
    print("\n1. GENERATING SYNTHETIC LOAN DATA...")
    df = credit_model.create_synthetic_data(n_samples=10000)
    print(f"   Generated {len(df):,} loan applications")
    print(f"   Default rate: {df['default'].mean():.2%}")

    # Prepare data
    print("\n2. PREPROCESSING DATA...")
    df_processed = credit_model.preprocess_data(df)

    # Split features and target
    X = df_processed.drop('default', axis=1)
    y = df_processed['default']

    # Train-test split
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42, stratify=y
    )
    print(f"   Training set: {len(X_train):,} samples")
    print(f"   Test set: {len(X_test):,} samples")

    # Train model
    print("\n3. TRAINING MODEL...")
    credit_model.train(X_train, y_train)

    # Evaluate model
    print("\n4. EVALUATING MODEL...")
    metrics = credit_model.evaluate(X_test, y_test)

    # Feature importance
    print("\n5. ANALYZING FEATURE IMPORTANCE...")
    credit_model.get_feature_importance()

    # Test prediction on new customer
    print("\n6. TESTING PREDICTION ON NEW APPLICATION...")
    print("-" * 70)

    # Create a sample customer
    sample_customer = pd.DataFrame({
        'age': [35],
        'income': [75000],
        'employment_years': [8],
        'debt_to_income': [0.35],
        'credit_score': [720],
        'loan_amount': [30000],
        'loan_term_months': [36],
        'num_credit_lines': [5],
        'num_delinquencies': [0],
        'revolving_balance': [5000],
        'utilization_rate': [0.3],
        'home_ownership': [1],  # Already encoded
        'purpose': [0],
        'employment_status': [0],
        'loan_to_income': [0.4],
        'monthly_payment': [833.33],
        'payment_to_income': [0.133]
    })

    result = credit_model.predict_risk(sample_customer)
    print(f"Default Probability: {result['default_probability']:.2%}")
    print(f"Risk Level: {result['risk_level']}")
    print(f"Decision: {result['decision']}")

    # Save model
    print("\n7. SAVING MODEL FOR PRODUCTION...")
    credit_model.save_model('data/credit_risk_model.pkl')

    print("\n" + "=" * 70)
    print("CREDIT RISK MODEL PIPELINE COMPLETE")
    print("=" * 70)


if __name__ == "__main__":
    main()
