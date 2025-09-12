from consts import FXURL, HEADERS, AVURL2, AVURL5, AVURL6
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

        print(f"scripts/cryptos.py :: {fromcurrency}/{tocurrency} query starting...")

        print(f"scripts/cryptos.py :: Requesting x-rates.com for {fromcurrency}:{tocurrency}'s HTML content...")

        response = requests.get(cmcurl, headers=HEADERS)

        soup = BeautifulSoup(response.text, "html.parser")

        table = soup.find("table", "data-table")

        if not table:
        print("Could not find table with class 'data-table'")
        sys.exit(1)

        rows = table.find_all("tr")
        data = []

        headers = [th.get_text(strip=True) for th in rows[0].find_all("th")]

        for tr in rows[1:]:
        cols = tr.find_all("td")
        if len(cols) != len(headers):
            continue
        row = [td.get_text(strip=True) for td in cols]
        data.append(row)

        df = pd.DataFrame(data, columns=headers)
        print(df)

        df.to_csv(f"data/{fromcurrency}{tocurrency}.csv", index=False)

        print(f"scripts/cryptos.py :: {fromcurrency}/{tocurrency} data successfully written to data/{fromcurrency}{tocurrency}.csv")
