import os
import pandas as pd
import glob

def merge():
    csv_list = glob.glob('*.csv')
    print('processing ~')
    for i in csv_list:
        fr = open(i, 'r').read()
        with open('ann.csv', 'a') as f:
            f.write(fr)
    print('done ~')

def rm_dup(file):
    df = pd.read_csv(file, header=0)
    datalist = df.drop_duplicates()
    datalist.to_csv(file)

def get_num(li):
    li = list(li)
    set1 = set(li)
    dict1 = {}
    for i in set1:
        dict1.update({i : li.count(i)})
    return dict1

if __name__ == '__main__':
    # merge()

    df = pd.read_csv('croped.csv', header=None)
    # df[0].value_counts()
    labels = get_num(df[5])
    print(labels)