import sys

if __name__ == "__main__":
    if len(sys.argv) > 1:
        ticker = sys.argv[1].upper()
        print(f"scripts/main.py :: Ticker received: {ticker}")
    else:
        print("scripts/main.py :: No ticker argument provided.")