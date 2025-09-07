import sys
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
        print(f"scripts/main.py :: Raw HTML Response:\n{response}")
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