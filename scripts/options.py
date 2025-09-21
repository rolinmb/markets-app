from util import *
from consts import TRADINGDAYS, OPTIONSURL1, OPTIONSURL2
import sys
import time
import requests
from datetime import datetime
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

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
        driver.execute_script("arguments[0].scrollIntoView(true);", button)
        driver.execute_script("arguments[0].click();", button)

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

    print(f"scripts/options.py :: All Expirations for {ticker}:\n")
    formatted_expirations = []
    exp_in_years = []
    for e in expirations:
        date_str = e.split("(")[0].strip()
        
        exp_dt = datetime.strptime(date_str, "%b %d, %Y")
        final_dt = exp_dt.strftime("%y%m%d")
        formatted_expirations.append(final_dt)
        
        days_part = e.split("(")[1].split()[0]
        days = int(days_part)
        years = days / TRADINGDAYS
        exp_in_years.append(years)

        if len(formatted_expirations) != len(exp_in_years):
            print(f"scripts/options.py :: len(formatted_expirations) != len(exp_in_years)")
            sys.exit(1)

    blue_cells = driver.find_elements(By.CSS_SELECTOR, "td.tw-bg-blue-50")
    blue_texts = [cell.text.strip() for cell in blue_cells if cell.text.strip()]
    expiries = []
    for i in range(0, len(exp_in_years)):
        calls = []
        puts = []
        for text in blue_texts:
            money_text = str(text).replace(".", "")
            dollar_part = f"00{money_text}00"
            formatted_csymbol = f"O:{ticker}{formatted_expirations[i]}C{dollar_part}"
            formatted_psymbol = f"O:{ticker}{formatted_expirations[i]}P{dollar_part}"
            calls.append(OptionContract(ticker, formatted_csymbol, text, exp_in_years[i], 100.00, True))
            puts.append(OptionContract(ticker, formatted_psymbol, text, exp_in_years[i], 100.00, False))

        expiries.append(OptionExpiry(ticker, expirations[i], exp_in_years[i], calls, puts))

    option_chain = OptionChain(ticker, expiries)
    for e in option_chain.expiries:  # For each expiry
        for i in range(0, len(e.calls)):  # For each strike
            print(f"scripts/options.py :: Expiry {e.date} ::: "
                f"Call {e.calls[i]} | "
                f"Strike {e.calls[i].strike} | "
                f"Put {e.puts[i]}\n")
