# -*- coding: utf-8 -*-
from flask import Blueprint, render_template, request, Flask
import datetime 
from datetime import date
import blpapi
import pdblp
import json

#Function to get the dividend dates and dividends based on the tikcer searched

def Bloom_ticker_api():
    Ticker_data = request.get_json()
    
    ref_data_service = '//blp/refdata'

    securities = [Ticker_data + "Equity"]
    fields = ['BDVD_PR_EX_DATES_AND_DVD_AMOUNTS','last_price']
    request_parameters = []
    overrides = []

    # Initialize a default SessionOptions (localhost:8194)
    sessionOptions = blpapi.SessionOptions()

    # Initialize a Session
    session = blpapi.Session(sessionOptions)

    if not session.start():
        return
    try:      
        # Open the service from which we will request historical data
        if not session.openService(ref_data_service):
            return

        # Retrieve previously opened service
        refDataService = session.getService(ref_data_service)

        # Reference data
        request_data = refDataService.createRequest('ReferenceDataRequest')
        
        # Fill the request parameters
        securities_element = request_data.getElement('securities')
        for security in securities:
            securities_element.appendValue(security)
        
        fields_element = request_data.getElement('fields')
        for field in fields:
            fields_element.appendValue(field)

        for element, element_value in request_parameters:
            request_data.set(element, element_value)

        overrides_element = request_data.getElement('overrides')
        for field_id, value in overrides:
            end_date_override = overrides_element.appendElement()
            end_date_override.setElement('fieldId', field_id)
            end_date_override.setElement('value', value)

        session.sendRequest(request_data)

        # Process received events   
        pool = True
        while(pool):
            # We provide timeout to give the chance for Ctrl+C handling:
            event = session.nextEvent(50)

            if event.eventType() == blpapi.Event.RESPONSE or event.eventType() == blpapi.Event.PARTIAL_RESPONSE:                
                partial_result = dvd_hist_all_response_handler(event)
            
                Dividend_date= partial_result[0]
                Dividend_rate= list(map(float,partial_result[1]))
                Spot_Price = float(partial_result[2])

                if event.eventType() == blpapi.Event.RESPONSE:
                    pool = False
            else:
                # Monotoring messages - They can be printed/logged
                continue
    finally:
        session.stop()
    Output={'Ticker':Ticker_data, 'Spot_price':Spot_Price,'Dividend_date':Dividend_date, 'Dividend_rate':Dividend_rate }
    return  Output

def dvd_hist_all_response_handler(event):
    
    dividend_date_result,Dividend_rate_result,Last_price,Full_div_data = [],[],[],[]
    
    for message in event:
        securitydata_array = message.getElement('securityData')
        for i in range(securitydata_array.numValues()):
            security_data = securitydata_array.getValueAsElement(i)
        
            security = security_data.getElementAsString('security')
            sequence_number = security_data.getElementAsInteger('sequenceNumber')
        
            if security_data.hasElement('securityError'):
                security_error = security_data.getElement('securityError')
                continue

            field_data = security_data.getElement('fieldData')
            
            dvd_hist_all_array = field_data.getElement('BDVD_PR_EX_DATES_AND_DVD_AMOUNTS')
            
            Last_price = field_data.getElementAsString('last_price')

            for j in range(dvd_hist_all_array.numValues()):  
                    
                dvd_hist_all = dvd_hist_all_array.getValueAsElement(j)                

                ex_date_element = dvd_hist_all.getElement('Ex-Date')
                
                Div_date= ex_date_element.getValueAsString() if not ex_date_element.isNull() else ''
                
                dt = datetime.datetime.strptime(Div_date, '%Y-%m-%d')
                #converting thr date format y-m-d date format to d-m-y format
                dividend_date_result.append('{0}-{1}-{2:02}'.format(dt.day, dt.month, dt.year ))

                Dividend_rate_result.append(dvd_hist_all.getElementAsString('Dividend Per Share'))
                
                Full_div_data=[dividend_date_result,Dividend_rate_result,Last_price]
                
    return Full_div_data



#Function to get the options stikes from bloomberg after matching with substring
def Bloom_strike_api():
    
    strike_api_data = request.get_json()
    
    Strike_data, ticker_data=[],[]
    
    for i in strike_api_data:
        Strike_data.append(i['Strike_value'])
        ticker_data.append(i['Ticker_name'])
        
    con = pdblp.BCon(timeout=2000)
    con.start()

    data=con.bulkref(ticker_data[0] + "Equity"  ,flds=['OPT_CHAIN'])

    data=data.loc[:,"value"]

    Bloom_api_options_ticker_data=[]

    for i in data:
        Bloom_api_options_ticker_data.append(i)
        
#SUBSTRING EXAMPLE LIKE THIS "11/18/22 P45 Equity" WHICH WE ARE PASSING TO SEARCH THE LIST OF MATCHED STRIKES    
    sub_string=Strike_data[0]

    Bloom_option_strikes_result= [s for s in Bloom_api_options_ticker_data if sub_string in s]
    Bloom_option_strikes_result.insert(0,'Select Strike')
   
    return {'Bloom_option_strikes_result': Bloom_option_strikes_result}

#for the BID, ASK and VOL PRICES
def Bloom_bid_ask_api():
    con = pdblp.BCon(timeout=2000)
    con.start()
    
    Strike_data, Ticker_data=[],[]
    
    Refresh_Data= request.get_json()

    for i in Refresh_Data:
        Strike_data.append(i['strikes_list'])
        Ticker_data.append(i['ticker_name'])    
    
    Market_spot=con.ref(Ticker_data[0] + "Equity",flds=['last_price'])
    
    Market_spot=Market_spot.loc[:,"value"]

    data=con.ref(Strike_data[0] ,flds=['BID','ASK','IVOL_MID'])
    data=data.loc[:,"value"]
    data[2]="{0:.3f}".format(data[2]/100)

    Ticker_data=[]

    for i in data:
        Ticker_data.append(i)
    
    Output={ 'Strike_price_value': Market_spot[0],'Bid_price': Ticker_data[0],'ASK_Price': Ticker_data[1], 'Vol_value':Ticker_data[2] }

    return  Output



#REFRESH ALL THE BIDS, ASKS AND VOLS IN THE TABLE WITH MARKET SPOT
def Bloom_refresh_api():
    con = pdblp.BCon(timeout=2000)
    con.start()

    Refresh_Data= request.get_json()
    
    Strike_data, Ticker_data=[],[]

    for i in Refresh_Data:
        Strike_data.append(i['strikes_list'])
        Ticker_data.append(i['ticker_name'])    
    
    Market_spot=con.ref(Ticker_data[0] + "Equity",flds=['last_price'])
    Market_spot=Market_spot.loc[:,"value"]

    Strikes_combined_data=[]

    for i in range(len(Strike_data)):
        Strikes_list_data=[]
        data=con.ref(Strike_data[i], flds=['IVOL_MID','BID','ASK'])
        data=data.loc[:,"value"]
        volatility=float(data.iloc[0]/100)
        Bid_price=data.iloc[1]
        Ask_price=data.iloc[2]
        Strikes_list_data.append("{0:.3f}".format(volatility))
        Strikes_list_data.append(Bid_price)
        Strikes_list_data.append(Ask_price)
        Strikes_combined_data.append(Strikes_list_data)

    #Option_market_values order is VOl followed by BID followed by ASK prices 
    Output={ 'Strike_price_value': Market_spot[0],'Option_market_values': Strikes_combined_data }
    
    return Output