"""
Financial-Specific Tokenizer
Handles financial terminology, numbers, and special symbols
"""

import re
import json
from typing import List, Dict, Optional, Tuple
from collections import Counter
import pickle


class FinancialTokenizer:
    """
    Tokenizer optimized for financial text

    Features:
    - Financial terminology preservation
    - Number tokenization (prices, volumes, percentages)
    - Ticker symbol handling
    - Date/time format preservation
    - Special financial symbols
    """

    # Financial-specific patterns
    TICKER_PATTERN = r'\b[A-Z]{1,5}\b'  # Stock tickers
    PRICE_PATTERN = r'\$?\d+\.?\d*[KMB]?'  # Prices with K/M/B suffixes
    PERCENTAGE_PATTERN = r'-?\d+\.?\d*%'  # Percentages
    DATE_PATTERN = r'\d{1,2}[-/]\d{1,2}[-/]\d{2,4}'  # Dates
    TIME_PATTERN = r'\d{1,2}:\d{2}(?::\d{2})?(?:\s*[AP]M)?'  # Times

    # Special financial tokens
    SPECIAL_TOKENS = {
        '[PAD]': 0,
        '[UNK]': 1,
        '[CLS]': 2,
        '[SEP]': 3,
        '[MASK]': 4,
        '[BUY]': 5,
        '[SELL]': 6,
        '[HOLD]': 7,
        '[BULL]': 8,
        '[BEAR]': 9,
        '[NEUTRAL]': 10,
        '[NUM]': 11,  # Generic number placeholder
        '[TICKER]': 12,
        '[PRICE]': 13,
        '[VOL]': 14,
        '[DATE]': 15,
        '[TIME]': 16,
    }

    # Common financial terms to preserve
    FINANCIAL_TERMS = {
        # Trading terms
        'bid', 'ask', 'spread', 'volume', 'liquidity', 'slippage',
        'market', 'limit', 'stop', 'order', 'execution', 'fill',
        'long', 'short', 'hedge', 'arbitrage', 'delta', 'gamma',
        'vega', 'theta', 'rho', 'greeks', 'volatility', 'implied',

        # Market terms
        'bullish', 'bearish', 'rally', 'selloff', 'correction', 'crash',
        'support', 'resistance', 'breakout', 'reversal', 'trend',
        'momentum', 'oscillator', 'indicator', 'signal',

        # Instruments
        'equity', 'bond', 'option', 'future', 'swap', 'derivative',
        'call', 'put', 'strike', 'expiry', 'premium', 'underlying',
        'etf', 'index', 'commodity', 'forex', 'crypto',

        # Analytics
        'sharpe', 'sortino', 'alpha', 'beta', 'correlation',
        'drawdown', 'var', 'cvar', 'backtest', 'backtesting',
        'risk', 'return', 'portfolio', 'diversification',

        # HFT specific
        'latency', 'colocation', 'tick', 'microstructure',
        'orderbook', 'depth', 'imbalance', 'toxicity',
        'adverse', 'selection', 'rebate', 'maker', 'taker',

        # Regulatory
        'compliance', 'regulation', 'sec', 'finra', 'mifid',
        'basel', 'dodd', 'frank', 'emir', 'aml', 'kyc',
    }

    def __init__(self, vocab_size: int = 50000):
        self.vocab_size = vocab_size
        self.token_to_id: Dict[str, int] = self.SPECIAL_TOKENS.copy()
        self.id_to_token: Dict[int, str] = {v: k for k, v in self.SPECIAL_TOKENS.items()}
        self.next_id = len(self.SPECIAL_TOKENS)

        # Add financial terms with high priority
        for term in sorted(self.FINANCIAL_TERMS):
            if term not in self.token_to_id:
                self.token_to_id[term] = self.next_id
                self.id_to_token[self.next_id] = term
                self.next_id += 1

    def preprocess_text(self, text: str) -> str:
        """Preprocess text to standardize financial formats"""
        # Lowercase
        text = text.lower()

        # Standardize whitespace
        text = re.sub(r'\s+', ' ', text)

        # Preserve specific patterns by adding spaces
        text = re.sub(self.PERCENTAGE_PATTERN, r' \g<0> ', text)
        text = re.sub(self.PRICE_PATTERN, r' \g<0> ', text)

        return text.strip()

    def tokenize(self, text: str, preserve_special: bool = True) -> List[str]:
        """
        Tokenize text with financial awareness

        Args:
            text: Input text
            preserve_special: Whether to preserve special financial patterns

        Returns:
            List of tokens
        """
        text = self.preprocess_text(text)

        if preserve_special:
            # Extract and replace special patterns
            replacements = []

            # Tickers (before lowercasing)
            original_text = text
            for match in re.finditer(self.TICKER_PATTERN, original_text):
                ticker = match.group()
                replacements.append((ticker, f'[TICKER:{ticker}]'))

            # Prices
            for match in re.finditer(self.PRICE_PATTERN, text):
                price = match.group()
                replacements.append((price, f'[PRICE:{price}]'))

            # Percentages
            for match in re.finditer(self.PERCENTAGE_PATTERN, text):
                pct = match.group()
                replacements.append((pct, f'[PCT:{pct}]'))

            # Apply replacements
            for original, replacement in replacements:
                text = text.replace(original.lower(), replacement)

        # Simple word tokenization
        tokens = re.findall(r'\[[\w:.\-]+\]|\w+|[^\w\s]', text)

        return tokens

    def build_vocab(self, texts: List[str], min_frequency: int = 2):
        """
        Build vocabulary from training texts

        Args:
            texts: List of training texts
            min_frequency: Minimum frequency for a token to be included
        """
        print(f"Building vocabulary from {len(texts)} texts...")

        # Tokenize all texts
        all_tokens = []
        for text in texts:
            all_tokens.extend(self.tokenize(text))

        # Count frequencies
        token_freq = Counter(all_tokens)
        print(f"Found {len(token_freq)} unique tokens")

        # Add tokens that meet frequency threshold
        sorted_tokens = sorted(token_freq.items(), key=lambda x: x[1], reverse=True)

        for token, freq in sorted_tokens:
            if freq < min_frequency:
                break

            if token not in self.token_to_id:
                if self.next_id >= self.vocab_size:
                    break

                self.token_to_id[token] = self.next_id
                self.id_to_token[self.next_id] = token
                self.next_id += 1

        print(f"Vocabulary size: {len(self.token_to_id)}")

    def encode(self, text: str, max_length: Optional[int] = None,
               padding: bool = False, truncation: bool = False) -> List[int]:
        """
        Encode text to token IDs

        Args:
            text: Input text
            max_length: Maximum sequence length
            padding: Whether to pad to max_length
            truncation: Whether to truncate to max_length

        Returns:
            List of token IDs
        """
        tokens = self.tokenize(text)

        # Convert to IDs
        ids = [self.token_to_id.get(token, self.SPECIAL_TOKENS['[UNK]'])
               for token in tokens]

        # Handle length constraints
        if truncation and max_length and len(ids) > max_length:
            ids = ids[:max_length]

        if padding and max_length:
            if len(ids) < max_length:
                ids = ids + [self.SPECIAL_TOKENS['[PAD]']] * (max_length - len(ids))

        return ids

    def decode(self, ids: List[int], skip_special_tokens: bool = True) -> str:
        """
        Decode token IDs to text

        Args:
            ids: List of token IDs
            skip_special_tokens: Whether to skip special tokens

        Returns:
            Decoded text
        """
        tokens = []

        for id in ids:
            if id in self.id_to_token:
                token = self.id_to_token[id]

                # Skip special tokens if requested
                if skip_special_tokens and token in self.SPECIAL_TOKENS:
                    continue

                tokens.append(token)

        # Join tokens
        text = ' '.join(tokens)

        # Clean up spacing
        text = re.sub(r'\s+([.,!?;:])', r'\1', text)
        text = re.sub(r'\(\s+', '(', text)
        text = re.sub(r'\s+\)', ')', text)

        return text

    def save(self, path: str):
        """Save tokenizer to file"""
        data = {
            'vocab_size': self.vocab_size,
            'token_to_id': self.token_to_id,
            'id_to_token': self.id_to_token,
            'next_id': self.next_id
        }

        with open(path, 'wb') as f:
            pickle.dump(data, f)

        print(f"Tokenizer saved to {path}")

    @classmethod
    def load(cls, path: str) -> 'FinancialTokenizer':
        """Load tokenizer from file"""
        with open(path, 'rb') as f:
            data = pickle.load(f)

        tokenizer = cls(vocab_size=data['vocab_size'])
        tokenizer.token_to_id = data['token_to_id']
        tokenizer.id_to_token = data['id_to_token']
        tokenizer.next_id = data['next_id']

        print(f"Tokenizer loaded from {path}")
        return tokenizer

    def get_vocab_size(self) -> int:
        """Return current vocabulary size"""
        return len(self.token_to_id)


class FinancialBPETokenizer:
    """
    Byte-Pair Encoding tokenizer for financial text

    More advanced subword tokenization for better handling of
    rare financial terms and numbers
    """

    def __init__(self, vocab_size: int = 50000):
        self.vocab_size = vocab_size
        self.base_tokenizer = FinancialTokenizer(vocab_size)
        self.merges: List[Tuple[str, str]] = []

    def train_bpe(self, texts: List[str], num_merges: int = 10000):
        """
        Train BPE on financial texts

        Args:
            texts: Training texts
            num_merges: Number of merge operations to perform
        """
        print(f"Training BPE with {num_merges} merges...")

        # Get initial tokens
        word_freqs = Counter()
        for text in texts:
            tokens = self.base_tokenizer.tokenize(text)
            word_freqs.update(tokens)

        # Initialize vocabulary with characters
        vocab = set()
        for word in word_freqs.keys():
            vocab.update(list(word))

        # Perform merges
        for i in range(num_merges):
            # Find most frequent adjacent pair
            pairs = Counter()

            for word, freq in word_freqs.items():
                symbols = list(word)
                for j in range(len(symbols) - 1):
                    pairs[(symbols[j], symbols[j + 1])] += freq

            if not pairs:
                break

            best_pair = max(pairs, key=pairs.get)
            self.merges.append(best_pair)

            # Merge in vocabulary
            new_word_freqs = Counter()
            bigram = ''.join(best_pair)

            for word, freq in word_freqs.items():
                new_word = word.replace(''.join(best_pair), bigram)
                new_word_freqs[new_word] = freq

            word_freqs = new_word_freqs

            if (i + 1) % 1000 == 0:
                print(f"Completed {i + 1} merges")

        print(f"BPE training complete with {len(self.merges)} merges")

    def save(self, path: str):
        """Save BPE tokenizer"""
        self.base_tokenizer.save(path + '.base')

        with open(path + '.merges', 'wb') as f:
            pickle.dump(self.merges, f)

        print(f"BPE tokenizer saved to {path}")

    @classmethod
    def load(cls, path: str) -> 'FinancialBPETokenizer':
        """Load BPE tokenizer"""
        tokenizer = cls()
        tokenizer.base_tokenizer = FinancialTokenizer.load(path + '.base')

        with open(path + '.merges', 'rb') as f:
            tokenizer.merges = pickle.load(f)

        print(f"BPE tokenizer loaded from {path}")
        return tokenizer


if __name__ == "__main__":
    # Example usage
    tokenizer = FinancialTokenizer()

    # Test text
    test_text = """
    AAPL stock rallied 5.2% to $175.50 on strong volume.
    The S&P 500 broke through resistance at 4500.
    High-frequency traders captured the bid-ask spread using low-latency
    algorithms. Market volatility spiked as the VIX reached 25.
    Portfolio managers adjusted delta hedges in options markets.
    """

    print("Original text:")
    print(test_text)
    print("\nTokens:")
    tokens = tokenizer.tokenize(test_text)
    print(tokens)

    print("\nEncoded:")
    encoded = tokenizer.encode(test_text)
    print(encoded[:20], "...")

    print("\nDecoded:")
    decoded = tokenizer.decode(encoded)
    print(decoded)

    print(f"\nVocabulary size: {tokenizer.get_vocab_size()}")
