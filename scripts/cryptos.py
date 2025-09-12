from consts import CMCURL, HEADERS, AVURL3, AVURL4
import os
import re
import csv
import sys
import requests
from PIL import Image
from bs4 import BeautifulSoup
import pandas as pd
import matplotlib.pyplot as plt

if __name__ == "__main__":
    if len(sys.argv) == 2:
        crypto = sys.argv[1].lower()
        print(f"scripts/cryptos.py :: Cryptocurrency {crypto} query starting...")

        cmcurl = f"{CMCURL}{crypto}"
        print(f"scripts/cryptos.py :: Requesting Coinmarketcap.com for {crypto}'s HTML content...")
        
        # Use requests and BeautifulSoup to get the tabular data
        response = requests.get(cmcurl, headers=HEADERS)
        
        soup = BeautifulSoup(response.text, "html.parser")
        
        pairs = []

        data_divs = soup.findAll("div", class_="CoinMetrics_overflow-content__tlFu7")
        marketcap = ""
        try:
            marketcap = data_divs[0].text
        except:
            print("scripts/cryptos.py :: Invalid cryptocurrency name; must type the full asset name. (bitcoin, ethereum, dogecoin...)")
            sys.exit(1)

        volume = data_divs[1].text
        fdv = data_divs[2].text
        volumecap = data_divs[3].text
        totalsupply = data_divs[4].text
        maxiumumsupply = data_divs[5].text
        circulatingsupply = data_divs[6].text

        price = soup.find("span", class_="sc-65e7f566-0 esyGGG base-text") # bitcoin

        if not price:
            price = soup.find("span", class_="sc-65e7f566-0 WXGwg base-text").text # ethereum/others
        else:
            price = price.text

        percent_span = soup.find("p", class_="sc-71024e3e-0 sc-9e7b7322-1 bgxfSG dXVXKV change-text")

        raw_percent = percent_span.text.strip()

        percentchange = ""

        match = re.search(r"[-+]?\d+(\.\d+)?%", raw_percent)
        if match:
            percentchange = match.group()

        priceval = float(price.replace("$", "").replace(",", ""))
        percentval = float(percentchange.replace("%", ""))
        sign = "-" if raw_percent.strip().startswith("-") else "+"

        dollarchange_val = priceval * (percentval / 100.0)
        dollarchange = f"{sign}${abs(dollarchange_val):,.2f}"

        crypto_symbol = soup.find("span", "sc-65e7f566-0 czZVlm base-text").text

        pairs = [
            ["Market Cap.", marketcap], ["Volume (24hrs)", volume], ["FDV", fdv], ["Volume/Market Cap. (24hr)", volumecap],
            ["Total Supply", totalsupply], ["Max. Supply", maxiumumsupply], ["Circulating Supply", circulatingsupply],
            ["Price", price], ["% Change (24hrs)", percentchange], ["Dollar Change (24hrs)", dollarchange], ["Symbol", crypto_symbol]
        ]

        # Ensure data directory exists
        os.makedirs("data", exist_ok=True)
        csv_path = os.path.join("data", f"{crypto}.csv")

        # Write Label/Value pairs
        with open(csv_path, mode="w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["Label", "Value"])
            writer.writerows(pairs)

        print(f"scripts/cryptos.py :: {crypto} data successfully written to {csv_path}")
        response.close()

        avurl = f"{AVURL3}{crypto}{AVURL4}"

        response = requests.get(avurl)
        data = response.json()
        time_series = data.get("Time Series (Daily)", {})

        if not time_series:
            print(f"scripts/cryptos.py :: No {crypto} time series data found in AlphaVantage response.\n")
            sys.exit()

        print(f"scripts/cryptos.py :: Successfully fetched time series data for {crypto}.")
        df = pd.DataFrame.from_dict(time_series, orient="index", dtype=float)

        # Columns are "1. open", "2. high", "3. low", "4. close", "5. volume"
        df.index = pd.to_datetime(df.index)  # Convert index to datetime
        df.sort_index(inplace=True)  # Ensure ascending by date

        # Plot Close price
        plt.figure(figsize=(10, 6))
        plt.plot(df.index, df["4. close"], label="Close Price")
        plt.title(f"{crypto} Daily Close Prices")
        plt.xlabel("Date")
        plt.ylabel("Price ($)")
        plt.legend()
        plt.grid(True)

        # Ensure img/ directory exists
        os.makedirs("img", exist_ok=True)
        chart_path = os.path.join("img", f"{crypto}_close.png")
        plt.savefig(chart_path, dpi=150, bbox_inches="tight")
        plt.close()

        # Convert PNG to BMP
        bmp_path = os.path.join("img", f"{crypto}_close.bmp")
        with Image.open(chart_path) as png:
            png = png.convert("RGB")
            png.save(bmp_path, format="BMP")

        if os.path.exists(chart_path):
            os.remove(chart_path)

        print(f"scripts/cryptos.py :: Saved {crypto} close price chart to {bmp_path}\n")
    else:
        print("scripts/cryptos.py :: No cryptocurrency argument provided.\n")
