#pragma once

#include "core/OrderBook.h"
#include "publisher/MDAdapter.h"
#include "publisher/MarketDataPublisher.h"
#include "securities/Security.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class SymbolManager {
public:
    enum class Symbol : uint32_t {
        AAPL = 1,
        GOOGL = 2,
        AMZN = 3,
        NFLX = 4,
        META = 5
    };

private:
    std::unordered_map<uint32_t,
                       std::unique_ptr<OrderBook<mdfeed::MarketDataPublisher>>>
            order_books_;
    std::unordered_map<std::string, uint32_t> symbol_to_id_;
    std::unordered_map<uint32_t, std::string> id_to_symbol_;
    std::unordered_map<
            uint32_t,
            std::unique_ptr<mdfeed::MDAdapter<mdfeed::MarketDataPublisher>>>
            adapters_;
    std::unordered_map<uint32_t, std::unique_ptr<Security>> securities_;
    mdfeed::MarketDataPublisher& md_publisher_;

public:
    explicit SymbolManager(mdfeed::MarketDataPublisher& md_publisher);

    OrderBook<mdfeed::MarketDataPublisher>* get_order_book(uint32_t symbol_id);

    OrderBook<mdfeed::MarketDataPublisher>*
    get_order_book(const std::string& symbol);

    [[nodiscard]] std::optional<uint32_t>
    get_symbol_id(const std::string& symbol) const;

    [[nodiscard]] std::optional<std::string>
    get_symbol_name(uint32_t symbol_id) const;

    [[nodiscard]] bool is_valid_symbol(const std::string& symbol) const;
    [[nodiscard]] bool is_valid_symbol(uint32_t symbol_id) const;

    [[nodiscard]] std::vector<std::string> get_all_symbols() const;

private:
    void initialize_symbols();
};
