# AI-Powered Trading Strategy System

## Overview
This project combines **Python ML models (TensorFlow/sklearn)** with a **C++20 trading engine**.
The ML model generates trading signals, and the C++ engine executes trades & computes PnL/Sharpe ratio.

## Tech Stack
- Python (Pandas, NumPy, sklearn)
- C++20
- CSV-based integration

## How It Works
1. Python ML model generates buy/sell signals (`ml_model.py`).
2. Signals saved to `signals.csv`.
3. C++ trading engine (`engine.cpp`) simulates order execution and reports performance.

## Run Instructions
```bash
# Step 1: Generate signals
python ml_model.py

# Step 2: Build and run engine
g++ engine.cpp -o engine -std=c++20
./engine
