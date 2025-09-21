from util import *
from consts import TRADINGDAYS, OPTIONSURL1, OPTIONSURL2, OPTIONSURL3, OPTIONSURL4
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

    driver.implicitly_wait(5)

    try:
        button = driver.find_element(By.ID, "expiration-dates-form-button-1")
        driver.execute_script("arguments[0].scrollIntoView(true);", button)
        driver.execute_script("arguments[0].click();", button)

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

    labels = driver.find_elements(
        By.CSS_SELECTOR, "label.tw-ml-3.tw-min-w-0.tw-flex-1.tw-text-gray-600"
    )
    expirations_text = [lbl.text for lbl in labels if lbl.text.strip()]

    formatted_expiration_dates = []
    exp_in_years = []
    for text in expirations_text:
        date_str = text.split("(")[0].strip()
        exp_dt = datetime.strptime(date_str, "%b %d, %Y")
        final_dt = exp_dt.strftime("%Y-%m-%d")
        formatted_expiration_dates.append(final_dt)

        days_part = text.split("(")[1].split()[0]
        days = int(days_part)
        years = days / TRADINGDAYS
        exp_in_years.append(years)
        if len(formatted_expiration_dates) != len(exp_in_years):
            print(f"scripts/options.py :: len(formatted_expiration_dates) != len(exp_in_years)")
            sys.exit(1)
    time.sleep(1.0)
    expiries = []
    for i in range(0, len(exp_in_years)):
        url = f"{OPTIONSURL3}{formatted_expiration_dates[i]}{OPTIONSURL4}"
        driver.get(url)
        calls = []
        puts = []
        table = driver.find_element(By.CSS_SELECTOR, "table.table.table-sm.table-hover")
        rows = table.find_elements(By.TAG_NAME, "tr")[1:]
        for row in rows:
            cbid = cask = cvol = c_oi = strike = 0
            pbid = pask = pvol = p_oi = 0
            cols = [c.text.strip() for c in row.find_elements(By.TAG_NAME, "td")]
            if len(cols) != 11 or cols[6] == "-":
                continue
            clast = cols[0] if cols[0] != "-" else "0.00"
            cbid = cols[1]
            cask = cols[2]
            cvol = cols[3]
            c_oi = cols[4]
            strike = cols[5]
            plast = cols[6] if cols[6] != "-" else "0.00"
            pbid = cols[7]
            pask = cols[8]
            pvol = cols[9]
            p_oi = cols[10]
            c = OptionContract(ticker, strike, exp_in_years[i], clast, cbid, cask, cvol, c_oi, True)
            p = OptionContract(ticker, strike, exp_in_years[i], plast, pbid, pask, pvol, p_oi, False)
            calls.append(c)
            puts.append(p)
            print(f"scripts/options.py :: Processed Call {c} and Put {p}")
            time.sleep(1.0)

        expiries.append(OptionExpiry(ticker, formatted_expiration_dates[i], exp_in_years[i], calls, puts))
    
    option_chain = OptionChain(ticker, expiries)
    for expiry in option_chain.expiries:  # For each expiry
        for i in range(0, len(e.calls)):  # For each strike
            print(f"scripts/options.py :: Expiry {expiry.date} ::: "
                f"Strike {expiry.calls[i].strike} | "
                f"Calls {expiry.calls[i]} | "
                f"Puts {expiry.puts[i]}\n")