class OptionContract:
    def __init__(self, ticker, symbol, strike, yte, price, cp_flag):
        self.underlying = ticker
        self.polygon_symbol = symbol
        self.strike = strike
        self.yte = yte
        self.price = price
        self.callorput = cp_flag

    def __repr__(self):
        return (f"OptionContract("
                f"underlying='{self.underlying}', "
                f"symbol='{self.polygon_symbol}', "
                f"strike={self.strike}, "
                f"type='{self.callorput}', "
                f"price={self.price}, "
                f"yte={self.yte:.4f})")

class OptionExpiry:
    def __init__(self, ticker, date, yte, calls=None, puts=None):
        self.underlying = ticker
        self.date = date
        self.yte = yte
        self.calls = calls if calls is not None else []
        self.puts = puts if puts is not None else []

    def __repr__(self):
        return (
            f"OptionExpiry("
            f"underlying='{self.underlying}', "
            f"date='{self.date}', "
            f"yte={self.yte:.4f}, "
            f"calls={len(self.calls)} contracts, "
            f"puts={len(self.puts)} contracts)"
        )

class OptionChain:
    def __init__(self, ticker, expiries=None):
        self.underlying = ticker
        self.expiries = expiries if expiries is not None else []

    def __repr__(self):
        expiry_dates = [exp.date for exp in self.expiries]
        return (f"OptionChain(underlying='{self.underlying}', "
                f"expiries={len(self.expiries)}, "
                f"dates={expiry_dates})")