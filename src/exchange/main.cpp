#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <csignal>
#include <memory>

#include "core/OrderBook.h"
#include "securities/Security.h"
#include "orders/Order.h"
#include "publisher/MarketDataPublisher.h"
#include "publisher/MulticastPublisherThread.h"
#include "utils/PublisherConfig.h"

namespace {
    std::atomic running{true};

    void signal_handler(const int signal) {
        std::cout << "\nReceived signal " << signal << ", shutting down exchange..." << std::endl;
        running.store(false);
    }
}

class ExchangeSimulator {
private:
    static constexpr int SECURITY_ID = 1;
    static constexpr auto USERNAME = "simulator";

    std::unique_ptr<mdfeed::MarketDataPublisher> md_publisher_;
    std::unique_ptr<mdfeed::MulticastPublisherThread> multicast_thread_;
    std::unique_ptr<mdfeed::MDAdapter<mdfeed::MarketDataPublisher>> md_adapter_;
    std::unique_ptr<OrderBook<mdfeed::MarketDataPublisher>> order_book_;

    std::random_device rd_;
    std::mt19937 generator_;
    std::normal_distribution<> price_dist_;
    std::normal_distribution<> quantity_dist_;
    std::bernoulli_distribution bool_dist_;

    long last_bid_price_;
    long last_ask_price_{};

public:
    explicit ExchangeSimulator(const mdfeed::PublisherConfig &config)
            : md_publisher_(std::make_unique<mdfeed::MarketDataPublisher>(config)), multicast_thread_(
            std::make_unique<mdfeed::MulticastPublisherThread>(md_publisher_->get_ring_buffer(), config)), md_adapter_(
            std::make_unique<mdfeed::MDAdapter<mdfeed::MarketDataPublisher>>(SECURITY_ID, *md_publisher_)),
              generator_(rd_()), price_dist_(50000, 500), quantity_dist_(100, 20), bool_dist_(0.5),
              last_bid_price_(49800) {
        Security apple("Apple Inc", "AAPL", SECURITY_ID);
        order_book_ = std::make_unique<OrderBook<mdfeed::MarketDataPublisher>>(apple, *md_adapter_);

        initialize_book();
    }

    void start() {
        std::cout << "Starting Multicast Publisher Thread..." << std::endl;
        if (!multicast_thread_->start()) {
            throw std::runtime_error("Failed to start multicast publisher thread");
        }

        std::cout << "Exchange simulation started. Publishing market data every 500ms..." << std::endl;
        std::cout << "Initial spread: " << last_bid_price_ / 100.0 << " - " << last_ask_price_ / 100.0 << std::endl;
    }

    void stop() {
        std::cout << "Stopping Multicast Publisher Thread..." << std::endl;
        multicast_thread_->stop();

        auto stats = multicast_thread_->get_stats();
        std::cout << "Final Publisher Stats:" << std::endl;
        std::cout << "  Messages sent: " << stats.messages_sent << std::endl;
        std::cout << "  Send failures: " << stats.send_failures << std::endl;
        std::cout << "  Empty polls: " << stats.ring_buffer_empty_count << std::endl;
    }

    void simulate_step() {
        if (bool_dist_(generator_) && bool_dist_(generator_)) {
            place_market_order();
        } else {
            add_limit_order();
        }
        print_book_status();
    }

private:
    void initialize_book() {
        const Order initial_bid(OrderCore(USERNAME, SECURITY_ID), last_bid_price_, 1000, true);
        const Order initial_ask(OrderCore(USERNAME, SECURITY_ID), last_ask_price_, 1000, false);

        order_book_->AddOrder(initial_bid);
        order_book_->AddOrder(initial_ask);

        for (int i = 1; i <= 5; ++i) {
            long bid_price = last_bid_price_ - (i * 100);
            long ask_price = last_ask_price_ + (i * 100);
            const auto quantity = static_cast<uint32_t>(std::abs(quantity_dist_(generator_)));

            Order bid(OrderCore(USERNAME, SECURITY_ID), bid_price, quantity, true);
            Order ask(OrderCore(USERNAME, SECURITY_ID), ask_price, quantity, false);

            order_book_->AddOrder(bid);
            order_book_->AddOrder(ask);
        }
    }

    void add_limit_order() {
        auto best_bid = order_book_->GetBestBidPrice();
        auto best_ask = order_book_->GetBestAskPrice();

        if (best_bid.has_value()) last_bid_price_ = best_bid.value();
        if (best_ask.has_value()) last_ask_price_ = best_ask.value();

        double mid_price = (last_bid_price_ + last_ask_price_) / 2.0;
        bool is_buy = bool_dist_(generator_);

        long price;
        if (is_buy) {
            double target_price = (last_bid_price_ + mid_price) / 2.0;
            price = static_cast<long>(std::abs(std::normal_distribution<double>(target_price, 200)(generator_)));
        } else {
            double target_price = (last_ask_price_ + mid_price) / 2.0;
            price = static_cast<long>(std::abs(std::normal_distribution<double>(target_price, 200)(generator_)));
        }

        auto quantity = static_cast<uint32_t>(std::abs(quantity_dist_(generator_)));
        if (quantity == 0) quantity = 1;

        const Order order(OrderCore(USERNAME, SECURITY_ID), price, quantity, is_buy);
        order_book_->AddOrder(order);

        std::cout << "Added " << (is_buy ? "BID" : "ASK")
                  << " order: " << quantity << " @ " << (price / 100.0) << std::endl;
    }

    void place_market_order() {
        auto quantity = static_cast<uint32_t>(std::abs(quantity_dist_(generator_)) / 2);
        if (quantity == 0) quantity = 1;

        if (bool_dist_(generator_)) {
            std::cout << "Placing market BUY order: " << quantity << std::endl;
            order_book_->PlaceMarketBuyOrder(quantity);
        } else {
            std::cout << "Placing market SELL order: " << quantity << std::endl;
            order_book_->PlaceMarketSellOrder(quantity);
        }
    }

    void print_book_status() const {
        auto best_bid = order_book_->GetBestBidPrice();
        auto best_ask = order_book_->GetBestAskPrice();
        auto spread = order_book_->GetSpread();

        std::cout << "Book Status - Orders: " << order_book_->Count();

        if (best_bid.has_value() && best_ask.has_value()) {
            std::cout << ", Best: " << (best_bid.value() / 100.0)
                      << " x " << (best_ask.value() / 100.0);
            if (spread.Spread().has_value()) {
                std::cout << ", Spread: " << (spread.Spread().value() / 100.0);
            }
        }

        std::cout << ", Matched: " << order_book_->GetOrdersMatched() << std::endl;
    }
};

int main(int argc, char *argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    mdfeed::PublisherConfig config;

    for (int i = 1; i < argc; ++i) {
        if (std::string arg = argv[i]; arg == "--ip" && i + 1 < argc) {
            config.multicast_ip = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            config.multicast_port = static_cast<uint16_t>(std::stoul(argv[++i]));
        } else if (arg == "--interface" && i + 1 < argc) {
            config.interface_ip = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --ip <address>       Multicast IP address (default: " << config.multicast_ip << ")\n"
                      << "  --port <port>        Multicast port (default: " << config.multicast_port << ")\n"
                      << "  --interface <ip>     Local interface IP (default: " << config.interface_ip << ")\n"
                      << "  --help, -h           Show this help message\n";
            return 0;
        }
    }

    std::cout << "=== EXCHANGE SIMULATOR ===" << std::endl;
    std::cout << "Multicast: " << config.multicast_ip << ":" << config.multicast_port << std::endl;
    std::cout << "Interface: " << config.interface_ip << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    try {
        ExchangeSimulator exchange(config);
        exchange.start();

        while (running.load()) {
            exchange.simulate_step();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        exchange.stop();
    }
    catch (const std::exception &e) {
        std::cerr << "Exchange error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Exchange simulator stopped." << std::endl;
    return 0;
}
