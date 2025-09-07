import sys
import os
import csv
import requests
from bs4 import BeautifulSoup

URLP = "https://finviz.com/quote.ashx?t="
HEADERS = {
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/118.0.0.0 Safari/537.36",
    "Accept-Language": "en-US,en;q=0.9",
    "Accept": "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8",
    "Referer": "https://finviz.com/"
}

if __name__ == "__main__":
    if len(sys.argv) > 1:
        ticker = sys.argv[1].upper()
        print(f"scripts/main.py :: Ticker received: {ticker}")
        url = f"{URLP}{ticker}"
        print("scripts/main.py :: Requesting Finviz.com for HTML content...")
        response = requests.get(url, headers=HEADERS)
        
        soup = BeautifulSoup(response.text, "html.parser")
        table = soup.find("table", class_="js-snapshot-table snapshot-table2 screener_snapshot-table-body")
        
        pairs = []
        if table:
            for row in table.find_all("tr"):
                cells = [cell.get_text(strip=True) for cell in row.find_all("td")]
                # cells come in [label, value, label, value, ...]
                for i in range(0, len(cells), 2):
                    label = cells[i]
                    value = cells[i+1] if i+1 < len(cells) else ""
                    pairs.append([label, value])

        # Ensure data directory exists
        os.makedirs("data", exist_ok=True)
        csv_path = os.path.join("data", f"{ticker}.csv")

        # Write Label/Value pairs
        with open(csv_path, mode="w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["Label", "Value"])  # header
            writer.writerows(pairs)

        print(f"scripts/main.py :: Data written to {csv_path}")
        
    else:
        print("scripts/main.py :: No ticker argument provided.")
