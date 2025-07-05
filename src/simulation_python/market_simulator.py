import random
import threading
import time
from typing import Optional, Dict, List
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
        self.running = False
        self.simulation_thread: Optional[threading.Thread] = None

        self.trades: List[Trade] = []
        self.base_username = "simulator"
        self.min_deviance = 100

        self.quantity_generator = lambda: max(50, int(random.normalvariate(100, 20)))
        self.price_volatility = 5

        self.buy_pressure_active = False
        self.sell_pressure_active = False
        self.pressure_duration = 5.0
        self.pressure_start_time = 0

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
        self.buy_pressure_active = True
        self.pressure_start_time = time.time()

        pressure_thread = threading.Thread(target=self._apply_buy_pressure, daemon=True)
        pressure_thread.start()

    def add_sell_pressure(self):
        self.sell_pressure_active = True
        self.pressure_start_time = time.time()

        pressure_thread = threading.Thread(target=self._apply_sell_pressure, daemon=True)
        pressure_thread.start()

    def _track_trade(self, price: int, quantity: int):
        self.trades.append(Trade(price, quantity, time.time()))

    def _apply_buy_pressure(self):
        start_time = time.time()
        while (time.time() - start_time < self.pressure_duration and
               self.running and self.buy_pressure_active):

            quantity = self.quantity_generator()
            old_matched = self.orderbook.get_orders_matched()
            best_ask = self.orderbook.get_best_ask_price()
            self.orderbook.place_market_buy_order(quantity)
            new_matched = self.orderbook.get_orders_matched()
            if new_matched > old_matched and best_ask:
                self._track_trade(best_ask, new_matched - old_matched)

            best_bid = self.orderbook.get_best_bid_price()
            best_ask = self.orderbook.get_best_ask_price()

            if best_bid and best_ask:
                mid_price = (best_bid + best_ask) // 2
                aggressive_price = mid_price + random.randint(50, 150)

                order = orderbook.create_order("pressure_buyer", 1, aggressive_price,
                                               self.quantity_generator(), True)
                self.orderbook.add_order(order)

            time.sleep(0.1)

        self.buy_pressure_active = False

    def _apply_sell_pressure(self):
        start_time = time.time()
        while (time.time() - start_time < self.pressure_duration and
               self.running and self.sell_pressure_active):

            quantity = self.quantity_generator()

            old_matched = self.orderbook.get_orders_matched()
            best_bid = self.orderbook.get_best_bid_price()

            self.orderbook.place_market_sell_order(quantity)

            new_matched = self.orderbook.get_orders_matched()
            if new_matched > old_matched and best_bid:
                self._track_trade(best_bid, new_matched - old_matched)

            best_bid = self.orderbook.get_best_bid_price()
            best_ask = self.orderbook.get_best_ask_price()

            if best_bid and best_ask:
                mid_price = (best_bid + best_ask) // 2
                aggressive_price = mid_price - random.randint(50, 150)

                order = orderbook.create_order("pressure_seller", 1, aggressive_price,
                                               self.quantity_generator(), False)
                self.orderbook.add_order(order)

            time.sleep(0.1)

        self.sell_pressure_active = False

    def _simulation_loop(self):
        while self.running:
            try:
                best_bid = self.orderbook.get_best_bid_price()
                best_ask = self.orderbook.get_best_ask_price()

                if best_bid is None or best_ask is None:
                    time.sleep(0.1)
                    continue

                mid_price = (best_bid + best_ask) / 2.0
                spread = best_ask - best_bid

                activity_type = random.choices(
                    ['limit_orders', 'market_order', 'cancel', 'nothing'],
                    weights=[60, 15, 10, 15]
                )[0]

                if activity_type == 'limit_orders':
                    self._add_limit_orders(best_bid, best_ask, mid_price, spread)
                elif activity_type == 'market_order':
                    self._add_market_order()
                elif activity_type == 'cancel':
                    pass

                time.sleep(0.01)

            except Exception as e:
                print(f"Simulation error: {e}")
                time.sleep(0.1)

    def _add_limit_orders(self, best_bid: int, best_ask: int, mid_price: float, spread: int):
        num_orders = random.randint(1, 2)

        for _ in range(num_orders):
            if random.random() < 0.3:
                if random.random() < 0.5:
                    price_range = max(100, spread // 2)
                    price = best_bid + random.randint(-price_range, 0)
                    is_buy = True
                else:
                    price_range = max(100, spread // 2)
                    price = best_ask + random.randint(0, price_range)
                    is_buy = False
            else:
                price_deviation = int(random.normalvariate(0, self.price_volatility * 100))
                price = int(mid_price + price_deviation)
                is_buy = price < mid_price

            if abs(price - mid_price) < self.min_deviance:
                continue

            quantity = self.quantity_generator()
            order = orderbook.create_order("sim_trader", 1, price, quantity, is_buy)
            old_matched = self.orderbook.get_orders_matched()
            self.orderbook.add_order(order)
            new_matched = self.orderbook.get_orders_matched()

            if new_matched > old_matched:
                trade_qty = new_matched - old_matched
                trade_price = best_ask if is_buy else best_bid
                self._track_trade(trade_price, trade_qty)

    def _add_market_order(self):
        is_buy = random.random() < 0.5
        quantity = self.quantity_generator() // 2

        old_matched = self.orderbook.get_orders_matched()
        if is_buy:
            best_ask = self.orderbook.get_best_ask_price()
            self.orderbook.place_market_buy_order(quantity)
            trade_price = best_ask
        else:
            best_bid = self.orderbook.get_best_bid_price()
            self.orderbook.place_market_sell_order(quantity)
            trade_price = best_bid

        new_matched = self.orderbook.get_orders_matched()
        if new_matched > old_matched and trade_price:
            self._track_trade(trade_price, new_matched - old_matched)

    def get_recent_volume(self, seconds: float = 1.0) -> int:
        current_time = time.time()
        recent_trades = [
            trade for trade in self.trades
            if current_time - trade.timestamp <= seconds
        ]
        return sum(trade.quantity for trade in recent_trades)