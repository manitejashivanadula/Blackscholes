![PRICER](https://user-images.githubusercontent.com/117934424/205066174-f253384b-2325-4fc3-a983-4ba680ff48d5.PNG)


#OPTIONS PRICING TOOL  

Options pricer with Discrete Dividend using QuantLib, Python, Javascript, Bootstrap and Bloomberg API's ( RUN THIS TOOL USING SPYDER IN ANACONDA )

I have made this tool by keeping my mind to make the options pricing easier. This is a dynamic tool where you can calculate the multiple options in a single click.

=> The data required for this pricer are being taken dynamically from bloomberg using the Bloomberg API's (BLPAPI & XBBG).

=> The pricer is provided with a refresh button to get the latest bloomberg data on everytime you click it. 

=> All the BID and ASK prices of MARKET_SPOT are dynamically adjusted to your SPOT Price. 

=> I am also calculating the BID-VOL AND ASK-VOL of these prices using the Greeks(DELTA & VEGA) and displaying for each individual options.

=> I am taking the Interest rates swaps curve depending upon the curreny of the Ticker that you enter and upon selection of maturity the appropriate interest rate will be automatically gets selected. 

=> I taken care of dividends if they fall with in the life of the options maturity and aslo provided an option to take % of dividends to build strategies.

=> In the end I have developed two screens. One is for displaying Adjusted BID & ASK prices and Adjusted BID-VOL & ASK-VOL's according to your custom SPOT 

and other screen is an InterdealerBroker Screen to input thier BID & ASK values and compare with Adjusted BID-VOL & ASK-VOL.

#STRATEGIES 

=> This tool can recognize the type of strategy you are pricing eg: Call Spread or Straddle and display the quote with the strategy.

Eg of quote: MBG GY DEC22 60 AC REF 61.40 2.65/2.75 

=>In total it can track 16 Strategies

a) Call 

b) Put

c) Straddle

d) Strangle

e) Call Spread

f) Put Spread

g) Call Calender

h) Put Calender

i) Call Ladder

j) Put Ladder

k) Call Ratio

l) Put Ratio

m) Call Butterfly 

n) Put Butterfly

o) Risk Reversal

p) Synthetic 


Python Libraries to run this tool...

1) pip install QuantLib-Python

2) pip install pdblp

3) pip install xbbg

4) pip install --index-url=https://bcms.bloomberg.com/pip/simple blpapi
