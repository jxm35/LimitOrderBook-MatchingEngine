# LimitOrderBook & MatchingEngine
- Limit Orderbook &amp; Matching Engine   + market simulation &amp; visualsation written in c++.
- Design Document [here](https://github.com/jxm35/LimitOrderBook-MatchingEngine/wiki/Design-Document).

## Preview


## Benchmarks

Benchmark methodoly explained [here](https://github.com/jxm35/LimitOrderBook-MatchingEngine/wiki/Design-Document#213-benchmarking).
In a [simulation of market trading](https://github.com/jxm35/LimitOrderBook-MatchingEngine/wiki/Design-Document#23-market-simulation) the application was able to place and match orders at an average rate of <strong>2.4 Million orders per second</strong>.

| Function | Time (ns) | Implied rate per second |
| --- | --- | --- |
| Get Order | 21.2 | 47,139,000 |
| Get Best Bid Price | 20.4 | N/A |
| Place Limit Order | 343 | 2,915,000 |
| Place Limit Order (New Price Level) | 430 | 2,327,000 |
| Place Market Order | 25.7 | 38,899,000 |
| Remove Order | 88.2 | 11,340,000 |
| Match Orders (uncrossed) | 24.0 | 41,624,000 |
| Match Orders (crossed) | 140 | 7,151,000 |


## Features

- A limit orderbook capable of handling limit and market orders, order cancellattions, and order modifications.
- Matching Engine implementing a price-time priority algorithm.
- A market simulation to to test performance and different implementations of the limit order book.
- Visualisation showing a price/time graph, limit level heatmaps, and a volume/time graph.
