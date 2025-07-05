#include "SymbolManager.h"

SymbolManager::SymbolManager(mdfeed::MarketDataPublisher& md_publisher)
    : md_publisher_(md_publisher)
{
    initialize_symbols();
}

OrderBook<mdfeed::MarketDataPublisher>*
SymbolManager::get_order_book(uint32_t symbol_id)
{
    const auto it = order_books_.find(symbol_id);
    return it != order_books_.end() ? it->second.get() : nullptr;
}

OrderBook<mdfeed::MarketDataPublisher>*
SymbolManager::get_order_book(const std::string& symbol)
{
    const auto it = symbol_to_id_.find(symbol);
    if (it == symbol_to_id_.end()) { return nullptr; }
    return get_order_book(it->second);
}

std::optional<uint32_t>
SymbolManager::get_symbol_id(const std::string& symbol) const
{
    if (const auto it = symbol_to_id_.find(symbol); it != symbol_to_id_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string>
SymbolManager::get_symbol_name(uint32_t symbol_id) const
{
    if (const auto it = id_to_symbol_.find(symbol_id);
        it != id_to_symbol_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool SymbolManager::is_valid_symbol(const std::string& symbol) const
{
    return symbol_to_id_.contains(symbol);
}

bool SymbolManager::is_valid_symbol(uint32_t symbol_id) const
{
    return order_books_.contains(symbol_id);
}

std::vector<std::string> SymbolManager::get_all_symbols() const
{
    std::vector<std::string> symbols;
    symbols.reserve(symbol_to_id_.size());
    for (const auto& [symbol, id]: symbol_to_id_) { symbols.push_back(symbol); }
    return symbols;
}

void SymbolManager::initialize_symbols()
{
    struct SymbolInfo {
        Symbol id;
        std::string ticker;
        std::string name;
    };

    const std::vector<SymbolInfo> symbols
            = {{Symbol::AAPL, "AAPL", "Apple Inc"},
               {Symbol::GOOGL, "GOOGL", "Alphabet Inc"},
               {Symbol::AMZN, "AMZN", "Amazon.com Inc"},
               {Symbol::NFLX, "NFLX", "Netflix Inc"},
               {Symbol::META, "META", "Meta Platforms Inc"}};

    for (const auto& [id, ticker, name]: symbols) {
        auto symbol_id = static_cast<uint32_t>(id);
        securities_[symbol_id] = std::make_unique<Security>(
                std::move(name), std::move(ticker), symbol_id);
        adapters_[symbol_id] = std::make_unique<
                mdfeed::MDAdapter<mdfeed::MarketDataPublisher>>(symbol_id,
                                                                md_publisher_);
        order_books_[symbol_id]
                = std::make_unique<OrderBook<mdfeed::MarketDataPublisher>>(
                        *securities_[symbol_id], *adapters_[symbol_id]);
        symbol_to_id_[ticker] = symbol_id;
        id_to_symbol_[symbol_id] = ticker;
    }
}
