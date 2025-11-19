"""
LangChain: Financial Document Q&A System
=========================================
Real-world example: RAG system for querying financial documents

Industry Use Case: Financial analysts and compliance teams use RAG systems to:
- Query thousands of financial reports instantly
- Extract insights from SEC filings, earnings calls, research reports
- Compliance monitoring and regulatory research
- Due diligence for M&A and investments
"""

import os
from typing import List, Dict, Any
from datetime import datetime
import re

# Note: This is a demonstration. In production, install: pip install langchain openai chromadb
# For this demo, we'll create a mock implementation showing the architecture


class MockEmbeddings:
    """Mock embeddings for demonstration (replace with OpenAI or HuggingFace in production)"""

    def embed_documents(self, texts: List[str]) -> List[List[float]]:
        """Generate mock embeddings"""
        import hashlib
        embeddings = []
        for text in texts:
            # Simple hash-based mock embedding
            hash_val = int(hashlib.md5(text.encode()).hexdigest(), 16)
            embedding = [(hash_val >> i) % 1000 / 1000.0 for i in range(384)]
            embeddings.append(embedding)
        return embeddings

    def embed_query(self, text: str) -> List[float]:
        """Generate mock query embedding"""
        return self.embed_documents([text])[0]


class MockVectorStore:
    """Mock vector store for demonstration (replace with ChromaDB, Pinecone, or FAISS)"""

    def __init__(self):
        self.documents = []
        self.embeddings = []
        self.metadata = []

    def add_texts(self, texts: List[str], metadatas: List[Dict] = None, embeddings_model=None):
        """Add documents to vector store"""
        if embeddings_model:
            embeds = embeddings_model.embed_documents(texts)
            self.documents.extend(texts)
            self.embeddings.extend(embeds)
            self.metadata.extend(metadatas or [{} for _ in texts])

    def similarity_search(self, query: str, k: int = 4, embeddings_model=None):
        """Simple similarity search (cosine similarity)"""
        if not self.documents:
            return []

        query_embedding = embeddings_model.embed_query(query)

        # Calculate cosine similarity
        similarities = []
        for i, doc_embedding in enumerate(self.embeddings):
            similarity = sum(a * b for a, b in zip(query_embedding, doc_embedding))
            similarities.append((similarity, i))

        # Get top k
        similarities.sort(reverse=True)
        top_k = similarities[:k]

        results = []
        for _, idx in top_k:
            results.append({
                'content': self.documents[idx],
                'metadata': self.metadata[idx]
            })

        return results


class MockLLM:
    """Mock LLM for demonstration (replace with OpenAI GPT-4, Claude, or local LLM)"""

    def generate(self, prompt: str) -> str:
        """Generate mock response based on prompt"""
        # Extract context and question
        context_match = re.search(r'Context:(.*?)Question:', prompt, re.DOTALL)
        question_match = re.search(r'Question:(.*?)(?:Answer:|$)', prompt, re.DOTALL)

        context = context_match.group(1).strip() if context_match else ""
        question = question_match.group(1).strip() if question_match else ""

        # Simple rule-based responses for demo
        question_lower = question.lower()

        if "revenue" in question_lower or "sales" in question_lower:
            return "Based on the financial documents, the company's revenue for the quarter was $2.5 billion, representing a 15% year-over-year growth. This growth was primarily driven by increased demand in the cloud computing segment."

        elif "risk" in question_lower:
            return "The key risks identified in the documents include: (1) Market volatility and economic uncertainty, (2) Regulatory changes in key markets, (3) Cybersecurity threats, and (4) Competition from emerging technologies. The company has implemented risk mitigation strategies including diversification and enhanced security measures."

        elif "acquisition" in question_lower or "merger" in question_lower:
            return "The company completed a strategic acquisition of TechCorp for $500 million. This acquisition is expected to enhance the company's capabilities in artificial intelligence and expand its market presence in the enterprise sector."

        elif "dividend" in question_lower:
            return "The company declared a quarterly dividend of $0.50 per share, payable to shareholders of record as of the end of the quarter. This represents a 10% increase from the previous quarter, reflecting strong cash flow generation."

        elif "outlook" in question_lower or "guidance" in question_lower:
            return "Management provided positive guidance for the next fiscal year, projecting revenue growth of 12-15% and expanding profit margins. The outlook is supported by strong product pipeline, growing customer base, and operational efficiency improvements."

        else:
            return f"Based on the analyzed documents, regarding '{question}': The financial documents indicate positive performance trends with strategic initiatives focused on growth and operational excellence. Specific metrics and details can be found in the quarterly reports and management discussion sections."


class FinancialDocumentQA:
    """
    Production-grade RAG system for financial document question answering
    Architecture: Document Loading → Chunking → Embedding → Vector Store → Retrieval → LLM
    """

    def __init__(self):
        self.embeddings = MockEmbeddings()
        self.vector_store = MockVectorStore()
        self.llm = MockLLM()
        self.documents_loaded = False

    def generate_sample_documents(self) -> List[Dict[str, Any]]:
        """
        Generate sample financial documents
        In production, load from PDF, HTML, or APIs (SEC EDGAR, company websites)
        """
        documents = [
            {
                'content': """
                QUARTERLY EARNINGS REPORT - Q3 2024
                Company: TechFinance Corp
                Revenue: $2.5 billion (up 15% YoY)
                Net Income: $450 million
                EPS: $3.25 (beating estimates of $3.10)

                Key Highlights:
                - Cloud computing segment grew 25% YoY
                - Enterprise customer base increased by 30%
                - Operating margin improved to 18%
                - Strong cash flow generation of $600 million
                """,
                'metadata': {
                    'source': 'Q3_2024_Earnings.pdf',
                    'type': 'earnings_report',
                    'date': '2024-10-15',
                    'company': 'TechFinance Corp'
                }
            },
            {
                'content': """
                RISK FACTORS - Annual Report 2024

                1. Market and Economic Risks
                - Global economic uncertainty may impact demand
                - Currency exchange rate fluctuations
                - Interest rate changes affecting financing costs

                2. Operational Risks
                - Cybersecurity threats and data breaches
                - Supply chain disruptions
                - Key personnel retention

                3. Regulatory Risks
                - Compliance with evolving data privacy regulations (GDPR, CCPA)
                - Changes in tax laws
                - Industry-specific regulations

                4. Competitive Risks
                - Intense competition from established players and startups
                - Rapid technological changes
                - Pricing pressures
                """,
                'metadata': {
                    'source': 'Annual_Report_2024.pdf',
                    'type': 'annual_report',
                    'date': '2024-12-31',
                    'company': 'TechFinance Corp'
                }
            },
            {
                'content': """
                MERGER AND ACQUISITION ANNOUNCEMENT

                TechFinance Corp Acquires AI Startup TechCorp

                Transaction Details:
                - Purchase price: $500 million in cash and stock
                - Expected to close in Q1 2025
                - Subject to regulatory approval

                Strategic Rationale:
                - Enhances AI and machine learning capabilities
                - Expands product portfolio
                - Access to top-tier AI talent and technology
                - Cross-selling opportunities with existing customer base

                Financial Impact:
                - Expected to be accretive to EPS in first full year
                - Minimal integration risks identified
                - Synergies estimated at $50 million annually
                """,
                'metadata': {
                    'source': 'M&A_Announcement.pdf',
                    'type': 'press_release',
                    'date': '2024-11-01',
                    'company': 'TechFinance Corp'
                }
            },
            {
                'content': """
                DIVIDEND ANNOUNCEMENT

                Board of Directors Approves Quarterly Dividend

                Dividend Details:
                - Amount: $0.50 per share
                - Record Date: December 31, 2024
                - Payment Date: January 15, 2025
                - Increase: 10% from previous quarter

                Management Commentary:
                "This dividend increase reflects our strong financial position,
                robust cash flow generation, and confidence in our business outlook.
                We remain committed to returning value to shareholders while
                maintaining sufficient capital for strategic investments and growth."

                Dividend History:
                - Q1 2024: $0.42
                - Q2 2024: $0.45
                - Q3 2024: $0.47
                - Q4 2024: $0.50 (current)
                """,
                'metadata': {
                    'source': 'Dividend_Announcement.pdf',
                    'type': 'press_release',
                    'date': '2024-12-01',
                    'company': 'TechFinance Corp'
                }
            },
            {
                'content': """
                MANAGEMENT DISCUSSION AND ANALYSIS - Q3 2024

                Business Overview:
                Our company continues to execute on our strategic priorities,
                delivering strong financial results and operational performance.

                Revenue Analysis:
                - Cloud computing: $1.2B (+25% YoY) - Primary growth driver
                - Enterprise software: $800M (+10% YoY) - Stable growth
                - Professional services: $500M (+8% YoY) - Consistent performance

                Outlook for Q4 2024 and FY 2025:
                - Revenue growth: 12-15% expected
                - Margin expansion: Targeting 20% operating margin
                - Investment priorities: AI/ML, cloud infrastructure, sales & marketing
                - Capital allocation: Balance growth investments with shareholder returns

                Key Initiatives:
                1. Product innovation in AI-powered analytics
                2. Geographic expansion in Asia-Pacific
                3. Strategic partnerships with leading technology companies
                4. Talent acquisition and retention programs
                """,
                'metadata': {
                    'source': 'MD&A_Q3_2024.pdf',
                    'type': 'management_discussion',
                    'date': '2024-10-15',
                    'company': 'TechFinance Corp'
                }
            }
        ]

        return documents

    def load_documents(self):
        """Load and process documents into vector store"""
        print("\nLoading financial documents...")

        documents = self.generate_sample_documents()

        # Chunk documents (in production, use recursive character text splitter)
        all_chunks = []
        all_metadata = []

        for doc in documents:
            # Simple chunking by paragraph
            chunks = [p.strip() for p in doc['content'].split('\n\n') if p.strip()]

            for chunk in chunks:
                all_chunks.append(chunk)
                all_metadata.append(doc['metadata'])

        # Add to vector store
        self.vector_store.add_texts(
            texts=all_chunks,
            metadatas=all_metadata,
            embeddings_model=self.embeddings
        )

        self.documents_loaded = True
        print(f"✓ Loaded {len(documents)} documents ({len(all_chunks)} chunks)")

    def retrieve_relevant_context(self, query: str, k: int = 4) -> List[Dict]:
        """Retrieve most relevant document chunks"""
        if not self.documents_loaded:
            raise ValueError("Documents not loaded. Call load_documents() first.")

        results = self.vector_store.similarity_search(
            query=query,
            k=k,
            embeddings_model=self.embeddings
        )

        return results

    def answer_question(self, question: str) -> Dict[str, Any]:
        """
        Answer question using RAG pipeline
        Steps: Question → Retrieve Context → Generate Answer with LLM
        """
        print(f"\nQuestion: {question}")
        print("-" * 70)

        # 1. Retrieve relevant context
        print("Retrieving relevant documents...")
        relevant_docs = self.retrieve_relevant_context(question, k=3)

        # 2. Prepare context for LLM
        context = "\n\n".join([doc['content'] for doc in relevant_docs])

        # 3. Create prompt
        prompt = f"""
You are a financial analyst assistant. Answer the question based on the provided context.

Context:
{context}

Question: {question}

Answer: Provide a clear, concise answer based on the context above. Include specific numbers and facts when available.
"""

        # 4. Generate answer
        print("Generating answer...")
        answer = self.llm.generate(prompt)

        # 5. Prepare response with sources
        sources = [
            {
                'source': doc['metadata'].get('source', 'Unknown'),
                'type': doc['metadata'].get('type', 'Unknown'),
                'date': doc['metadata'].get('date', 'Unknown')
            }
            for doc in relevant_docs
        ]

        return {
            'question': question,
            'answer': answer,
            'sources': sources,
            'timestamp': datetime.now().isoformat()
        }

    def chat(self):
        """Interactive chat interface"""
        print("\n" + "=" * 70)
        print("FINANCIAL DOCUMENT Q&A SYSTEM")
        print("=" * 70)
        print("\nType your questions about the financial documents.")
        print("Type 'quit' to exit.\n")

        while True:
            question = input("\nYou: ").strip()

            if question.lower() in ['quit', 'exit', 'q']:
                print("\nGoodbye!")
                break

            if not question:
                continue

            try:
                result = self.answer_question(question)

                print(f"\nAssistant: {result['answer']}")

                print(f"\nSources:")
                for i, source in enumerate(result['sources'], 1):
                    print(f"  {i}. {source['source']} ({source['type']}, {source['date']})")

            except Exception as e:
                print(f"\nError: {e}")


def main():
    """Demo: Financial document Q&A system"""
    print("=" * 70)
    print("LANGCHAIN: FINANCIAL DOCUMENT Q&A SYSTEM")
    print("=" * 70)

    # Initialize system
    qa_system = FinancialDocumentQA()

    # Load documents
    print("\n1. LOADING FINANCIAL DOCUMENTS...")
    qa_system.load_documents()

    # Sample questions
    print("\n2. DEMONSTRATING Q&A CAPABILITIES...")
    print("=" * 70)

    sample_questions = [
        "What was the revenue in Q3 2024?",
        "What are the main risk factors?",
        "Tell me about recent acquisitions",
        "What is the dividend policy?",
        "What is the company's outlook for next year?"
    ]

    for question in sample_questions:
        result = qa_system.answer_question(question)

        print(f"\nAnswer: {result['answer']}")
        print(f"\nSources used:")
        for i, source in enumerate(result['sources'], 1):
            print(f"  {i}. {source['source']}")
        print("\n" + "-" * 70)

    # Interactive mode (commented out for demo)
    # print("\n3. INTERACTIVE MODE...")
    # qa_system.chat()

    print("\n" + "=" * 70)
    print("Q&A SYSTEM DEMO COMPLETE")
    print("=" * 70)

    print("\nPRODUCTION RECOMMENDATIONS:")
    print("- Use real embeddings (OpenAI, HuggingFace, Cohere)")
    print("- Implement proper vector database (Pinecone, Weaviate, ChromaDB)")
    print("- Use production LLM (GPT-4, Claude, Llama)")
    print("- Add document preprocessing (PDF parsing, OCR)")
    print("- Implement caching for frequently asked questions")
    print("- Add citation and source verification")
    print("- Implement user authentication and access control")
    print("- Add conversation memory for multi-turn dialogue")


if __name__ == "__main__":
    main()
