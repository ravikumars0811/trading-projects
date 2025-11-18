"""
PyTorch: Financial Sentiment Analysis
======================================
Real-world example: Analyzing news and social media for trading signals

Industry Use Case: Quantitative trading firms use sentiment analysis to:
- Generate trading signals from news
- Measure market sentiment in real-time
- Risk monitoring and early warning systems
- Alternative data for alpha generation
"""

import numpy as np
import pandas as pd
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
import re
from collections import Counter
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, precision_recall_fscore_support, confusion_matrix
import warnings
warnings.filterwarnings('ignore')

# Set random seeds
torch.manual_seed(42)
np.random.seed(42)


class FinancialNewsDataset(Dataset):
    """Custom PyTorch Dataset for financial news sentiment"""

    def __init__(self, texts, labels, vocab, max_length=100):
        self.texts = texts
        self.labels = labels
        self.vocab = vocab
        self.max_length = max_length

    def __len__(self):
        return len(self.texts)

    def __getitem__(self, idx):
        text = self.texts[idx]
        label = self.labels[idx]

        # Convert text to token indices
        tokens = self._tokenize(text)
        indices = [self.vocab.get(token, self.vocab['<UNK>']) for token in tokens]

        # Pad or truncate to max_length
        if len(indices) < self.max_length:
            indices += [self.vocab['<PAD>']] * (self.max_length - len(indices))
        else:
            indices = indices[:self.max_length]

        return torch.tensor(indices, dtype=torch.long), torch.tensor(label, dtype=torch.long)

    def _tokenize(self, text):
        """Simple tokenization"""
        text = text.lower()
        text = re.sub(r'[^a-zA-Z0-9\s]', '', text)
        return text.split()


class SentimentLSTM(nn.Module):
    """
    LSTM-based sentiment classifier
    Architecture: Embedding → LSTM → Attention → Dense → Output
    """

    def __init__(self, vocab_size, embedding_dim=128, hidden_dim=256, num_classes=3, dropout=0.3):
        super(SentimentLSTM, self).__init__()

        self.embedding = nn.Embedding(vocab_size, embedding_dim, padding_idx=0)
        self.lstm = nn.LSTM(
            embedding_dim,
            hidden_dim,
            num_layers=2,
            batch_first=True,
            dropout=dropout,
            bidirectional=True
        )

        # Attention mechanism
        self.attention = nn.Linear(hidden_dim * 2, 1)

        # Classification layers
        self.fc1 = nn.Linear(hidden_dim * 2, 128)
        self.dropout = nn.Dropout(dropout)
        self.fc2 = nn.Linear(128, num_classes)

        self.relu = nn.ReLU()

    def forward(self, x):
        # x shape: (batch_size, seq_length)

        # Embedding
        embedded = self.embedding(x)  # (batch_size, seq_length, embedding_dim)

        # LSTM
        lstm_out, _ = self.lstm(embedded)  # (batch_size, seq_length, hidden_dim*2)

        # Attention mechanism
        attention_weights = torch.softmax(self.attention(lstm_out), dim=1)
        attended = torch.sum(attention_weights * lstm_out, dim=1)  # (batch_size, hidden_dim*2)

        # Classification
        out = self.fc1(attended)
        out = self.relu(out)
        out = self.dropout(out)
        out = self.fc2(out)

        return out


class FinancialSentimentAnalyzer:
    """
    Production-grade sentiment analyzer for financial text
    """

    def __init__(self, max_length=100):
        self.max_length = max_length
        self.vocab = None
        self.model = None
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        print(f"Using device: {self.device}")

    def generate_financial_news_data(self, n_samples=5000):
        """
        Generate synthetic financial news headlines with sentiment labels
        In production, use real news APIs (NewsAPI, Bloomberg, Reuters)
        """
        np.random.seed(42)

        # Positive sentiment templates
        positive_templates = [
            "{company} reports record earnings, beats expectations",
            "{company} stock surges on strong quarterly results",
            "{company} announces breakthrough innovation in {sector}",
            "{company} expands market share, revenue up {percent}%",
            "Analysts upgrade {company} stock to buy rating",
            "{company} CEO optimistic about future growth prospects",
            "{company} secures major contract worth ${amount}M",
            "{company} launches successful product, sales exceed forecasts",
        ]

        # Negative sentiment templates
        negative_templates = [
            "{company} misses earnings estimates, stock plunges",
            "{company} faces regulatory investigation over {issue}",
            "{company} announces layoffs, cuts {percent}% of workforce",
            "Analysts downgrade {company} amid declining revenue",
            "{company} reports significant loss in Q{quarter}",
            "{company} stock tumbles on disappointing guidance",
            "{company} faces supply chain disruptions, delays expected",
            "{company} CEO steps down amid controversy",
        ]

        # Neutral sentiment templates
        neutral_templates = [
            "{company} holds quarterly earnings call, results in line with expectations",
            "{company} announces board meeting scheduled for next month",
            "{company} maintains dividend at current levels",
            "{company} files routine SEC disclosure documents",
            "{company} CEO speaks at industry conference",
            "{company} announces regular quarterly dividend",
            "{company} updates investor relations website",
            "{company} reports standard quarterly metrics",
        ]

        companies = ["Apple", "Microsoft", "Amazon", "Google", "Tesla", "Meta",
                     "NVIDIA", "JPMorgan", "Goldman Sachs", "Morgan Stanley"]
        sectors = ["AI", "cloud computing", "electric vehicles", "fintech", "biotech"]
        issues = ["data privacy", "antitrust", "securities fraud", "environmental concerns"]

        data = []

        # Generate samples
        for _ in range(n_samples):
            sentiment = np.random.choice([0, 1, 2], p=[0.3, 0.4, 0.3])  # negative, neutral, positive

            if sentiment == 2:  # Positive
                template = np.random.choice(positive_templates)
            elif sentiment == 1:  # Neutral
                template = np.random.choice(neutral_templates)
            else:  # Negative
                template = np.random.choice(negative_templates)

            # Fill template
            text = template.format(
                company=np.random.choice(companies),
                sector=np.random.choice(sectors),
                percent=np.random.randint(5, 50),
                amount=np.random.randint(10, 500),
                quarter=np.random.randint(1, 5),
                issue=np.random.choice(issues)
            )

            data.append({'text': text, 'sentiment': sentiment})

        return pd.DataFrame(data)

    def build_vocabulary(self, texts, min_freq=2):
        """Build vocabulary from texts"""
        all_tokens = []
        for text in texts:
            text = text.lower()
            text = re.sub(r'[^a-zA-Z0-9\s]', '', text)
            all_tokens.extend(text.split())

        # Count token frequencies
        token_counts = Counter(all_tokens)

        # Create vocabulary
        vocab = {'<PAD>': 0, '<UNK>': 1}
        for token, count in token_counts.items():
            if count >= min_freq:
                vocab[token] = len(vocab)

        print(f"Vocabulary size: {len(vocab)}")
        return vocab

    def train(self, train_texts, train_labels, val_texts, val_labels,
              epochs=20, batch_size=32, learning_rate=0.001):
        """Train the sentiment model"""

        # Build vocabulary
        print("\nBuilding vocabulary...")
        self.vocab = self.build_vocabulary(train_texts)

        # Create datasets
        train_dataset = FinancialNewsDataset(train_texts, train_labels, self.vocab, self.max_length)
        val_dataset = FinancialNewsDataset(val_texts, val_labels, self.vocab, self.max_length)

        train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
        val_loader = DataLoader(val_dataset, batch_size=batch_size)

        # Initialize model
        print("\nBuilding PyTorch LSTM model...")
        self.model = SentimentLSTM(
            vocab_size=len(self.vocab),
            embedding_dim=128,
            hidden_dim=256,
            num_classes=3,
            dropout=0.3
        ).to(self.device)

        print(f"\nModel Parameters: {sum(p.numel() for p in self.model.parameters()):,}")

        # Loss and optimizer
        criterion = nn.CrossEntropyLoss()
        optimizer = optim.Adam(self.model.parameters(), lr=learning_rate)
        scheduler = optim.lr_scheduler.ReduceLROnPlateau(
            optimizer, mode='min', factor=0.5, patience=3, verbose=True
        )

        # Training loop
        print(f"\nTraining for {epochs} epochs...")
        best_val_loss = float('inf')

        for epoch in range(epochs):
            # Training phase
            self.model.train()
            train_loss = 0
            train_correct = 0
            train_total = 0

            for texts, labels in train_loader:
                texts, labels = texts.to(self.device), labels.to(self.device)

                optimizer.zero_grad()
                outputs = self.model(texts)
                loss = criterion(outputs, labels)
                loss.backward()
                optimizer.step()

                train_loss += loss.item()
                _, predicted = torch.max(outputs.data, 1)
                train_total += labels.size(0)
                train_correct += (predicted == labels).sum().item()

            # Validation phase
            self.model.eval()
            val_loss = 0
            val_correct = 0
            val_total = 0

            with torch.no_grad():
                for texts, labels in val_loader:
                    texts, labels = texts.to(self.device), labels.to(self.device)
                    outputs = self.model(texts)
                    loss = criterion(outputs, labels)

                    val_loss += loss.item()
                    _, predicted = torch.max(outputs.data, 1)
                    val_total += labels.size(0)
                    val_correct += (predicted == labels).sum().item()

            # Calculate averages
            avg_train_loss = train_loss / len(train_loader)
            avg_val_loss = val_loss / len(val_loader)
            train_acc = 100 * train_correct / train_total
            val_acc = 100 * val_correct / val_total

            # Print progress
            print(f"Epoch {epoch + 1}/{epochs}:")
            print(f"  Train Loss: {avg_train_loss:.4f}, Train Acc: {train_acc:.2f}%")
            print(f"  Val Loss:   {avg_val_loss:.4f}, Val Acc:   {val_acc:.2f}%")

            # Learning rate scheduling
            scheduler.step(avg_val_loss)

            # Save best model
            if avg_val_loss < best_val_loss:
                best_val_loss = avg_val_loss
                torch.save(self.model.state_dict(), 'best_sentiment_model.pth')

        print("\n✓ Training completed")

    def evaluate(self, test_texts, test_labels):
        """Comprehensive model evaluation"""
        print("\n" + "=" * 70)
        print("MODEL EVALUATION REPORT")
        print("=" * 70)

        # Create dataset
        test_dataset = FinancialNewsDataset(test_texts, test_labels, self.vocab, self.max_length)
        test_loader = DataLoader(test_dataset, batch_size=32)

        # Predictions
        self.model.eval()
        all_predictions = []
        all_labels = []

        with torch.no_grad():
            for texts, labels in test_loader:
                texts = texts.to(self.device)
                outputs = self.model(texts)
                _, predicted = torch.max(outputs.data, 1)
                all_predictions.extend(predicted.cpu().numpy())
                all_labels.extend(labels.numpy())

        # Calculate metrics
        accuracy = accuracy_score(all_labels, all_predictions)
        precision, recall, f1, _ = precision_recall_fscore_support(
            all_labels, all_predictions, average='weighted'
        )

        print(f"\nOverall Metrics:")
        print(f"  Accuracy:  {accuracy:.4f}")
        print(f"  Precision: {precision:.4f}")
        print(f"  Recall:    {recall:.4f}")
        print(f"  F1 Score:  {f1:.4f}")

        # Per-class metrics
        print("\nPer-Class Performance:")
        class_names = ['Negative', 'Neutral', 'Positive']
        precision, recall, f1, support = precision_recall_fscore_support(
            all_labels, all_predictions, average=None
        )

        for i, name in enumerate(class_names):
            print(f"  {name}:")
            print(f"    Precision: {precision[i]:.4f}")
            print(f"    Recall:    {recall[i]:.4f}")
            print(f"    F1 Score:  {f1[i]:.4f}")
            print(f"    Support:   {support[i]}")

        # Confusion Matrix
        cm = confusion_matrix(all_labels, all_predictions)
        print("\nConfusion Matrix:")
        print("                Predicted")
        print("              Neg  Neu  Pos")
        for i, name in enumerate(class_names):
            print(f"  Actual {name:8s} {cm[i][0]:4d} {cm[i][1]:4d} {cm[i][2]:4d}")

        return {'accuracy': accuracy, 'f1': f1}

    def predict_sentiment(self, text):
        """Predict sentiment for a single text"""
        self.model.eval()

        # Tokenize and convert to indices
        tokens = text.lower()
        tokens = re.sub(r'[^a-zA-Z0-9\s]', '', tokens).split()
        indices = [self.vocab.get(token, self.vocab['<UNK>']) for token in tokens]

        # Pad or truncate
        if len(indices) < self.max_length:
            indices += [self.vocab['<PAD>']] * (self.max_length - len(indices))
        else:
            indices = indices[:self.max_length]

        # Convert to tensor
        input_tensor = torch.tensor([indices], dtype=torch.long).to(self.device)

        # Predict
        with torch.no_grad():
            output = self.model(input_tensor)
            probabilities = torch.softmax(output, dim=1)[0]
            predicted_class = torch.argmax(probabilities).item()

        sentiment_map = {0: 'NEGATIVE', 1: 'NEUTRAL', 2: 'POSITIVE'}
        confidence = probabilities[predicted_class].item()

        return {
            'sentiment': sentiment_map[predicted_class],
            'confidence': confidence,
            'probabilities': {
                'negative': probabilities[0].item(),
                'neutral': probabilities[1].item(),
                'positive': probabilities[2].item()
            }
        }


def main():
    """Demo: Complete sentiment analysis pipeline"""
    print("=" * 70)
    print("FINANCIAL SENTIMENT ANALYSIS WITH PYTORCH")
    print("=" * 70)

    # Initialize analyzer
    analyzer = FinancialSentimentAnalyzer(max_length=50)

    # Generate data
    print("\n1. GENERATING FINANCIAL NEWS DATA...")
    df = analyzer.generate_financial_news_data(n_samples=5000)
    print(f"   Generated {len(df):,} news headlines")
    print(f"   Sentiment distribution:")
    print(f"     Negative: {(df['sentiment'] == 0).sum():,}")
    print(f"     Neutral:  {(df['sentiment'] == 1).sum():,}")
    print(f"     Positive: {(df['sentiment'] == 2).sum():,}")

    # Split data
    print("\n2. SPLITTING DATA...")
    train_texts, test_texts, train_labels, test_labels = train_test_split(
        df['text'].values, df['sentiment'].values, test_size=0.2, random_state=42
    )
    train_texts, val_texts, train_labels, val_labels = train_test_split(
        train_texts, train_labels, test_size=0.2, random_state=42
    )

    print(f"   Training: {len(train_texts):,} samples")
    print(f"   Validation: {len(val_texts):,} samples")
    print(f"   Test: {len(test_texts):,} samples")

    # Train model
    print("\n3. TRAINING MODEL...")
    analyzer.train(train_texts, train_labels, val_texts, val_labels,
                   epochs=15, batch_size=64, learning_rate=0.001)

    # Evaluate model
    print("\n4. EVALUATING MODEL...")
    metrics = analyzer.evaluate(test_texts, test_labels)

    # Test predictions
    print("\n5. TESTING PREDICTIONS ON NEW HEADLINES...")
    print("-" * 70)

    test_headlines = [
        "Apple reports record-breaking Q3 earnings, stock soars 10%",
        "Tesla faces recall over safety concerns, shares tumble",
        "Microsoft announces regular quarterly dividend payment",
        "Amazon expands into healthcare with major acquisition",
        "Goldman Sachs downgrades tech stocks amid recession fears"
    ]

    for headline in test_headlines:
        result = analyzer.predict_sentiment(headline)
        print(f"\nHeadline: {headline}")
        print(f"  Sentiment: {result['sentiment']} (confidence: {result['confidence']:.2%})")

    print("\n" + "=" * 70)
    print("SENTIMENT ANALYSIS PIPELINE COMPLETE")
    print("=" * 70)


if __name__ == "__main__":
    main()
