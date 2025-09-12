from avkey import AVKEY

FVURL = "https://finviz.com/quote.ashx?t="
CMCURL = "https://coinmarketcap.com/currencies/"
FXURL = "https://www.bloomberg.com/markets/currencies"
HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/118.0.0.0 Safari/537.36",
    "Accept-Language": "en-US,en;q=0.9",
    "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
    "Referer": "https://finviz.com/"
}
AVURL1 = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol="
AVURL2 = "&apikey=" + AVKEY
AVURL3 = "https://www.alphavantage.co/query?function=DIGITAL_CURRENCY_DAILY&symbol="
AVURL4 = "&market=USD" + AVURL2
AVURL5 = "https://www.alphavantage.co/query?function=FX_DAILY&from_symbol="
AVURL6 = "&to_symbol="