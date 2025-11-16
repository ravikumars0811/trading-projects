"""
Setup script for GenAI HFT & Investment Banking
"""

from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

with open("requirements.txt", "r", encoding="utf-8") as fh:
    requirements = [line.strip() for line in fh if line.strip() and not line.startswith("#")]

setup(
    name="genai-hft-investment-bank",
    version="1.0.0",
    author="GenAI Finance Team",
    author_email="info@genai-finance.com",
    description="Generative AI for High-Frequency Trading and Investment Banking",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/your-org/genai-hft-investment-bank",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Financial and Insurance Industry",
        "Topic :: Office/Business :: Financial :: Investment",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
    ],
    python_requires=">=3.9",
    install_requires=requirements,
    extras_require={
        "dev": [
            "pytest>=7.4.0",
            "pytest-cov>=4.1.0",
            "black>=23.7.0",
            "flake8>=6.0.0",
            "mypy>=1.4.0",
        ],
        "gpu": [
            "torch>=2.0.0+cu118",
        ],
        "cloud": [
            "boto3>=1.28.0",
            "google-cloud-storage>=2.10.0",
            "azure-storage-blob>=12.17.0",
        ],
    },
    entry_points={
        "console_scripts": [
            "genai-hft=src.cli:main",
        ],
    },
)
