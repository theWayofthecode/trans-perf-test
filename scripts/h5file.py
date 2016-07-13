import pandas as pd
import re

def ls_HDFStore(filename):
    with pd.HDFStore(filename, mode='r') as store:
        return str(store)

def del_path(filename, path):
    with pd.HDFStore(filename) as store:
        del store[path]

#Not tested yet
def get_dfs(filename, pattern_str):
    pattern = re.compile(pattern_str)
    dfs=[]
    with pd.HDFStore(filename, mode='r') as store:
        for i in store.items():
            for j in i:
                if type(j) is str and pattern.match(j):
                    dfs.append(store[j])
    return dfs

def get_df(filename, path):
    with pd.HDFStore(filename, mode='r') as store:
        return store[path]