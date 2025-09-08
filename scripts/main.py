from avkey import AVKEY
import os
import re
import csv
import sys
import requests
from PIL import Image
from bs4 import BeautifulSoup
import pandas as pd
import matplotlib.pyplot as plt

FVURL = "https://finviz.com/quote.ashx?t="
HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/118.0.0.0 Safari/537.36",
    "Accept-Language": "en-US,en;q=0.9",
    "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
    "Referer": "https://finviz.com/"
}
AVURL1 = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol="
AVURL2 = "&apikey="+AVKEY

if __name__ == "__main__":
    if len(sys.argv) == 2:
        ticker = sys.argv[1].upper()
        print(f"scripts/main.py :: Ticker {ticker} query starting...")

        fvurl = f"{FVURL}{ticker}"
        print(f"scripts/main.py :: Requesting Finviz.com for {ticker}'s HTML content...")
        
        # Use requests and BeautifulSoup to get the tabular data
        response = requests.get(fvurl, headers=HEADERS)
        
        soup = BeautifulSoup(response.text, "html.parser")
        
        pairs = []

        table = soup.find("table", class_="js-snapshot-table snapshot-table2 screener_snapshot-table-body")

        if table:
            for row in table.find_all("tr"):
                cells = [cell.get_text(strip=True) for cell in row.find_all("td")]
                # cells come in [label, value, label, value, ...]
                for i in range(0, len(cells), 2):
                    label = cells[i]
                    value = cells[i+1] if i+1 < len(cells) else ""
                    pairs.append([label, value])

        spans = soup.find_all("span", class_=[
            "table-row", "w-full", "items-baseline",
            "justify-end", "whitespace-nowrap",
            "text-negative", "text-muted-2"
        ])

        if not spans:
            print(f"scripts/main.py :: Ticker {ticker} is invalid.\n")
            sys.exit()

        dollar_change = spans[0].get_text(strip=True)
        dollar_change_clean = re.search(r"[-+]?[\d.,]+", dollar_change) # Keep only numbers, sign, decimal, and percent
        dollar_change = dollar_change_clean.group(0) if dollar_change_clean else dollar_change
        pairs.append(["Dollar Change", dollar_change])

        # Ensure data directory exists
        os.makedirs("data", exist_ok=True)
        csv_path = os.path.join("data", f"{ticker}.csv")

        # Write Label/Value pairs
        with open(csv_path, mode="w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["Label", "Value"])
            writer.writerows(pairs)

        print(f"scripts/main.py :: {ticker} data successfully written to {csv_path}")
        response.close()

        # TODO: Use alphavantage and matplotlib to get ohlc data to create chart for windows app
        avurl = f"{AVURL1}{ticker}{AVURL2}"

        response = requests.get(avurl)
        data = response.json()
        time_series = data.get("Time Series (Daily)", {})

        if not time_series:
            print("scripts/main.py :: No time series data found in AlphaVantage response.\n")
            sys.exit(1)

        df = pd.DataFrame.from_dict(time_series, orient="index", dtype=float)

        # Columns are "1. open", "2. high", "3. low", "4. close", "5. volume"
        df.index = pd.to_datetime(df.index)  # Convert index to datetime
        df.sort_index(inplace=True)  # Ensure ascending by date

        # Plot Close price
        plt.figure(figsize=(10, 6))
        plt.plot(df.index, df["4. close"], label="Close Price")
        plt.title(f"{ticker} Daily Close Prices")
        plt.xlabel("Date")
        plt.ylabel("Price ($)")
        plt.legend()
        plt.grid(True)

        # Ensure img/ directory exists
        os.makedirs("img", exist_ok=True)
        chart_path = os.path.join("img", f"{ticker}_close.png")
        plt.savefig(chart_path, dpi=150, bbox_inches="tight")
        plt.close()

        # Convert PNG to BMP
        bmp_path = os.path.join("img", f"{ticker}_close.bmp")
        with Image.open(chart_path) as png:
            png = png.convert("RGB")
            png.save(bmp_path, format="BMP")

        print(f"scripts/main.py :: Saved OHLC close price chart to {bmp_path}\n")
    else:
        print("scripts/main.py :: No ticker argument provided.\n")
