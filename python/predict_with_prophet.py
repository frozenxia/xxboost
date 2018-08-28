import pymysql
import pandas as pd
from fbprophet import Prophet
from datetime import datetime, timedelta
import seaborn as sns
import numpy as np
import sys
def get_connection(db):
    
#     if db == 'c03':  
    return pymysql.connect(host='c4-mc-caifeng-db00.bj',
                             user='caifeng_wr',
                             password='KKaLUlaaSE2Zj1Bx9NWtHpazpFxkkpMV',
                             db='caifeng_fault',
                             charset='utf8mb4',
                             cursorclass=pymysql.cursors.DictCursor)
    

def get_shouhou_data(sku,problem,endtime,cnn,starttime = '2018-03-15',source='shouhou'):
    with cnn.cursor() as cur:
        sql = "select create_date as ds ,count as y from abnormal where device_name = '%s' and problem = '%s' and source = '%s' and create_date >= '%s' and create_date <= '%s' " %(sku,problem,source,starttime,endtime)
        return pd.read_sql(sql,cnn)
             
def predict_values(df,periods =7):
    #%matplotlib inline
    m = Prophet()
    m.fit(df)
    future = m.make_future_dataframe(periods = periods)
    forcast = m.predict(future)
#     m.plot(forcast)    
    
#     m.plot_components(forcast)
#     print(forcast.tail(periods))
    return forcast.tail(periods)['yhat'].sum()

def get_all_sku_and_problem(cnn,starttime,endtime,threshold=10,source='shouhou'):
    with cnn.cursor() as cur:
        sql = "select * from (select device_name,problem ,avg(count) as cnt from abnormal where create_date between '%s' and '%s' and source= '%s' group by device_name,problem ) a where a.cnt >=%d " %(starttime,endtime,source,threshold)
        print(sql)
        cur.execute(sql)
        res = cur.fetchall()
        pros = []
        for re in res:
            pr = []
            pr.append(re.get('device_name'))
            pr.append(re.get('problem'))
            pr.append(re.get('cnt'))
            
            pros.append(pr)
        
        return pros
    

def predict_for_shouhou(cnn,endtime,periods = 7):
    starttime =  datetime.strftime(datetime.strptime(endtime,'%Y-%m-%d')- timedelta(days=periods-1),'%Y-%m-%d')
    print(starttime)
    sku_pros = get_all_sku_and_problem(cnn,starttime,endtime)
    res = pd.DataFrame(columns=['sku','pro','actual','predict']) 
    for sp in sku_pros:
#         print(sp)
        try:
        
            df = get_shouhou_data(sp[0],sp[1],endtime,cnn)
            base = df.tail(periods)['y'].sum()
            pv = predict_values(df,periods)
            res = res.append({'sku':sp[0],'pro':sp[1],'actual':sp[2]*7,'predict':pv},ignore_index=True)
        except :
            print(sp)
            #         print({'sku':sp[0],'pro':sp[1],'actual':sp[2],'predict':pv/periods,'p_base':base})
#         break
    
    return res


def predict_for_shouhou_v2(cnn,endtime,periods = 7):
    starttime =  datetime.strftime(datetime.strptime(endtime,'%Y-%m-%d')- timedelta(days=periods-1),'%Y-%m-%d')
    print(starttime)
    sku_pros = get_all_sku_and_problem(cnn,starttime,endtime)
    res = pd.DataFrame(columns=['sku','pro','actual','predict']) 
    for sp in sku_pros:
#         print(sp)
        try:
        
            df = get_shouhou_data(sp[0],sp[1],starttime,cnn)

    #         df = get_shouhou_data(sp[0],sp[1],endtime,cnn)
            base = df.tail(periods)['y'].sum()
    #         print(df)
            pv = predict_values(df,periods)
            df2 = update_dsy_dataframe(df,periods)
            df2 = df2[['ds','y1']]
            df2.columns =['ds','y']
            pv2 = predict_values(df2,periods)
            res = res.append({'sku':sp[0],'pro':sp[1],'actual':sp[2]*7,'predict':pv,'p_base':base,'pv2':pv2},ignore_index=True)
        except :
            print(sp)
            #         print({'sku':sp[0],'pro':sp[1],'actual':sp[2],'predict':pv/periods,'p_base':base})
#         break
    
    return res


def update_dsy_dataframe(dt,periods):
    dt['y1'] = 0
    for index,rows in dt.iterrows():
        start = max(index - periods+1,0)
        ic = index - start+1
        dt.loc[index,'y1'] = dt.iloc[start:index+1,]['y'].sum()/ic
    return dt

def get_active_info(cnn,starttime,endtime):
    with cnn.cursor() as cur:
        sql = "select sku,sum(count) as num from sku_active_info where create_date between '%s' and '%s' group by sku"%(starttime,endtime)
        print(sql)
        cur.execute(sql)
        res = cur.fetchall()
        mp ={}
        for re in res:
            mp[re.get('sku')] = re.get('num')
        return mp

def get_shouhou_problems(cnn,starttime,endtime):
    with cnn.cursor() as cur:
        sql = "select device_name,problem,sum(count) as tt from abnormal where  create_date <= '%s'  and create_date >= '%s' and source= 'shouhou' group by device_name,problem "%(endtime,starttime)
        print(sql)
        cur.execute(sql)
        res = cur.fetchall()
        mp = {}
        for re in res:
            mp[re.get('device_name') + '++++' + re.get('problem')] = re.get('tt')
            
        return mp
        
        
        
def calculate_ffr(cnn,endtime,starttime='2011-01-01'):
    
    re = predict_for_shouhou(cnn,endtime)
    
    active_infos = get_active_info(cnn,starttime,endtime)
    lst =  datetime.strftime(datetime.strptime(endtime,'%Y-%m-%d')- timedelta(days=30),'%Y-%m-%d')
    last_month_ai = get_active_info(cnn,lst,endtime)
#     
    shouhou_problems = get_shouhou_problems(cnn,starttime,endtime)
#     last_month_sp = get_shouhou_problems(cnn,lst,endtime)
    
    
    
    
    ret = pd.DataFrame(columns=['sku','pro','active','p_count','p_count_f','p_active_f','ffr1','ffr2'])
    
    for idx,row in re.iterrows():
        ky = row['sku'] + '++++' + row['pro']
        p_count = shouhou_problems[ky]
        p_count_f = row['predict']
        p_active = active_infos[row['sku']]
        p_active_f = (float(last_month_ai[row['sku']])/30)*7
        
        ffr1 = float(p_count)/float(p_active)
        ffr2 = (float(p_count) + float(p_count_f)) / (float(p_active) + float(p_active_f))
        
#         print(ffr1,ffr2,ky,p_active,p_count)
        
        ret = ret.append({'sku':row['sku'],'pro':row['pro'],'active':p_active,
                          'p_count': p_count,'p_count_f':p_count_f,'p_active_f':p_active_f,'ffr1':ffr1,'ffr2':ffr2},ignore_index=True)
#     print(active_infos)
#     print(lst)
#     print(last_month_ai)
#     print(shouhou_problems)
#     print(last_month_sp)
#     print(ret)
    return ret
    
        
def writeback_predict(cnn,pre,endtime):
    with cnn.cursor() as cur:
        cur.execute("delete from shouhou_ffr_predict where create_date = '%s'" %(endtime))
        for index,row in pre.iterrows():
            sql = "insert into shouhou_ffr_predict(create_date,device_name,problem,active_counts,problem_counts,problem_counts_p,active_counts_p,ffr1,ffr2) values ('%s','%s','%s','%s','%s','%s','%s','%s','%s')" 
            sql = sql%(endtime,row['sku'],row['pro'],row['active'],row['p_count'],row['p_count_f'],row['p_active_f'],row['ffr1'],row['ffr2'])
            print(sql)
            cur.execute(sql)
    
    cnn.commit()




print(','.join(sys.argv))
re = calculate_ffr(get_connection(None),sys.argv[1])
writeback_predict(get_connection(None),re,sys.argv[2])