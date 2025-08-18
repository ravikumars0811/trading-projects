# High-Frequency Order Book Engine

## Overview
A simplified **limit order book** written in C++20, capable of matching buy/sell orders with microsecond latency.

## Features
- FIFO order matching
- Limit & market orders
- Lock-free queue prototype

## Run Instructions
```bash
g++ orderbook.cpp -o orderbook -std=c++20
./orderbook