import sys
import requests
from bs4 import BeautifulSoup

URLP = "https://finviz.com/quote.ashx?t="

if __name__ == "__main__":
    if len(sys.argv) > 1:
        ticker = sys.argv[1].upper()
        print(f"scripts/main.py :: Ticker received: {ticker}")
        url = f"{URLP}{ticker}"
        print("scripts/main.py :: Requesting Finviz.com for HTML content...")
        response = requests.get(url)
        soup = BeautifulSoup(response.text, "html.parser")
        table = soup.find("table", class_="js-snapshot-table snapshot-table2 screener_snapshot-table-body")
        data = []
        if table:
            for row in table.find_all("tr"):
                cells = [cell.get_text(strip=True) for cell in row.find_all("td")]
                data.append(cells)
        for r in data:
            print(r)
    else:
        print("scripts/main.py :: No ticker argument provided.")