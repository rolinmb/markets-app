from consts import OPTIONSURL1, OPTIONSURL2
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import sys

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("scripts/options.py :: Usage: python scripts/options.py <TICKER>")
        sys.exit(1)

    ticker = sys.argv[1].upper()

    # Setup headless Chrome
    chrome_options = Options()
    chrome_options.add_argument("--headless")
    driver = webdriver.Chrome(options=chrome_options)

    url = f"{OPTIONSURL1}{ticker}{OPTIONSURL2}"
    driver.get(url)

    # Wait for the page to load
    driver.implicitly_wait(5)

    # Click the expiration button
    try:
        button = driver.find_element(By.ID, "expiration-dates-form-button-1")
        button.click()

        # Wait until at least one label of the desired class is visible
        wait = WebDriverWait(driver, 10)
        wait.until(
            EC.visibility_of_element_located(
                (By.CSS_SELECTOR, "label.tw-ml-3.tw-min-w-0.tw-flex-1.tw-text-gray-600")
            )
        )
    except Exception as e:
        print(f"scripts/options.py :: Error clicking expiration button or waiting for labels: {e}")
        driver.quit()
        sys.exit(1)

    # Find labels by CSS class
    labels = driver.find_elements(
        By.CSS_SELECTOR, "label.tw-ml-3.tw-min-w-0.tw-flex-1.tw-text-gray-600"
    )
    expirations = [lbl.text for lbl in labels if lbl.text.strip()]

    print(f"Expirations for {ticker}:")
    for e in expirations:
        print(e)

    driver.quit()
    """
    ticker = sys.argv[1].upper()
    url = f"{POLYGONURL1}{ticker}{POLYGONURL2}"
        
    response = requests.get(url)
    data = response.json()
        
    if data.get("status") != "OK":
        print("scripts/options.py :: Error fetching data:", data)
        sys.exit(1)

    # Polygon can return either a single contract or a list depending on endpoint
    contracts = data.get("results", [])
        
    # ensure we always have a list
    if isinstance(contracts, dict):
        contracts = [contracts]

    # filter only calls
    calls = [c for c in contracts if c.get("contract_type") == "call"]

    print(f"scripts/options.py :: Found {len(calls)} calls for {ticker}:\n")
    for c in calls:
        print(f"{c['ticker']} | Strike: {c['strike_price']} | Exp: {c['expiration_date']}")
     """