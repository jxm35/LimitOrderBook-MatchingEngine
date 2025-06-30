import random
import threading
import time
from typing import Optional

from order_book import OrderBook


class MarketSimulator:
    def __init__(self, order_book: OrderBook):
        self.order_book = order_book
        self.running = False
        self.simulation_thread: Optional[threading.Thread] = None

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

    def _apply_buy_pressure(self):
        start_time = time.time()
        while (time.time() - start_time < self.pressure_duration and
               self.running and self.buy_pressure_active):

            quantity = self.quantity_generator()
            self.order_book.place_market_buy_order(quantity)

            best_bid = self.order_book.get_best_bid_price()
            best_ask = self.order_book.get_best_ask_price()

            if best_bid and best_ask:
                mid_price = (best_bid + best_ask) // 2
                aggressive_price = mid_price + random.randint(50, 150)
                self.order_book.add_order(aggressive_price, self.quantity_generator(), True)

            time.sleep(0.1)

        self.buy_pressure_active = False

    def _apply_sell_pressure(self):
        start_time = time.time()
        while (time.time() - start_time < self.pressure_duration and
               self.running and self.sell_pressure_active):

            quantity = self.quantity_generator()
            self.order_book.place_market_sell_order(quantity)

            best_bid = self.order_book.get_best_bid_price()
            best_ask = self.order_book.get_best_ask_price()

            if best_bid and best_ask:
                mid_price = (best_bid + best_ask) // 2
                aggressive_price = mid_price - random.randint(50, 150)
                self.order_book.add_order(aggressive_price, self.quantity_generator(), False)

            time.sleep(0.1)

        self.sell_pressure_active = False

    def _simulation_loop(self):
        while self.running:
            try:
                best_bid = self.order_book.get_best_bid_price()
                best_ask = self.order_book.get_best_ask_price()

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
                    self._cancel_random_order()

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
            self.order_book.add_order(price, quantity, is_buy)

    def _add_market_order(self):
        is_buy = random.random() < 0.5
        quantity = self.quantity_generator() // 2

        if is_buy:
            self.order_book.place_market_buy_order(quantity)
        else:
            self.order_book.place_market_sell_order(quantity)

    def _cancel_random_order(self):
        pass
