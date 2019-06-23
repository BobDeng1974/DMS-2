# coding: utf-8
import os
import cv2
import pandas as pd
import numpy as np
import glob

data_path = '/home/share/dsm_dataset/'
if not os.path.exists(data_path + 'cropped_data'):
    os.makedirs(data_path + 'cropped_data')

anno_lst =[]

def cropImage(img_file, bbox, type_id, size, stride):

    basename = os.path.basename(img_file)
    image = cv2.imread(img_file)
    
    """
    :param image: 要滑窗的图片
    :param bbox: 图片中唯一的bbox，存储内容为(xmin, ymin, xmax, ymax)
    :param size: 滑窗尺寸
    :param stride: 滑窗步长
    :return:
    cropImages，所有有效的滑窗（有效滑窗指的是完全包含bbox的滑窗、与bbox无交集的滑窗）
    cropBboxes，完全包含bbox的滑窗其对应的cropBboxes元素为bbox，与bbox无交集的滑窗其对应的cropBboxes元素为[]
    """
    bbox = np.array(bbox)
    assert bbox.shape == (4,)
    hOfImg, wOfImg = np.shape(image)[:2]
    sampleNumOfX = np.ceil((wOfImg - size) / stride) + 1
    sampleNumOfY = np.ceil((hOfImg - size) / stride) + 1
    cropImages = []
    cropBboxes = []
    for indOfX in range(int(sampleNumOfX)):
        for indOfY in range(int(sampleNumOfY)):
            xminOfCrop = indOfX * stride
            yminOfCrop = indOfY * stride
            xmaxOfCrop = xminOfCrop + size - 1
            ymaxOfCrop = yminOfCrop + size - 1
            if xmaxOfCrop > (wOfImg - 1):
                xminOfCrop = wOfImg - size
                xmaxOfCrop = wOfImg - 1
            if ymaxOfCrop > (hOfImg - 1):
                yminOfCrop = hOfImg - size
                ymaxOfCrop = hOfImg - 1
            cropAreaCoor = [xminOfCrop, yminOfCrop, xmaxOfCrop, ymaxOfCrop]
            
            cropArea = image[yminOfCrop: ymaxOfCrop + 1, xminOfCrop: xmaxOfCrop + 1]
            interBbox = np.concatenate([np.maximum(bbox[:2], cropAreaCoor[:2]),
                                        np.minimum(bbox[2:], cropAreaCoor[2:])], axis=-1)
            
            # 完全包含bbox的滑窗其对应的cropBboxes元素为bbox
            if np.all(interBbox == bbox):

                interBbox = np.concatenate([interBbox[[0,2]] - cropAreaCoor[0],
                                            interBbox[[1,3]] - cropAreaCoor[1]], axis=-1)
                cropImages.append(cropArea)
                cropBboxes.append(interBbox)
            '''
            # 与bbox无交集的滑窗其对应的cropBboxes元素为[]
            if np.any(interBbox[:2] >= interBbox[2:]):
                cropImages.append(cropArea)
                cropBboxes.append([0, 0, 0, 0])
            '''
    assert len(cropImages) == len(cropBboxes)
   #cropBboxes [xmin, ymin, xmax, ymax]

    for i in range(len(cropImages)):
        anno_lst.append(list((basename[:-4] + '_' + str(i) + ".jpg", str(cropBboxes[i][0]), str(cropBboxes[i][2]),
        str(cropBboxes[i][1]), str(cropBboxes[i][3]), str(type_id))))
        #cv2.rectangle(cropImages[i], tuple((cropBboxes[i][0], cropBboxes[i][2])), tuple((cropBboxes[i][1], cropBboxes[i][3])), (0,255,0), 4)
        cv2.imwrite(data_path + 'cropped_data/' + basename[:-4] + '_' + str(i) + ".jpg", cropImages[i])



def main():

    df = pd.read_csv(data_path + 'anno.csv', header=None)
    df.columns = ["file_name", "xmin", "ymin", "xmax", "ymax", "type"]
    #print(df.head()
    #df.drop(["type"], axis=1,inplace=True)
    data_dict = df.set_index('file_name').T.to_dict('list')
    images_path = '/home/share/dsm_dataset/JPEGImages'
    images = glob.glob('{}/*.jpg'.format(images_path))
    for image in images:
        basename = os.path.basename(image)
        print(basename)
        #print(data_dict[basename])
        if data_dict[basename][-1] is 'phone':
            cropImage(image,  data_dict[basename][:-1], data_dict[basename][-1], 250, 5)
        else:
            cropImage(image, data_dict[basename][:-1], data_dict[basename][-1], 250, 10)
    
    df = pd.DataFrame(anno_lst, columns=["filename", "xmin", "ymin", "xmax", "ymax", "type"])
    df.to_csv(data_path + "croped.csv", header = 0, index = 0)

    
if __name__ == "__main__":
    main()
