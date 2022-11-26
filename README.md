#OPTIONS PRICING TOOL  

Options pricer with Discrete Dividend using QuantLib, Python, Javascript, Bootstrap and Bloomberg API's

I have made this tool by keeping my mind to make the options pricing easier. This is a dynamic tool where you can calculate the multiple options in a single click.

=> The data required for this pricer are being taken dynamically from bloomberg using the Bloomberg API's (BLPAPI & XBBG).

=> The pricer is provided with a refresh button to get the latest bloomberg data on everytime you click it. 

=> All the BID and ASK prices of MARKET_SPOT are dynamically adjusted to your SPOT Price. 

=> I am also calculating the BID-VOL AND ASK-VOL of these prices using the Greeks(DELTA & VEGA) and displaying for each individual options.

=> I am taking the Interest rates swaps curve depending upon the curreny of the Ticker that you enter and upon selection of maturity the appropriate interest rate will be automatically gets selected. 

=> I taken care of dividends if they fall with in the life of the options maturity and aslo provided an option to take % of dividends to build strategies.

=> In the end I have developed two screens. One is for displaying Adjusted BID & ASK prices and Adjusted BID-VOL & ASK-VOL's according to your custom SPOT 

and other screen is an InterdealerBroker Screen to input thier BID & ASK values and compare with Adjusted BID-VOL & ASK-VOL.

