import pandas as pd
import re

def ls_HDFStore(filename):
    with pd.HDFStore(filename, mode='r') as store:
        return str(store)

def del_path(filename, path):
    with pd.HDFStore(filename) as store:
        del store[path]

def get_dfs(filename, pattern_str):
    pattern = re.compile(pattern_str)
    dfs=[]
    with pd.HDFStore(filename, mode='r') as store:
        for i in store.items():
            for j in i:
                if type(j) is str and pattern.match(j):
                    dfs.append(store[j])
    return dfs

def get_thrs(lats_df):
    thrs=pd.Series(index=lats_df.keys())
    for k in lats_df.keys():
        lats_df_sorted = lats_df[k].sort_values()
        median = (lats_df_sorted[499] + lats_df_sorted[500])/2
        thrs[k] = k / median
    return thrs

def get_df(filename, path):
    with pd.HDFStore(filename, mode='r') as store:
        return store[path]
