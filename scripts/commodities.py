from avkey import AVKEY
import sys
import os
import json
import csv
import requests
import matplotlib.pyplot as plt
from PIL import Image

def main():
    if len(sys.argv) != 2:
        print("scripts/commodities.py :: Usage: python scripts/commodities.py <function>")
        return

    function = sys.argv[1]
    base_url = "https://www.alphavantage.co/query"

    print("Fetching AlphaVantage API", base_url)

    params = {
        "function": function,
        "interval": "monthly",
        "apikey": AVKEY
    }

    print("Fetching AlphaVantage API for", function)

    try:
        resp = requests.get(base_url, params=params, timeout=10)
        resp.raise_for_status()
        parsed = resp.json()
    except requests.exceptions.RequestException as e:
        print("scripts/commodities.py :: Error fetching URL:", e)
        return
    except json.JSONDecodeError as e:
        print("scripts/commodities.py :: Error parsing JSON:", e)
        print("Response text (truncated):", resp.text[:500])
        return

    # AlphaVantage commodities return "data" key
    data_entries = parsed.get("data", [])

    if not data_entries:
        print("scripts/commodities.py :: No data returned from API.")
        return

    csv_path = os.path.join("data", f"{function.lower()}.csv")

    dates, values = [], []
    try:
        with open(csv_path, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(["Date", "Value"])
            for entry in data_entries:
                date = entry.get("date", "")
                value = entry.get("value", "")
                if date and value:
                    writer.writerow([date, value])
                    dates.append(date)
                    try:
                        values.append(float(value))
                    except ValueError:
                        values.append(None)
    except Exception as e:
        print("scripts/commodities.py :: Error writing CSV:", e)
        return

    print("scripts/commodities.py :: CSV saved to", csv_path)

    # Plot chart if data exists
    if dates and values:
        try:
            plt.figure(figsize=(10, 5))
            plt.plot(dates, values, color="blue", label=function)
            plt.xlabel("Date")
            plt.ylabel("Value")
            plt.title(f"Time Series Data: {function}")
            plt.xticks(rotation=45)
            plt.legend()
            plt.tight_layout()

            png_path = os.path.join("img", f"{function.lower()}.png")
            bmp_path = os.path.join("img", f"{function.lower()}.bmp")

            plt.savefig(png_path, dpi=150)
            plt.close()
            print("scripts/commodities.py :: PNG chart saved to", png_path)

            with Image.open(png_path) as img:
                img = img.resize((400, 300), Image.LANCZOS)
                img.convert("RGB").save(bmp_path, "BMP")
            print("scripts/commodities.py :: BMP image saved to", bmp_path)

            # Clean up
            try:
                os.remove(csv_path)
                print("scripts/commodities.py :: Deleted temporary CSV:", csv_path)
            except:
                pass
            try:
                os.remove(png_path)
                print("scripts/commodities.py :: Deleted temporary PNG:", png_path)
            except:
                pass

        except Exception as e:
            print("scripts/commodities.py :: Error creating chart:", e)


if __name__ == "__main__":
    # Make sure stdout is UTF-8 (Windows console fix)
    sys.stdout.reconfigure(encoding="utf-8")
    main()
