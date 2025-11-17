"""
Module 1.2: Embeddings - Converting Text to Meaningful Vectors

Embeddings are the backbone of semantic search, similarity matching,
and many AI applications in finance.
"""

import numpy as np
from typing import List, Tuple
import openai
from sentence_transformers import SentenceTransformer
from sklearn.metrics.pairwise import cosine_similarity
import os
from dotenv import load_dotenv

load_dotenv()


class EmbeddingDemo:
    """
    Demonstrates embeddings for financial text analysis.
    Critical for: semantic search, document similarity, clustering.
    """

    def __init__(self):
        # OpenAI embeddings (requires API key)
        self.openai_client = openai.OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

        # Local embeddings (free, runs on your machine)
        self.local_model = SentenceTransformer('all-MiniLM-L6-v2')

    def get_openai_embedding(self, text: str, model: str = "text-embedding-3-small") -> np.ndarray:
        """
        Get embeddings from OpenAI API.

        Args:
            text: Input text
            model: Embedding model (text-embedding-3-small or text-embedding-3-large)

        Returns:
            Embedding vector
        """
        response = self.openai_client.embeddings.create(
            input=text,
            model=model
        )
        return np.array(response.data[0].embedding)

    def get_local_embedding(self, text: str) -> np.ndarray:
        """
        Get embeddings using local model (no API call needed).

        Args:
            text: Input text

        Returns:
            Embedding vector
        """
        return self.local_model.encode(text)

    def semantic_similarity(self, text1: str, text2: str) -> dict:
        """
        Calculate semantic similarity between two texts.
        Useful for: finding similar trades, matching market conditions, etc.

        Args:
            text1, text2: Texts to compare

        Returns:
            Similarity scores
        """
        # Get embeddings
        emb1_local = self.get_local_embedding(text1)
        emb2_local = self.get_local_embedding(text2)

        # Calculate cosine similarity
        similarity_local = cosine_similarity(
            emb1_local.reshape(1, -1),
            emb2_local.reshape(1, -1)
        )[0][0]

        result = {
            "text1": text1,
            "text2": text2,
            "similarity": float(similarity_local),
            "interpretation": self._interpret_similarity(similarity_local)
        }

        # Optionally use OpenAI embeddings (if API key available)
        if os.getenv("OPENAI_API_KEY"):
            try:
                emb1_openai = self.get_openai_embedding(text1)
                emb2_openai = self.get_openai_embedding(text2)
                similarity_openai = cosine_similarity(
                    emb1_openai.reshape(1, -1),
                    emb2_openai.reshape(1, -1)
                )[0][0]
                result["openai_similarity"] = float(similarity_openai)
            except Exception as e:
                result["openai_similarity"] = f"Error: {str(e)}"

        return result

    def _interpret_similarity(self, score: float) -> str:
        """Interpret similarity score."""
        if score > 0.8:
            return "Very similar"
        elif score > 0.6:
            return "Moderately similar"
        elif score > 0.4:
            return "Somewhat similar"
        else:
            return "Different"

    def find_similar_documents(
        self,
        query: str,
        documents: List[str],
        top_k: int = 3
    ) -> List[Tuple[str, float]]:
        """
        Find most similar documents to query.
        Use case: Find similar historical market conditions, trades, or news.

        Args:
            query: Search query
            documents: List of documents to search
            top_k: Number of results to return

        Returns:
            List of (document, similarity_score) tuples
        """
        # Get query embedding
        query_emb = self.get_local_embedding(query)

        # Get document embeddings
        doc_embeddings = [self.get_local_embedding(doc) for doc in documents]

        # Calculate similarities
        similarities = [
            cosine_similarity(
                query_emb.reshape(1, -1),
                doc_emb.reshape(1, -1)
            )[0][0]
            for doc_emb in doc_embeddings
        ]

        # Sort and return top k
        doc_scores = list(zip(documents, similarities))
        doc_scores.sort(key=lambda x: x[1], reverse=True)

        return doc_scores[:top_k]

    def cluster_financial_news(self, news_items: List[str]) -> dict:
        """
        Group similar news items together.
        Use case: Categorize market news, identify themes.

        Args:
            news_items: List of news headlines/snippets

        Returns:
            Clustering results with similarity matrix
        """
        # Get embeddings for all news items
        embeddings = np.array([self.get_local_embedding(news) for news in news_items])

        # Calculate similarity matrix
        similarity_matrix = cosine_similarity(embeddings)

        # Simple clustering: find items that are very similar (>0.7)
        clusters = []
        used = set()

        for i in range(len(news_items)):
            if i in used:
                continue

            cluster = [i]
            for j in range(i + 1, len(news_items)):
                if j not in used and similarity_matrix[i][j] > 0.7:
                    cluster.append(j)
                    used.add(j)

            clusters.append(cluster)
            used.add(i)

        return {
            "clusters": [
                [news_items[idx] for idx in cluster]
                for cluster in clusters
            ],
            "similarity_matrix": similarity_matrix.tolist(),
            "num_clusters": len(clusters)
        }


class FinancialSemanticSearch:
    """
    Production-ready semantic search for financial documents.
    """

    def __init__(self):
        self.model = SentenceTransformer('all-MiniLM-L6-v2')
        self.document_embeddings = None
        self.documents = None

    def index_documents(self, documents: List[str]):
        """
        Create searchable index of documents.

        Args:
            documents: List of documents to index
        """
        self.documents = documents
        print(f"Indexing {len(documents)} documents...")
        self.document_embeddings = self.model.encode(
            documents,
            show_progress_bar=True
        )
        print("Indexing complete!")

    def search(self, query: str, top_k: int = 5) -> List[dict]:
        """
        Search indexed documents.

        Args:
            query: Search query
            top_k: Number of results

        Returns:
            Search results with scores
        """
        if self.document_embeddings is None:
            raise ValueError("No documents indexed. Call index_documents() first.")

        # Get query embedding
        query_embedding = self.model.encode([query])[0]

        # Calculate similarities
        similarities = cosine_similarity(
            query_embedding.reshape(1, -1),
            self.document_embeddings
        )[0]

        # Get top results
        top_indices = np.argsort(similarities)[-top_k:][::-1]

        results = [
            {
                "document": self.documents[idx],
                "score": float(similarities[idx]),
                "rank": rank + 1
            }
            for rank, idx in enumerate(top_indices)
        ]

        return results


def main():
    """Run embedding demonstrations."""

    demo = EmbeddingDemo()

    # Example 1: Semantic similarity of financial statements
    print("Example 1: Semantic Similarity Analysis")
    print("="*80)

    pairs = [
        (
            "Strong earnings beat expectations, revenue up 15%",
            "Company reports record quarterly profits exceeding forecasts"
        ),
        (
            "Federal Reserve raises interest rates by 0.5%",
            "Apple launches new iPhone with advanced camera"
        ),
        (
            "Stock market crashes on recession fears",
            "Major market selloff amid economic concerns"
        ),
    ]

    for text1, text2 in pairs:
        result = demo.semantic_similarity(text1, text2)
        print(f"\nText 1: {result['text1']}")
        print(f"Text 2: {result['text2']}")
        print(f"Similarity: {result['similarity']:.4f} ({result['interpretation']})")

    # Example 2: Find similar market conditions
    print("\n\nExample 2: Finding Similar Historical Market Conditions")
    print("="*80)

    current_condition = "High volatility with VIX above 30, tech stocks down 5%"

    historical_conditions = [
        "VIX spiked to 35, technology sector declined 6% in heavy selling",
        "Market rallied 3% on positive economic data, VIX fell to 15",
        "Extreme volatility observed, VIX hit 32 as growth stocks tumbled",
        "Bond yields rising, value stocks outperform growth by 4%",
        "Market uncertainty peaks with VIX at 28, tech selloff continues",
    ]

    similar = demo.find_similar_documents(current_condition, historical_conditions)

    print(f"\nCurrent condition: {current_condition}\n")
    print("Most similar historical conditions:")
    for doc, score in similar:
        print(f"  [{score:.4f}] {doc}")

    # Example 3: News clustering
    print("\n\nExample 3: Clustering Financial News")
    print("="*80)

    news_items = [
        "Apple stock hits all-time high on strong iPhone sales",
        "Microsoft reports cloud revenue growth of 25%",
        "AAPL shares reach record levels driven by device demand",
        "Federal Reserve maintains interest rates at current levels",
        "Amazon Web Services grows 30% year-over-year",
        "Fed keeps rates unchanged in latest policy meeting",
        "Google Cloud Platform revenue surges in Q3",
    ]

    clustering_result = demo.cluster_financial_news(news_items)

    print(f"\nFound {clustering_result['num_clusters']} distinct themes:\n")
    for i, cluster in enumerate(clustering_result['clusters'], 1):
        print(f"Theme {i}:")
        for item in cluster:
            print(f"  - {item}")
        print()

    # Example 4: Production semantic search
    print("\n\nExample 4: Production Semantic Search System")
    print("="*80)

    # Create search index
    search_engine = FinancialSemanticSearch()

    # Sample financial documents
    documents = [
        "Q4 2023 earnings call: Revenue exceeded guidance at $150M, up 20% YoY. EBITDA margin improved to 35%.",
        "Risk assessment: High exposure to interest rate risk. Duration of bond portfolio is 7 years.",
        "M&A update: Acquisition of TechCorp completed. Synergies expected to reach $50M annually.",
        "Market commentary: Equity markets volatile amid inflation concerns. Fed likely to maintain hawkish stance.",
        "Trading strategy: Momentum-based approach focusing on stocks with strong relative strength.",
        "Credit analysis: Company maintains strong balance sheet with debt-to-equity ratio of 0.3.",
        "Macro outlook: Economic growth expected to slow to 2% in 2024. Unemployment to rise modestly.",
        "Portfolio update: Reduced technology allocation from 30% to 25%, increased healthcare to 15%.",
        "Derivatives: Long volatility position via VIX calls, strike 25, expiring in 30 days.",
        "Fixed income: Shifted to short duration as yield curve inversion deepens.",
    ]

    search_engine.index_documents(documents)

    # Run searches
    queries = [
        "How did the company perform financially?",
        "What are the risks in the portfolio?",
        "Tell me about mergers and acquisitions",
    ]

    for query in queries:
        print(f"\nQuery: '{query}'")
        print("-" * 80)
        results = search_engine.search(query, top_k=3)
        for result in results:
            print(f"  [{result['score']:.4f}] {result['document'][:80]}...")


if __name__ == "__main__":
    main()
