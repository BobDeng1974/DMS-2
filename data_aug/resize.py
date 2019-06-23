import os
import cv2
import numpy as np
import pandas as pd
from tqdm import tqdm
from glob import glob
from PIL import Image
from data_aug import *
from config import config

class Augumentor():
    def __init__(self):
        self.image_paths = glob(config.raw_images+"/*.%s"%config.image_format)   # 默认图片格式为jpg
        self.annotations_path = config.raw_csv_files
        self.nlc = keep_size()     # 不改变原始图像大小的增强方式
        self.wlc = change_size(config)   # 改变原始图像大小的增强方式

    def fit(self):
        total_boxes = {}
        # read box info for csv format
        annotations = pd.read_csv(self.annotations_path,header=None).values
        for annotation in annotations:
            key = annotation[0].split(os.sep)[-1]
            value = np.array([annotation[1:]])
            if key in total_boxes.keys():
                total_boxes[key] = np.concatenate((total_boxes[key],value),axis=0)
            else:
                total_boxes[key] = value
        # read image and process boxes
        for image_path in tqdm(self.image_paths):
            image = Image.open(image_path)
            raw_boxes = total_boxes[image_path.split(os.sep)[-1]].tolist()  # convert csv box to list

            # do augumentation: keep size
            img_file_name = config.augmented_images+image_path.split(os.sep)[-1].split("."+config.image_format)[0]

            # change image size
            raw_labels = np.array(raw_boxes)[:,-1]
            # resize
            resize_image, resize_box = self.wlc.resize(image,np.array(raw_boxes)[:,:-1].tolist(), size=(300, 300))
            self.write_csv(img_file_name+".%s"%config.image_format,[raw_labels,resize_box],original=False)
            self.write_image(resize_image,img_file_name+".%s"%config.image_format)


    def write_csv(self,filename,boxes,original=True):
        saved_file = open(config.augmented_csv_file,"a+")
        if original:
            new_boxes = boxes
            for new_box in new_boxes:
                label = new_box[-1]
                saved_file.write(filename+","+str(new_box[0])+","+str(new_box[1])+","+str(new_box[2])+","+str(new_box[3]) + ","+label+"\n")
        else:
            labels, new_boxes= boxes[0],boxes[1]
            for label,new_box in zip(labels,new_boxes):
                saved_file.write(filename+","+str(new_box[0])+","+str(new_box[1])+","+str(new_box[2])+","+str(new_box[3]) + ","+label+"\n")

    def write_image(self,image,filename):
        image.save(filename)

if __name__ == "__main__":
    if not os.path.exists(config.augmented_images):
        os.makedirs(config.augmented_images)
    # if not os.path.exists(config.augmented_csv_file):
    #     os.makedirs(config.augmented_csv_file)
    detection_augumentor = Augumentor()
    detection_augumentor.fit()