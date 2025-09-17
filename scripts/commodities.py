from avkey import AVKEY
import sys
import os
import urllib.request
import urllib.parse
import json
import csv
import matplotlib.pyplot as plt
from PIL import Image


def main():
    if len(sys.argv) != 2:
        print("scripts/commodities.py :: Usage: python scripts/commodities.py <url>")
        return

    raw_url = "https://www.alphavantage.co"
    full_url = raw_url+"/query?function="+sys.argv[1].upper()+"&interval=monthly&apikey=" + AVKEY
    
    print("Fetching AlphaVantage API")

    try:
        with urllib.request.urlopen(raw_url) as resp:
            body = resp.read().decode("utf-8")
    except Exception as e:
        print("scripts/commodities.py :: Error fetching URL:", e)
        return

    try:
        parsed = json.loads(body)
    except Exception as e:
        print("scripts/commodities.py :: Error parsing JSON:", e)
        return

    try:
        u = urllib.parse.urlparse(raw_url)
        q = urllib.parse.parse_qs(u.query)
        function = q.get("function", ["unknown"])[0].lower()
    except Exception as e:
        print("scripts/commodities.py :: Error parsing URL:", e)
        function = "unknown"

    csv_path = os.path.join("data", f"{function}.csv")

    dates, values = [], []

    try:
        with open(csv_path, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["Date", "Value"])
            for entry in parsed.get("data", []):
                date = entry.get("date", "")
                value = entry.get("value", "")
                if date and value:
                    writer.writerow([date, value])
                    dates.append(date)
                    try:
                        values.append(float(value))
                    except ValueError:
                        values.append(None)  # Skip invalid numbers
    except Exception as e:
        print("scripts/commodities.py :: Error writing CSV:", e)
        return

    print("scripts/commodities.py :: CSV saved to", csv_path)

    if dates and values:
        try:
            plt.figure(figsize=(10, 5))
            plt.plot(dates, values, color="blue", label=function)
            plt.xlabel("Date")
            plt.ylabel("Value")
            plt.title(f"Time Series Data: {function.upper()}")
            plt.xticks(rotation=45)
            plt.legend()
            plt.tight_layout()

            png_path = os.path.join("img", f"{function}.png")
            bmp_path = os.path.join("img", f"{function}.bmp")
            plt.savefig(png_path, dpi=150)
            plt.close()
            print("scripts/commodities.py :: PNG chart saved to", png_path)

            with Image.open(png_path) as img:
                img = img.resize((400, 300), Image.LANCZOS)
                img.convert("RGB").save(bmp_path, "BMP")
            print("scripts/commodities.py :: BMP image saved to", bmp_path)    

            try:
                os.remove(csv_path)
                print("scripts/commodities.py :: Deleted temporary CSV:", csv_path)
            except Exception as e:
                print("scripts/commodities.py :: Warning - could not delete CSV:", e)

            try:
                os.remove(png_path)
                print("scripts/commodities.py :: Deleted temporary PNG:", png_path)
            except Exception as e:
                print("scripts/commodities.py :: Warning - could not delete PNG:", e)

        except Exception as e:
            print("scripts/commodities.py :: Error creating chart:", e)


if __name__ == "__main__":
    main()
