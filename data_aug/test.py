from PIL import Image
import cv2
import numpy as np
import pandas as pd
import os
from glob import glob
from data_aug import box_utils


img_path = "/home/share/dsm_dataset/JPEGImages"
ann_path = "/home/yang/Applications/dms_data/csv/augmented_labels.csv"

total_boxes = {}
anns = pd.read_csv(ann_path, header=None).values
for annotation in anns:
    key = annotation[0].split(os.sep)[-1]
    value = np.array([annotation[1:]])
    if key in total_boxes.keys():
        total_boxes[key] = np.concatenate((total_boxes[key], value), axis=0)
    else:
        total_boxes[key] = value
image_paths = glob(img_path+"/*.jpg")
for path in image_paths:
    image = Image.open(path)
    ann = path.split(os.sep)[-1]
    img = np.array(image)
    img = box_utils.draw_rect(img, total_boxes[ann], color=(0, 0, 255))
    cv2.imshow("img", img)
    cv2.waitKey()

# # x1, y1, x2, y2, _ = v[0]
# # cv2.rectangle(img_origin, (x1, y1), (x2, y2), (255, 0, 0), 1)
# # cv2.imshow("ret1", img_origin)
# cv2.waitKey()
# boxes = np.array(v)[:, :-1].astype(np.float64)
# x = 300 / shape[1]
# y = 300 / shape[0]
# boxes *= [x, y, x, y]
# print(boxes)
# boxes = boxes.astype(np.int)
# x1, y1, x2, y2 = boxes[0]
# print(x1)
# cv2.rectangle(img, (x1, y1), (x2, y2), (255, 0, 0), 2)
# cv2.imshow("ret2", img)
# cv2.waitKey()