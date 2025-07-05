#include "Exchange.h"
#include <utility>

Exchange::Exchange(const Mode mode, mdfeed::PublisherConfig md_config,
                   uint16_t oe_port)
    : mode_(mode), running_(false), md_config_(std::move(md_config)),
      order_entry_port_(oe_port), generator_(rd_()), price_dist_(50000, 500),
      quantity_dist_(100, 20), bool_dist_(0.5)
{
    md_publisher_ = std::make_unique<mdfeed::MarketDataPublisher>(md_config_);
    multicast_thread_ = std::make_unique<mdfeed::MulticastPublisherThread>(
            md_publisher_->get_ring_buffer(), md_config_);

    symbol_manager_ = std::make_unique<SymbolManager>(*md_publisher_);

    if (mode_ == Mode::CLIENT_ORDERS) {
        order_server_ = std::make_unique<orderentry::OrderEntryServer>(
                order_entry_port_);
    }
}

bool Exchange::start()
{
    if (running_.load()) return true;
    std::cout << "Starting Multicast Publisher Thread..." << std::endl;
    if (!multicast_thread_->start()) {
        std::cerr << "Failed to start multicast publisher thread" << std::endl;
        return false;
    }
    if (mode_ == Mode::CLIENT_ORDERS) {
        std::cout << "Starting Order Entry Server on port " << order_entry_port_
                  << "..." << std::endl;
        if (!order_server_->start()) {
            std::cerr << "Failed to start order entry server" << std::endl;
            multicast_thread_->stop();
            return false;
        }
    }

    running_.store(true);

    if (mode_ == Mode::SIMULATION) {
        std::cout << "Exchange started in SIMULATION mode" << std::endl;
        initialize_simulation_books();
    }
    else {
        std::cout << "Exchange started in CLIENT_ORDERS mode" << std::endl;
        std::cout
                << "Order books initialized empty, waiting for client orders..."
                << std::endl;
    }
    print_supported_symbols();
    return true;
}

void Exchange::stop()
{
    if (!running_.load()) return;

    std::cout << "Stopping Enhanced Exchange..." << std::endl;
    running_.store(false);
    if (order_server_) { order_server_->stop(); }
    if (multicast_thread_) { multicast_thread_->stop(); }
    print_final_stats();
}

void Exchange::run()
{
    if (!start()) {
        std::cerr << "Failed to start exchange" << std::endl;
        return;
    }
    if (mode_ == Mode::SIMULATION) { simulation_loop(); }
    else {
        client_order_loop();
    }

    stop();
}

void Exchange::initialize_simulation_books()
{
    for (auto symbols = symbol_manager_->get_all_symbols();
         const auto& symbol: symbols) {
        auto* order_book = symbol_manager_->get_order_book(symbol);
        if (!order_book) continue;

        auto symbol_id = symbol_manager_->get_symbol_id(symbol);
        if (!symbol_id) continue;

        long base_price = 50000;
        Order initial_bid(OrderCore("exchange_mm", *symbol_id),
                          base_price - 200, 1000, true);
        Order initial_ask(OrderCore("exchange_mm", *symbol_id),
                          base_price + 200, 1000, false);

        order_book->AddOrder(initial_bid);
        order_book->AddOrder(initial_ask);

        for (int i = 1; i <= 5; ++i) {
            long bid_price = base_price - 200 - (i * 100);
            long ask_price = base_price + 200 + (i * 100);
            auto quantity = static_cast<uint32_t>(
                    std::abs(quantity_dist_(generator_)));

            Order bid(OrderCore("exchange_mm", *symbol_id), bid_price, quantity,
                      true);
            Order ask(OrderCore("exchange_mm", *symbol_id), ask_price, quantity,
                      false);

            order_book->AddOrder(bid);
            order_book->AddOrder(ask);
        }

        std::cout << "Initialized " << symbol
                  << " with spread: " << (base_price - 200) / 100.0 << " - "
                  << (base_price + 200) / 100.0 << std::endl;
    }
}

void Exchange::simulation_loop()
{
    const auto symbols = symbol_manager_->get_all_symbols();

    while (running_.load()) {
        if (symbols.empty()) break;

        const auto& symbol = symbols[generator_() % symbols.size()];
        auto* order_book = symbol_manager_->get_order_book(symbol);
        auto symbol_id = symbol_manager_->get_symbol_id(symbol);
        if (!order_book || !symbol_id) continue;
        simulate_trading_activity(order_book, *symbol_id, symbol);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void Exchange::simulate_trading_activity(
        OrderBook<mdfeed::MarketDataPublisher>* order_book,
        const uint32_t symbol_id, const std::string& symbol)
{
    if (bool_dist_(generator_) && bool_dist_(generator_)) {
        place_simulation_market_order(order_book, symbol);
    }
    else {
        add_simulation_limit_order(order_book, symbol_id, symbol);
    }
}

void Exchange::place_simulation_market_order(
        OrderBook<mdfeed::MarketDataPublisher>* order_book,
        const std::string& symbol)
{
    auto quantity
            = static_cast<uint32_t>(std::abs(quantity_dist_(generator_)) / 2);
    if (quantity == 0) quantity = 1;

    if (bool_dist_(generator_)) {
        std::cout << "[SIM] " << symbol << " market BUY: " << quantity
                  << std::endl;
        order_book->PlaceMarketBuyOrder(quantity);
    }
    else {
        std::cout << "[SIM] " << symbol << " market SELL: " << quantity
                  << std::endl;
        order_book->PlaceMarketSellOrder(quantity);
    }
}

void Exchange::add_simulation_limit_order(
        OrderBook<mdfeed::MarketDataPublisher>* order_book,
        const uint32_t symbol_id, const std::string& symbol)
{
    auto best_bid = order_book->GetBestBidPrice();
    auto best_ask = order_book->GetBestAskPrice();

    if (!best_bid.has_value() || !best_ask.has_value()) return;

    double mid_price = (best_bid.value() + best_ask.value()) / 2.0;
    bool is_buy = bool_dist_(generator_);

    long price;
    if (is_buy) {
        double target_price = (best_bid.value() + mid_price) / 2.0;
        price = static_cast<long>(std::abs(std::normal_distribution<double>(
                target_price, 200)(generator_)));
    }
    else {
        double target_price = (best_ask.value() + mid_price) / 2.0;
        price = static_cast<long>(std::abs(std::normal_distribution<double>(
                target_price, 200)(generator_)));
    }

    auto quantity = static_cast<uint32_t>(std::abs(quantity_dist_(generator_)));
    if (quantity == 0) quantity = 1;

    const Order order(OrderCore("simulator", symbol_id), price, quantity,
                      is_buy);
    order_book->AddOrder(order);

    std::cout << "[SIM] " << symbol << " " << (is_buy ? "BID" : "ASK") << ": "
              << quantity << " @ " << (price / 100.0) << std::endl;
}

void Exchange::client_order_loop()
{
    orderentry::OrderBuffer order_buffer;

    while (running_.load()) {
        if (order_server_->get_order_buffer()->pop(order_buffer)) {
            process_client_order(order_buffer);
        }
        else {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
}

void Exchange::process_client_order(const orderentry::OrderBuffer& buffer)
{
    const auto* header
            = reinterpret_cast<const orderentry::MessageHeader*>(buffer.data);

    try {
        switch (static_cast<orderentry::MessageType>(header->message_type)) {
        case orderentry::MessageType::NEW_ORDER:
            handle_new_order(buffer);
            break;
        case orderentry::MessageType::CANCEL_ORDER:
            handle_cancel_order(buffer);
            break;
        default:
            std::cerr << "Unsupported message type: " << header->message_type
                      << std::endl;
            break;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing client order: " << e.what() << std::endl;
    }
}

void Exchange::handle_new_order(const orderentry::OrderBuffer& buffer)
{
    const auto* msg = buffer.as<orderentry::NewOrderMessage>();
    std::string symbol(msg->symbol, strnlen(msg->symbol, sizeof(msg->symbol)));

    std::cout << "[CLIENT] New order: " << msg->client_order_id << " " << symbol
              << " " << (msg->side == orderentry::Side::BUY ? "BUY" : "SELL")
              << " " << msg->quantity << " @ " << (msg->price / 100.0)
              << std::endl;

    if (!symbol_manager_->is_valid_symbol(symbol)) {
        std::cerr << "Rejecting order: Unknown symbol " << symbol << std::endl;
        send_order_reject(buffer.client_fd, msg->client_order_id, 1,
                          "Unknown symbol");
        return;
    }

    auto* order_book = symbol_manager_->get_order_book(symbol);
    const auto symbol_id = symbol_manager_->get_symbol_id(symbol);

    if (!order_book || !symbol_id) {
        send_order_reject(buffer.client_fd, msg->client_order_id, 2,
                          "Internal error");
        return;
    }

    const std::string username = "client_" + std::to_string(buffer.client_fd);
    const OrderCore core(username, *symbol_id);
    const Order order(core, msg->price, msg->quantity,
                      msg->side == orderentry::Side::BUY);

    client_to_exchange_id_[msg->client_order_id] = order.OrderId();
    exchange_to_client_id_[order.OrderId()]
            = {buffer.client_fd, msg->client_order_id};

    send_order_ack(buffer.client_fd, msg->client_order_id, order.OrderId(),
                   symbol);

    const uint64_t old_matched = order_book->GetOrdersMatched();
    order_book->AddOrder(order);

    if (const uint64_t new_matched = order_book->GetOrdersMatched();
        new_matched > old_matched) {
        const uint64_t executed_qty = new_matched - old_matched;
        auto best_price = msg->side == orderentry::Side::BUY
                                  ? order_book->GetBestAskPrice()
                                  : order_book->GetBestBidPrice();

        if (best_price.has_value()) {
            send_execution_report(buffer.client_fd, msg->client_order_id,
                                  order.OrderId(), best_price.value(),
                                  executed_qty, order.CurrentQuantity(),
                                  msg->side);
        }
    }
}

void Exchange::handle_cancel_order(const orderentry::OrderBuffer& buffer)
{
    const auto* msg = buffer.as<orderentry::CancelOrderMessage>();

    std::cout << "[CLIENT] Cancel order: " << msg->client_order_id
              << " (original: " << msg->original_client_order_id << ")"
              << std::endl;

    const auto it = client_to_exchange_id_.find(msg->original_client_order_id);
    if (it == client_to_exchange_id_.end()) {
        send_order_reject(buffer.client_fd, msg->client_order_id, 3,
                          "Order not found");
        return;
    }

    const long exchange_order_id = it->second;
    const std::string symbol(msg->symbol,
                             strnlen(msg->symbol, sizeof(msg->symbol)));

    auto* order_book = symbol_manager_->get_order_book(symbol);
    if (!order_book) {
        send_order_reject(buffer.client_fd, msg->client_order_id, 4,
                          "Symbol not found");
        return;
    }

    try {
        order_book->RemoveOrder(exchange_order_id);

        client_to_exchange_id_.erase(msg->original_client_order_id);
        exchange_to_client_id_.erase(exchange_order_id);

        send_order_ack(buffer.client_fd, msg->client_order_id,
                       exchange_order_id, symbol);
        std::cout << "Order " << msg->original_client_order_id
                  << " cancelled successfully" << std::endl;
    }
    catch (const std::exception& e) {
        send_order_reject(buffer.client_fd, msg->client_order_id, 5,
                          "Cancel failed");
    }
}

void Exchange::send_order_ack(const int client_fd,
                              const uint64_t client_order_id,
                              const long exchange_order_id,
                              const std::string& symbol)
{
    orderentry::OrderAckMessage ack{};
    orderentry::message_utils::init_header(
            ack, orderentry::MessageType::ORDER_ACK, 0, 0);
    ack.client_order_id = client_order_id;
    ack.exchange_order_id = exchange_order_id;

    const size_t copy_len = std::min(symbol.length(), sizeof(ack.symbol));
    std::memcpy(ack.symbol, symbol.c_str(), copy_len);
    if (copy_len < sizeof(ack.symbol)) {
        std::memset(ack.symbol + copy_len, 0, sizeof(ack.symbol) - copy_len);
    }
    std::memset(ack.reserved, 0, sizeof(ack.reserved));

    send_response(client_fd, &ack, sizeof(ack));
}

void Exchange::send_order_reject(const int client_fd,
                                 const uint64_t client_order_id,
                                 const uint32_t reason, const std::string& text)
{
    orderentry::OrderRejectMessage reject{};
    orderentry::message_utils::init_header(
            reject, orderentry::MessageType::ORDER_REJECT, 0, 0);
    reject.client_order_id = client_order_id;
    reject.reject_reason = reason;

    const size_t copy_len = std::min(text.length(), sizeof(reject.reject_text));
    std::memcpy(reject.reject_text, text.c_str(), copy_len);
    if (copy_len < sizeof(reject.reject_text)) {
        std::memset(reject.reject_text + copy_len, 0,
                    sizeof(reject.reject_text) - copy_len);
    }
    std::memset(reject.symbol, 0, sizeof(reject.symbol));
    std::memset(reject.reserved, 0, sizeof(reject.reserved));

    send_response(client_fd, &reject, sizeof(reject));
}

void Exchange::send_execution_report(const int client_fd,
                                     const uint64_t client_order_id,
                                     const long exchange_order_id,
                                     const uint64_t price,
                                     const uint64_t executed_qty,
                                     const uint64_t remaining_qty,
                                     const orderentry::Side side)
{
    orderentry::ExecutionReportMessage exec{};
    orderentry::message_utils::init_header(
            exec, orderentry::MessageType::EXECUTION_REPORT, 0, 0);
    exec.client_order_id = client_order_id;
    exec.exchange_order_id = exchange_order_id;
    exec.execution_id = exchange_order_id;
    exec.execution_price = price;
    exec.execution_quantity = executed_qty;
    exec.leaves_quantity = remaining_qty;
    exec.side = side;
    std::memset(exec.reserved, 0, sizeof(exec.reserved));

    send_response(client_fd, &exec, sizeof(exec));
}

void Exchange::send_response(const int client_fd, const void* data,
                             const size_t length)
{
    if (ssize_t sent = send(client_fd, data, length, MSG_NOSIGNAL); sent < 0) {
        std::cerr << "Failed to send response to client " << client_fd
                  << std::endl;
    }
}

void Exchange::print_supported_symbols() const
{
    const auto symbols = symbol_manager_->get_all_symbols();
    std::cout << "Supported symbols: ";
    for (size_t i = 0; i < symbols.size(); ++i) {
        std::cout << symbols[i];
        if (i < symbols.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
}

void Exchange::print_final_stats() const
{
    const auto stats = multicast_thread_->get_stats();
    std::cout << "\n=== FINAL EXCHANGE STATISTICS ===" << std::endl;
    std::cout << "Market Data Messages sent: " << stats.messages_sent
              << std::endl;
    std::cout << "Market Data Send failures: " << stats.send_failures
              << std::endl;

    if (order_server_) {
        auto [orders_received, connections] = order_server_->get_stats();
        std::cout << "Order Entry Orders received: " << orders_received
                  << std::endl;
        std::cout << "Order Entry Total connections: " << connections
                  << std::endl;
    }

    for (const auto symbols = symbol_manager_->get_all_symbols();
         const auto& symbol: symbols) {
        if (auto* order_book = symbol_manager_->get_order_book(symbol);
            order_book && order_book->Count() > 0) {
            std::cout << symbol << " - Orders: " << order_book->Count()
                      << ", Matched: " << order_book->GetOrdersMatched()
                      << std::endl;
        }
    }
}
