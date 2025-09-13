from consts import FXURL1, FXURL2, HEADERS, AVURL2, AVURL5, AVURL6
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
        fromcurrency = sys.argv[1][0:3].lower()
        tocurrency = sys.argv[1][3:].lower()

        print(f"scripts/forex.py :: {fromcurrency}/{tocurrency} query starting...")

        fxurl = FXURL1+fromcurrency+tocurrency+FXURL2
        print(f"scripts/forex.py :: Requesting finance.yahoo.com for {fromcurrency}/{tocurrency}'s HTML content...")

        response = requests.get(fxurl, headers=HEADERS)

        soup = BeautifulSoup(response.text, "html.parser")
        '''
        lilist = soup.findAll("li", class_="yf-1qull9i")

        for li in lilist:
            print(li.text+"\n")
        '''
        price_span = soup.find("span", class_="yf-ipw1h0 base")
        price = price_span.text if price_span else "--"

        dollar_span = soup.find("span", attrs={"data-testid": "qsp-price-change"})
        dollarchange = dollar_span.text if dollar_span else "--"

        percentchange = "--"
        if dollar_span:
            sibling = dollar_span.find_next("span")
            if sibling:
                percentchange = sibling.text

        percentchange = re.sub(r"[()]", "", percentchange)  
        print("Price:", price)
        print("Dollar change:", dollarchange)
        print("Percent change:", percentchange)

        pairs = [
            ["Symbol", f"{fromcurrency}{tocurrency}"], ["Price", price],
            ["Change", percentchange], ["$ Change", dollarchange]
        ]

        # Ensure data directory exists
        os.makedirs("data", exist_ok=True)
        csv_path = os.path.join("data", f"{fromcurrency}{tocurrency}.csv")

        # Write Label/Value pairs
        with open(csv_path, mode="w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["Label", "Value"])
            writer.writerows(pairs)

        print(f"scripts/cryptos.py :: {fromcurrency}{tocurrency} data successfully written to {csv_path}")
        response.close()
