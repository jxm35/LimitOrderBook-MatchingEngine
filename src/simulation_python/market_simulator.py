import random
import threading
import time
from typing import Optional, List
from dataclasses import dataclass

import orderbook


@dataclass
class Trade:
    price: int
    quantity: int
    timestamp: float


class MarketSimulator:
    def __init__(self, cpp_orderbook):
        self.orderbook = cpp_orderbook
        self.simulation_thread: Optional[threading.Thread] = None
        self.generator_ = None
        self.running = False
        self.last_ask = None
        self.last_bid = None

        self.trades: List[Trade] = []
        self.base_username = "simulator"

    def start_simulation(self):
        if self.running:
            return
        self.running = True
        self.simulation_thread = threading.Thread(target=self._simulation_loop, daemon=True)
        self.simulation_thread.start()

    def stop_simulation(self):
        self.running = False
        if self.simulation_thread:
            self.simulation_thread.join()

    def add_buy_pressure(self):
        pressure_thread = threading.Thread(target=self._apply_buy_pressure, daemon=True)
        pressure_thread.start()

    def add_sell_pressure(self):
        pressure_thread = threading.Thread(target=self._apply_sell_pressure, daemon=True)
        pressure_thread.start()

    def _track_trade(self, price: int, quantity: int):
        self.trades.append(Trade(price, quantity, time.time()))

    def _apply_sell_pressure(self):
        SECURITY_ID = 1
        USERNAME = "test"
        quantity_dist = lambda: self.generator_.normalvariate(100, 10)
        place_limits = lambda: self.generator_.random() < 0.3

        for i in range(500):
            time.sleep(0.002) # sleep 2ms

            self.orderbook.place_market_sell_order(int(quantity_dist()))
            best_bid = self.orderbook.get_best_bid_price()
            if best_bid is None:
                best_bid = self.last_bid

            best_ask = self.orderbook.get_best_ask_price()
            if best_ask is None:
                best_ask = self.last_ask

            if place_limits() and (best_ask - best_bid) > 6:
                mid_price = (best_ask + best_bid) / 2.0
                bid_price_dist = lambda: self.generator_.normalvariate(best_bid, 3)
                sell_price_dist = lambda: self.generator_.normalvariate(mid_price, 3)

                sell_price = sell_price_dist()
                buy_price = bid_price_dist()

                if sell_price < best_bid:
                    sell_price = mid_price
                if buy_price > sell_price:
                    buy_price = best_bid

                ask = orderbook.create_order(USERNAME, SECURITY_ID, round(sell_price), round(quantity_dist()), False)
                self.orderbook.add_order(ask)
                self.last_ask = round(sell_price)

                bid = orderbook.create_order(USERNAME, SECURITY_ID, round(buy_price), round(quantity_dist()), True)
                self.orderbook.add_order(bid)
                self.last_bid = round(buy_price)

    def _apply_buy_pressure(self):
        quantityDist = lambda: self.generator_.normalvariate(100, 10)
        for i in range(500):
            time.sleep(0.002) # sleep 2ms
            self.orderbook.place_market_buy_order(int(quantityDist()))


    def run_simulation(self):
        SECURITY_ID = 1
        USERNAME = "simulation"

        bool_dist = lambda: self.generator_.random() < 0.5
        quantity_dist = lambda: self.generator_.normalvariate(100, 10)

        first_bid = orderbook.create_order(USERNAME, SECURITY_ID, 497_00, 2500, True)
        first_ask = orderbook.create_order(USERNAME, SECURITY_ID, 503_00, 2500, False)
        self.orderbook.add_order(first_bid)
        self.orderbook.add_order(first_ask)

        self.last_ask = 503_00
        self.last_bid = 497_00
        self.generator_ = random.Random()

        while self.running:
            time.sleep(0.01) # sleep 10ms

            best_bid = self.orderbook.get_best_bid_price()
            if best_bid is None:
                best_bid = self.last_bid

            best_ask = self.orderbook.get_best_ask_price()
            if best_ask is None:
                best_ask = self.last_ask

            spread = max(1, best_ask - best_bid)
            MIN_DEVIANCE = 1 + random.randint(0, spread - 1)

            midPrice = (best_ask + best_bid) / 2.0

            bid_mean = (best_bid + midPrice) / 2
            ask_mean = (best_ask + midPrice) / 2
            bid_price_dist = lambda: self.generator_.normalvariate(bid_mean, 5)
            sell_price_dist = lambda: self.generator_.normalvariate(ask_mean, 5)

            price_double = bid_price_dist()
            price = round(price_double)
            for j in range(2):
                old_matched = self.orderbook.get_orders_matched()
                if abs(midPrice - price_double) < MIN_DEVIANCE and abs(price_double - midPrice) < MIN_DEVIANCE:
                    isBuy = bool_dist()
                    qty_to_place = int(quantity_dist() / 2)
                    if isBuy:
                        self.orderbook.place_market_buy_order(qty_to_place)
                    else:
                        self.orderbook.place_market_sell_order(qty_to_place)
                elif price > best_ask:
                    ask = orderbook.create_order(USERNAME, SECURITY_ID, price, int(quantity_dist()), False)
                    self.orderbook.add_order(ask)
                    self.last_ask = price + 3_00
                elif price < best_bid:
                    bid = orderbook.create_order(USERNAME, SECURITY_ID, price, int(quantity_dist()), True)
                    self.orderbook.add_order(bid)
                    self.last_bid = price - 3_00
                else:
                    if price < midPrice:
                        bid = orderbook.create_order(USERNAME, SECURITY_ID, price, int(quantity_dist()), True)
                        self.orderbook.add_order(bid)
                        self.last_bid = price - 3_00
                    elif price > midPrice:
                        ask = orderbook.create_order(USERNAME, SECURITY_ID, price, int(quantity_dist()), False)
                        self.orderbook.add_order(ask)
                        self.last_ask = price + 3_00
                new_matched = self.orderbook.get_orders_matched()
                if new_matched > old_matched:
                    self._track_trade(price, new_matched - old_matched)

                price_double = sell_price_dist()
                price = round(price_double)

    def _simulation_loop(self):
        self.run_simulation()

    def get_recent_volume(self, seconds: float = 1.0) -> int:
        current_time = time.time()
        recent_trades = [
            trade for trade in self.trades
            if current_time - trade.timestamp <= seconds
        ]
        return sum(trade.quantity for trade in recent_trades)