#pragma once

#include "SymbolManager.h"
#include "messages/OrderMessages.h"
#include "publisher/MarketDataPublisher.h"
#include "publisher/MulticastPublisherThread.h"
#include "server/OrderEntryServer.h"
#include "utils/PublisherConfig.h"
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <sys/socket.h>
#include <thread>
#include <unordered_map>

class Exchange {
public:
    enum class Mode { SIMULATION, CLIENT_ORDERS };

private:
    Mode mode_;
    std::atomic<bool> running_;

    std::unique_ptr<mdfeed::MarketDataPublisher> md_publisher_;
    std::unique_ptr<mdfeed::MulticastPublisherThread> multicast_thread_;
    mdfeed::PublisherConfig md_config_;

    std::unique_ptr<SymbolManager> symbol_manager_;

    std::unique_ptr<orderentry::OrderEntryServer> order_server_;
    uint16_t order_entry_port_;

    std::unordered_map<uint64_t, long> client_to_exchange_id_;
    std::unordered_map<long, std::pair<int, uint64_t>>
            exchange_to_client_id_; // {fd, client_id}

    std::random_device rd_;
    std::mt19937 generator_;
    std::normal_distribution<> price_dist_;
    std::normal_distribution<> quantity_dist_;
    std::bernoulli_distribution bool_dist_;

public:
    Exchange(Mode mode, mdfeed::PublisherConfig md_config,
             uint16_t oe_port = 8080);

    bool start();
    void stop();
    void run();

private:
    void initialize_simulation_books();
    void simulation_loop();
    void simulate_trading_activity(
            OrderBook<mdfeed::MarketDataPublisher>* order_book,
            uint32_t symbol_id, const std::string& symbol);
    void place_simulation_market_order(
            OrderBook<mdfeed::MarketDataPublisher>* order_book,
            const std::string& symbol);
    void add_simulation_limit_order(
            OrderBook<mdfeed::MarketDataPublisher>* order_book,
            uint32_t symbol_id, const std::string& symbol);

    void client_order_loop();
    void process_client_order(const orderentry::OrderBuffer& buffer);
    void handle_new_order(const orderentry::OrderBuffer& buffer);
    void handle_cancel_order(const orderentry::OrderBuffer& buffer);

    static void send_order_ack(int client_fd, uint64_t client_order_id,
                               long exchange_order_id,
                               const std::string& symbol);
    static void send_order_reject(int client_fd, uint64_t client_order_id,
                                  uint32_t reason, const std::string& text);
    static void send_execution_report(int client_fd, uint64_t client_order_id,
                                      long exchange_order_id, uint64_t price,
                                      uint64_t executed_qty,
                                      uint64_t remaining_qty,
                                      orderentry::Side side);
    static void send_response(int client_fd, const void* data, size_t length);

    void print_supported_symbols() const;
    void print_final_stats() const;
};
