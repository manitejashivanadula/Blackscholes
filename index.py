import QuantLib as ql
import numpy as np
from flask import Flask, render_template, request, json
from scipy.stats import norm
from datetime import datetime
from datetime import date
from dateutil.parser import parse
import bloom_api
from numpy import array


app= Flask(__name__)
app.add_url_rule('/Blackscholes_bloomberg_apicall',view_func=bloom_api.Bloom_ticker_api, methods=['GET','POST'])
app.add_url_rule('/Blackscholes_bloomberg_strikecall',view_func=bloom_api.Bloom_strike_api, methods=['GET','POST'])
app.add_url_rule('/Blackscholes_bloomberg_bid_ask',view_func=bloom_api.Bloom_bid_ask_api, methods=['GET','POST'])
app.add_url_rule('/Blackscholes_table_refresh',view_func=bloom_api.Bloom_refresh_api, methods=['GET','POST'])
app.add_url_rule('/Blackscholes_bloomberg_Get_interest_rate',view_func=bloom_api.Bloom_get_interestrate, methods=['GET','POST'])


@app.route('/Blackscholes_model')
def index():
    return render_template("main.html")
    
N = norm.cdf
dt = date.today()
@app.route('/Blackscholes_model_form', methods=['GET','POST'])

#main function
def BS_data():

#option_price_calculation  
    NEW_raw_data = request.get_json()
    Quantlib_data = NEW_raw_data 
        
    Ticker, Option_data_zone, Option_type_data, Strikeprice, Spotprice, Volatility, Interestrate, Dividend, Maturity, Dividend_date, Multiple, Market_Bid_price, Market_Ask_price, Market_spot=[],[],[],[],[],[],[],[],[],[],[],[],[],[]

#forloop for extracting the raw data from the json and appending them into the list
    for i in Quantlib_data:
        Option_data_zone.append(i['Option_data_zone'])
        Option_type_data.append(i['Option_type_data'])
        Spotprice.append(i['Spotprice'])
        Strikeprice.append(i['Strikeprice'])
        Volatility.append(i['Volatility'])
        Interestrate.append(i['Interestrate'])
        Maturity.append(i['Maturity'])
        Dividend.append(i['Dividend']) 
        Dividend_date.append(i['Dividend_date'])
        Multiple.append(i['Multiple'])
        Market_Bid_price.append(i['Bid_price'])
        Market_Ask_price.append(i['Ask_price'])
        Market_spot.append(i['Market_spot'])
        Ticker.append(i['Ticker_data'])
    
#for taking only the first items of the dividend and dividend date lists
    Dividend_array, Dividend_date_array =[],[]
    Dividend_array = Dividend[0]
    Dividend_date_array = Dividend_date[0]
    
#for priniting arrays for checking later delete  
    print("Dividend:", Dividend_array)   
    print("Dividend date:", Dividend_date_array) 
    
#Todays date in Quantlib format
    today_date_result=[]
    for i in range(len(Maturity)):
        date=datetime.strftime(dt, '%#d')
        month=datetime.strftime(dt, '%#m')
        year=datetime.strftime(dt, '%Y')
        today_date=[]
        today_date=[int(date)]
        today_date.append(int(month))
        today_date.append(int(year))
        today_date_result.append(today_date)
#print("Today's date", today_date_result)
    
    Maturity_date,Maturity_date_list=[],[]
#for loop for converting the matrutiry into the quantlib maturity date format
    for i in range(len(Maturity)):
        datetime_str = Maturity[i]
        new_datetime_str = datetime_str.replace("/", "-" )
        datetime_object = datetime.strptime(new_datetime_str, '%m-%d-%y').date()
        date=datetime.strftime(datetime_object, '%#d')
        month=datetime.strftime(datetime_object, '%#m')
        year=datetime.strftime(datetime_object, '%Y')
        Maturity_date=[int(date)]
        Maturity_date.append(int(month))
        Maturity_date.append(int(year))
        Maturity_date_list.append(Maturity_date)
#print("Maturity_date :", Maturity_date_list)
    
    Matched_div_date,Updated_Matched_div_date=[],[]
#for loop for converting the Dividend_date into the quantlib Dividend_date format
    for i in range(len(Maturity)):
        Matched_div_date=[]
        for j in range(len(Dividend_date_array)):
            date1 = parse(Maturity[i])
            date2 = parse(Dividend_date_array[j])
            if(date2 <= date1):
                datetime_str=Dividend_date_array[j]
                datetime_object = datetime.strptime(datetime_str, '%d-%m-%Y').date()
                date=datetime.strftime(datetime_object, '%#d')
                month=datetime.strftime(datetime_object, '%#m')
                year=datetime.strftime(datetime_object, '%Y')
                Dividend_date_list=[int(date)]
                Dividend_date_list.append(int(month))
                Dividend_date_list.append(int(year))
                Matched_div_date.append([Dividend_date_list,Dividend_array[j]])
        Updated_Matched_div_date.insert(i,Matched_div_date)
    #print('Updated_Matched_div_date',Updated_Matched_div_date)
    
#if divdates are less than maturity dates so the array will be empty to repalce the empty array with below hardcoded code
    Empty_div_date=[today_date,0]
    for i in Updated_Matched_div_date:
        if(len(i)==0):
            i.append(Empty_div_date)
    print('Final Div with divdates',Updated_Matched_div_date)
    
#list for combining the entire data into one list except the divdate with dividend
    combine_pricer_list_result=[]
    combine_pricer_list= [today_date_result, Maturity_date_list, Spotprice, Strikeprice,Volatility, Interestrate, Option_data_zone, Option_type_data]
    #print("combine_pricer_list:",combine_pricer_list)
    
#for loop for seperating the sublist into seperate lists according to the Quantlib foramt
    for i in range(len(Maturity_date_list)):
        value=list(list(list(list(list(list(list(list(list(zip(*combine_pricer_list)))))))))[i])
        combine_pricer_list_result.append(value)
#print("combine_pricer_list_result :", combine_pricer_list_result)

#for loop appending the dividend with divdate list into the main list at required position using insert funciton
    for i in range(len(combine_pricer_list_result)):
        combine_pricer_list_result[i].insert(6,Updated_Matched_div_date[i])
            
    print(combine_pricer_list_result)
    
#---------------------From here coding for the option strategy display structure----------------------
    strategy_maturity_list,strategy_maturity_display_result=[],[]
    for i in range(len(Maturity)):
        datetime_str = Maturity[i]
        new_datetime_str = datetime_str.replace("/", "-" )
        datetime_object = datetime.strptime(new_datetime_str, '%m-%d-%y').date()
        day=datetime.strftime(datetime_object, '%d')
        month=datetime.strftime(datetime_object, '%B')
        month=month[:3]
        year=datetime.strftime(datetime_object, '%y')
#this is for the strategy logic so we need day,month and year
        strategy_maturity=day+month+year
        strategy_maturity=strategy_maturity.upper()
        strategy_maturity_list.append(strategy_maturity)
#this is for appending in the end list with only month and year
        strategy_maturity_display=month+year
        strategy_maturity_display=strategy_maturity_display.upper()
        strategy_maturity_display_result.append(strategy_maturity_display)    
    
    print(strategy_maturity_list)
    print(strategy_maturity_display_result)
    
    
    #Here Mat_list_matching is to check where all the maturities in the strategy_maturity_list are same or not
    Mat_list_matching = strategy_maturity_list.count(strategy_maturity_list[0]) == len(strategy_maturity_list)
    if (Mat_list_matching):
        print("All the elements are Equal")
        Mat_result= strategy_maturity_list[0]
    else:
        print("Elements are not equal")
        Mat_result = "/".join([str(item) for item in strategy_maturity_list])
    print('Mat_result',Mat_result)
    
    
#Here Display_Mat_list_matching is to check where all the maturities in the strategy_maturity_list are same or not
    disp_Mat_list_matching = strategy_maturity_display_result.count(strategy_maturity_display_result[0]) == len(strategy_maturity_display_result)
    if (disp_Mat_list_matching):
        print("All the Maturities are Equal")
        disp_Mat_list_matching_result= strategy_maturity_display_result[0]
    else:
        print("Maturities are not equal")
        disp_Mat_list_matching_result = "/".join([str(item) for item in strategy_maturity_display_result])
    print('disp_Mat_list_matching_result',disp_Mat_list_matching_result)
    
    
#Here Strike_list_matching is to check where all the strikes in the Strikeprice list are same or not        
    Strike_list_matching= Strikeprice.count(Strikeprice[0]) == len(Strikeprice)
    if (Strike_list_matching):
        print("Strikes are Same")
        Strike_match_result= Strikeprice[0]
    else:
        print("Strikes are not same")
        Strike_match_result = "/".join(map(str, Strikeprice))
#print(Strike_match_result)
    
    
#Code for making option type as P or C depending upon put or call AND MULTIPLE SIGN
    Strategy_Option_type_data=[]
    for i in range(len(Option_data_zone)):
        if(Option_data_zone[0] == 0):
            Strategy_zone= 'E'
        else:
            Strategy_zone= 'A'    
#print('Strategy_zone',Strategy_zone)
    
    for i in range(len(Multiple)): 
        if(Multiple[i]==1 and Option_type_data[i]==0 ):
            Strategy_Option_type_data.append('+P')
        elif(Multiple[i]==1 and Option_type_data[i]==1 ):
            Strategy_Option_type_data.append('+C')    
        elif(Multiple[i]==-1 and Option_type_data[i]==0 ):
            Strategy_Option_type_data.append('-P')    
        elif(Multiple[i]==-1 and Option_type_data[i]==1 ):
            Strategy_Option_type_data.append('-C')
        if(Multiple[i]>1 and Option_type_data[i]==0 ):
            Strategy_Option_type_data.append('+XP')
        elif(Multiple[i]>1 and Option_type_data[i]==1 ):
            Strategy_Option_type_data.append('+XC')    
        elif(Multiple[i]<-1 and Option_type_data[i]==0 ):
            Strategy_Option_type_data.append('-XP')    
        elif(Multiple[i]<-1 and Option_type_data[i]==1 ):
            Strategy_Option_type_data.append('-XC') 
    print('Strategy_Option_type_data', Strategy_Option_type_data)
    
#Code for joining all the strings into one string
    Join_Op_type_data = ''.join(map(str, Strategy_Option_type_data))
    print('Join_Op_type_data',Join_Op_type_data)
    
    print('Mat_list_matching',Mat_list_matching)
    print('Strike_list_matching',Strike_list_matching)
    
#FINAL STRATEGY DISPLAY CODE
    if(Join_Op_type_data == '+C' or Join_Op_type_data =='-C'):    
        Final_STR_builder= Strategy_zone+ 'C'                           #Call 
    elif(Join_Op_type_data == '+P' or Join_Op_type_data == '-P'):   
        Final_STR_builder= Strategy_zone+ 'P'                            #Put
        
    elif((Join_Op_type_data == '+C+P' or Join_Op_type_data =='+P+C') and  Mat_list_matching == True  and Strike_list_matching ==True) :
        Final_STR_builder= Strategy_zone+'STD'                          #Straddle
    elif((Join_Op_type_data == '+C+P' or Join_Op_type_data=='+P+C') and  Mat_list_matching == True  and Strike_list_matching ==False) :
        Final_STR_builder= Strategy_zone+'STG'                          #Strangle
    
    elif((Join_Op_type_data == '+C-C' or Join_Op_type_data =='-C+C') and  Mat_list_matching == True and Strike_list_matching ==False):
        Final_STR_builder= Strategy_zone+'CS'                           #Call spread
    elif((Join_Op_type_data == '+P-P' or Join_Op_type_data =='-P+P') and Mat_list_matching == True  and Strike_list_matching ==False):
        Final_STR_builder= Strategy_zone+'PS'                            #Put Spread
        
    elif((Join_Op_type_data == '+C-C' or Join_Op_type_data =='-C+C') and  Mat_list_matching == False  and Strike_list_matching ==True):
        Final_STR_builder= Strategy_zone+'CC'                           #Call Calender
    elif((Join_Op_type_data == '+P-P' or Join_Op_type_data =='-P+P') and  Mat_list_matching == False  and Strike_list_matching ==True):
        Final_STR_builder= Strategy_zone+'PC'                           #Put Calender 
        
    elif((Join_Op_type_data == '+C-C-C' or  Join_Op_type_data =='-C+C-C' or Join_Op_type_data =='-C-C+C') and  Mat_list_matching == True and Strike_list_matching ==False ):
        Final_STR_builder= Strategy_zone+'CL'                           #Call ladder
    elif((Join_Op_type_data == '+P-P-P' or Join_Op_type_data =='-P+P-P' or Join_Op_type_data =='-P-P+P') and  Mat_list_matching == True and Strike_list_matching ==False ):
        Final_STR_builder= Strategy_zone+'PL'                           #Put ladder 
    
    elif((Join_Op_type_data == '+C-XC' or Join_Op_type_data =='-XC+C') and  Mat_list_matching == True and Strike_list_matching ==False ):
        Final_STR_builder= Strategy_zone+'CR'                           #Call Ratio
    elif((Join_Op_type_data == '+P-XP' or Join_Op_type_data =='-XP+P' )and  Mat_list_matching == True and Strike_list_matching ==False ):
        Final_STR_builder= Strategy_zone+'PR'                            #Put Ratio
    
    elif(Join_Op_type_data == '+C-XC+C' or Join_Op_type_data =='-XC+C+C' or Join_Op_type_data =='+C+C-XC' and  Mat_list_matching == True and Strike_list_matching ==False ):
        Final_STR_builder= Strategy_zone+'CB'                           #Call Butterfly
    elif(Join_Op_type_data == '+P-XP+P' or Join_Op_type_data =='-XP+P+P' or Join_Op_type_data =='+P+P-XP' and  Mat_list_matching == True and Strike_list_matching ==False ):
        Final_STR_builder= Strategy_zone+'PB'                           #Put Butterfly
        
    else:
        Final_STR_builder= 'Not matched with any strategy'
        
    print('Final_STR_builder',Final_STR_builder)
    
    
    
#By declaring the function_return_result list and other lists before for loop helps to capture the values returning by the function inside the foor loop
    function_return_result, function_return_result_1, Vol_array, Vol_array_1,Prices,Prices_1,Vega,Delta=[],[],[],[],[],[],[],[]
    Adj_Bid, Adj_Ask, Adj_Bid_vol, Adj_Ask_vol=[],[],[],[]
    Our_Adj_Bid,Our_Adj_Ask=0,0
    Final_our_option_price=0
    Sum_of_vega=0
    
    for i in combine_pricer_list_result:
        for j in i:
            v_d=(i[0])
            e_d=(i[1])
            u_p=(i[2])
            s_p=(i[3])
            vl=(i[4])
            r_f=(i[5])
            div=(i[6])
            Is_A=(i[7])
            Is_c=(i[8])
        Vol_array.append(i[4])
        
        function_return_result.append(get_option_price(v_d , e_d , u_p, s_p, vl , r_f, div, Is_A, Is_c))
        function_return_result_1.append(get_option_price_1(v_d , e_d , u_p, s_p, vl , r_f, div, Is_A, Is_c))
      
#function_return_result contains first options price followed by Delta and gamma
    print(function_return_result)

#appending the options_prices of first function into Prices list
    for i in function_return_result:
        Prices.append(i[0])
        Delta.append(i[1])
    
#appending the options_prices of second function into Prices_1 list
    for i in function_return_result_1:
        Prices_1.append(float(i))
    
#makeing the Vol_array values increase by 1% and store them in Vol_array_1 list
    for i in Vol_array:
        i=i+(0.01*i)
        Vol_array_1.append(float("{0:.4f}".format(i)))
    
#Formula of Vega which is (P1-P2)/(V1-V2) and appending the vega and (option_price * multiple) to function_return_result list
    for i in range(len(Prices)):
        vega= (Prices[i]- Prices_1[i])/(Vol_array[i]-Vol_array_1[i])
        Vega.append(float("{0:.3f}".format(vega)))
        function_return_result[i].append(Vega[i])
        
    print("Multiple",Multiple)
    print("Prices", Prices)
    #print("Prices_1", Prices_1)
    print("Vol_array", Vol_array)
    #print("Vol_array_1", Vol_array_1)
    print("Vega", Vega)
    print('delta', Delta)
    print("option_prices_with_vega", function_return_result)

#Multiplying the multiple with options prices`
    print("...........Option_price_with_multiple..............")
    np_option_price_value = np.array(function_return_result)
    np_Multiple_values = np.array(Multiple)
    option_price_value_result = np_option_price_value * np_Multiple_values[:, None]
    option_price_value_result=(option_price_value_result.tolist())
    print("Option_price_with_multiple",option_price_value_result)
    
#All the Option_price_with_multiple are added to show one final option value
    for i in option_price_value_result:
        Final_our_option_price=float("{0:.3f}".format(Final_our_option_price+i[0]))
        Sum_of_vega=float("{0:.3f}".format(Sum_of_vega+i[3]))
    print("Final_our_option_price",Final_our_option_price)
    print('Sum_of_vega',Sum_of_vega)
    
    #Column sum of Greeks
    def column_sum(option_price_value_result):
        arr = array(option_price_value_result)
        return sum(arr, 0).tolist()

#Making column sum of Greeks and appending them in the last row of option_price_value_result and Round the entire list float values to only 2 decimal points
    option_price_value_result.append(column_sum(option_price_value_result))
    option_price_value_result= [[round(val, 2) for val in sublst] for sublst in option_price_value_result]
    print(option_price_value_result)
    
#sum of vol_array and vega for calculating the IDB prices
    Sum_vol_array=float("{0:.3f}".format(sum(Vol_array)))
    print('Sum_vol_array',Sum_vol_array)
    
    print("...........Market_values..............")
    print('Bid_price',Market_Bid_price)
    print('Ask_price',Market_Ask_price)
    print('Market_spot',Market_spot[0])

#for calculating the adjusted_bid_price, adjusted_ask_price using the Delta
#formula =>Delta= (Market_Bid-finding_bid)/(Market_spot-Current_spot)
#formula =>Vega = (Current_option_price-Market_bid_price)/(Current_option_vol-finding_vol)
    print("...........Adjusted_values..............")
    for i in range(len(Market_Bid_price)):
        Adj_Bid.append(float("{0:.3f}".format(Market_Bid_price[i] - Delta[i]*(Market_spot[0]-Spotprice[0]))))
        Adj_Ask.append(float("{0:.3f}".format(Market_Ask_price[i] - Delta[i]*(Market_spot[0]-Spotprice[0]))))
        Adj_Bid_vol.append(float("{0:.3f}".format((Vol_array[i] - ((Prices[i] - Adj_Bid[i])/Vega[i])  )*100)))
        Adj_Ask_vol.append(float("{0:.3f}".format((Vol_array[i] - ((Prices[i] - Adj_Ask[i])/Vega[i])  )*100))) 

    print('Adj_Bid',Adj_Bid)
    print('Adj_Ask',Adj_Ask)
    print('Adj_Bid_vol',Adj_Bid_vol)
    print('Adj_Ask_vol',Adj_Ask_vol)
    
    Sum_Adj_Bid_vol=float("{0:.2f}".format(sum(Adj_Bid_vol)))
    print('Sum_Adj_Bid_vol',Sum_Adj_Bid_vol)
    Sum_Adj_Ask_vol=float("{0:.2f}".format(sum(Adj_Ask_vol)))
    print('Sum_Adj_Ask_vol',Sum_Adj_Ask_vol)


#For loop to calculate our_bid price by multiplying adj_bid with miltiple if multiple is positive. else take adj_ask with multiple
    for i in range(len(Adj_Bid)):
        if(Multiple[i]>=1):
            Our_Adj_Bid=Our_Adj_Bid+(Adj_Bid[i]*Multiple[i])
            Our_Adj_Bid= round(Our_Adj_Bid, 2)
        else:
            Our_Adj_Bid=Our_Adj_Bid+(Adj_Ask[i]*Multiple[i])
            Our_Adj_Bid= round(Our_Adj_Bid, 2)
    print("Displaying_Our_Adj_Bid",Our_Adj_Bid)
    
    for i in range(len(Adj_Ask)):
        if(Multiple[i]>=1):
            Our_Adj_Ask=Our_Adj_Ask+(Adj_Ask[i]*Multiple[i])
            Our_Adj_Ask= round(Our_Adj_Ask, 2)
        else:
            Our_Adj_Ask=Our_Adj_Ask+(Adj_Bid[i]*Multiple[i])
            Our_Adj_Ask= round(Our_Adj_Ask, 2)
    print("Displaying_Our_Adj_Ask",Our_Adj_Ask)

#Displaying the final string of Strategy 
    Display_strategy= Ticker[0]+ ' ' + disp_Mat_list_matching_result + ' ' + str(Strike_match_result) + ' '+ Final_STR_builder +' '+ 'REF'+ ' '+ str(Spotprice[0]) + ' '+ str(Our_Adj_Bid) + '/' + str(Our_Adj_Ask)
    print(Display_strategy)
    
    if(Final_STR_builder== 'Not matched with any strategy'):
        Display_strategy= 'Not matched with any strategy'
    else:
        Display_strategy= Display_strategy
    
    print(Display_strategy)
    
    return {'Display_strategy':Display_strategy, 'display_options_data':option_price_value_result, 'Adj_Bid':Adj_Bid, 'Adj_Ask':Adj_Ask,'Adj_Bid_vol':Adj_Bid_vol,
            'Adj_Ask_vol':Adj_Ask_vol,'Sum_vol_array':Sum_vol_array, 'Sum_of_vega':Sum_of_vega, 'Sum_Adj_Bid_vol':Sum_Adj_Bid_vol, 'Sum_Adj_Ask_vol':Sum_Adj_Ask_vol, 'Our_Adj_Bid':Our_Adj_Bid,'Our_Adj_Ask': Our_Adj_Ask, 'Final_our_option_price':Final_our_option_price  }



def get_option_price(valuation_date, expiry_date, underlying_price, strike_price, volatility, risk_free_rate, dividends, is_american ,is_call ):
    new_value= 0
    div_dates= []
    div_values= []
    for div in dividends:
            date = div[0]
            value = div[1]
            div_values.append(value)
            div_dates.append(ql.Date(*date))

#Reformat dates from list into QL date format
    valuation_date = ql.Date(*valuation_date)
    expiry_date = ql.Date(*expiry_date)
    ql.Settings.instance().setEvaluationDate(valuation_date)
    day_count = ql.Actual365Fixed()
    calendar = ql.UnitedStates()

#Reformat prices and rates from list into QL format
    underlying_price = ql.QuoteHandle(ql.SimpleQuote(underlying_price))
    risk_free_rate = ql.YieldTermStructureHandle(ql.FlatForward(valuation_date, risk_free_rate, day_count))
    volatility = ql.BlackVolTermStructureHandle(ql.BlackConstantVol(valuation_date, calendar, volatility, day_count))

    if is_call:
        payoff = ql.PlainVanillaPayoff(ql.Option.Call, strike_price)
    else:
        payoff = ql.PlainVanillaPayoff(ql.Option.Put, strike_price)
    if is_american:
        exercise = ql.AmericanExercise(valuation_date, expiry_date)
    else:
        exercise = ql.EuropeanExercise(expiry_date)
    option = ql.DividendVanillaOption(payoff, exercise, div_dates, div_values)

#Black Scholes process
    process = ql.BlackScholesProcess(underlying_price,risk_free_rate,volatility)

#Create option's pricing engine
    precision_steps = 1000
    engine = ql.FdBlackScholesVanillaEngine(process, precision_steps, precision_steps - 1)
    option.setPricingEngine(engine)

#Price the option
    new_value="{0:.3f}".format(option.NPV())
    
    Delta="{0:.3f}".format(option.delta())
    Gamma="{0:.3f}".format(option.gamma())
    Greeks =[]
#Greeks.append(volatility)
    Greeks.append(float(new_value))
    Greeks.append(float(Delta))
    Greeks.append(float(Gamma))

    return Greeks




def get_option_price_1(valuation_date, expiry_date, underlying_price, strike_price, volatility, risk_free_rate, dividends, is_american ,is_call ):
    new_value= 0
    div_dates = []
    div_values = []
    volatility= volatility +(0.01*volatility)
    
    for div in dividends:
            date = div[0]
            value = div[1]
            div_values.append(value)
            div_dates.append(ql.Date(*date))

#Reformat dates from list into QL date format
    valuation_date = ql.Date(*valuation_date)
    expiry_date = ql.Date(*expiry_date)
    ql.Settings.instance().setEvaluationDate(valuation_date)
    day_count = ql.Actual365Fixed()
    calendar = ql.UnitedStates()

#Reformat prices and rates from list into QL format
    underlying_price = ql.QuoteHandle(ql.SimpleQuote(underlying_price))
    risk_free_rate = ql.YieldTermStructureHandle(ql.FlatForward(valuation_date, risk_free_rate, day_count))
    volatility = ql.BlackVolTermStructureHandle(ql.BlackConstantVol(valuation_date, calendar, volatility, day_count))

#Create option
    if is_call:
        payoff = ql.PlainVanillaPayoff(ql.Option.Call, strike_price)
    else:
        payoff = ql.PlainVanillaPayoff(ql.Option.Put, strike_price)
    if is_american:
        exercise = ql.AmericanExercise(valuation_date, expiry_date)
    else:
        exercise = ql.EuropeanExercise(expiry_date)
    option = ql.DividendVanillaOption(payoff, exercise, div_dates, div_values)

#Black Scholes process
    process = ql.BlackScholesProcess(underlying_price, risk_free_rate,volatility)

#Create option's pricing engine
    precision_steps = 1000
    engine = ql.FdBlackScholesVanillaEngine(process, precision_steps, precision_steps - 1)
    option.setPricingEngine(engine)

#Price the option
    new_value="{0:.3f}".format(option.NPV())
#print("Delta:", option.delta())
#print("gamma:", option.gamma())
    
    Delta=option.delta()
    Gamma=option.gamma()
    Greeks =[]
    Greeks.append(Delta)
    Greeks.append(Gamma)

    return new_value


app.run(port=4004,  host='0.0.0.0', debug=True)

     
