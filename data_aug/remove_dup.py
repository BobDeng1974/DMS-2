#!/usr/bin/python

import os
import sys

xml_path = '/home/yang/dataset/new_ann'
img_path = '/home/yang/dataset/new_images'
real_img_path = '/home/yang/dataset/img'
for files in os.listdir(xml_path):
    print(files)
    real_file_name = files[:-4]
    print(real_file_name)
    jpeg_img = real_file_name+'.jpg'
    command = 'cp '+img_path+'/'+jpeg_img+' '+real_img_path
    print(command)
    os.system(command)
