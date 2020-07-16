# -*- coding: utf-8 -*-
import cv2
import numpy as np
import random
import os
def sp_noise(img, snr):  # 添加椒盐噪声
    rate = 1 - snr
    output = np.zeros(img.shape, np.uint8)
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            threshold1 = random.random()
            if(threshold1 >= rate):
                output[i, j] = img[i, j]
            else:
                threshold2 = random.random()
                if(threshold2 >= 0.5):
                    output[i, j] = 0
                else:
                    output[i, j] = 255
    return output


def adaptive_median_filter(img, minsize, maxsize):  # 自适应中值滤波
    '''
    :param img: 图片
    :param minsize: 窗口最小值
    :param maxsize: 窗口最大值
    :return: 滤波之后的图片
    '''
    border_size = maxsize // 2  # 边界大小
    # 扩展图像的边界
    src = cv2.copyMakeBorder(img, border_size, border_size, border_size, border_size, cv2.BORDER_REFLECT)
    for i in range(border_size, src.shape[0] - border_size):
        for j in range(border_size, src.shape[1] - border_size):
            filter_size = minsize   # 最小的滤波窗口
            for k in range((maxsize - minsize) // 2 + 1):
                kernel_r = filter_size // 2  # 滤波窗口半径
                rio = src[i - kernel_r: i + kernel_r + 1, j - kernel_r: j + kernel_r + 1]  # 窗口范围
                max_pixel = np.max(rio)  # 窗口内最大值
                min_pixel = np.min(rio)  # 窗口内最小值
                med_pixel = np.median(rio)  # 窗口内中值
                Zxy = src[i, j]  # 窗口的中心点
                if (med_pixel > min_pixel) and (med_pixel < max_pixel):  # A层次
                    if (Zxy > min_pixel) and (Zxy > max_pixel):  # B层次
                        src[i, j] = src[i, j]
                        break
                    else:
                        src[i, j] = med_pixel
                        break
                else:
                    filter_size = filter_size + 2
                    if (filter_size <= maxsize):
                        continue
                    else:
                        src[i, j] = src[i, j]
                        break
    output = src[border_size : border_size + img.shape[0], border_size : border_size + img.shape[1]]  # 去掉拓展的边界
    return output


def adaptive_average_filter(img, sigma_N, size):  #自适应均值滤波
    '''

    :param img: 图片
    :param sigma_N: 需估计的噪声方差
    :param size: 窗口大小
    :return: 处理后的图像
    '''
    border_size = size // 2  # 边界大小
    filter_size = size  # 窗口大小
    # 扩展图像的边界
    src = cv2.copyMakeBorder(img, border_size, border_size, border_size, border_size, cv2.BORDER_REFLECT)
    kernel_r = filter_size // 2  # 滤波窗口半径
    for i in range(border_size, src.shape[0] - border_size):
        for j in range(border_size, src.shape[1] - border_size):
            rio = src[i - kernel_r: i + kernel_r + 1, j - kernel_r: j + kernel_r + 1]
            mL = np.average(rio)  # 窗口内的均值
            sigma_L = np.var(rio)  #窗口内的方差
            src[i, j] = src[i, j] - sigma_N * (src[i, j] - mL) / sigma_L  # 滤波公式
    output = src[border_size: border_size + img.shape[0], border_size: border_size + img.shape[1]]  # 去除拓展的边界
    return output
'''def gasuss_noise(img,means,sigma):  # 添加高斯噪声
    image = np.array(img / 255, dtype=float)
    ran = np.random.randn(img.shape[0], img.shape[1])
    noise = means + math.sqrt(sigma)* ran
    out = image + noise
    output = np.uint8(out * 255)
    return output
'''



# 主函数

path = os.getcwd()  # 获取当前路径
file_name = path + "\\2.png"
img = cv2.imread(file_name, 0)  # 读取原图片
'''cv2.namedWindow('image',cv2.WINDOW_NORMAL)
cv2.resizeWindow("image", 1200, 800)
cv2.imshow("image", img)
cv2.waitKey(0)
cv2.destroyAllWindows()'''
minsize = 3  # 初始窗口大小
maxsize = 21  # 窗口的最大值
snr = 0.9  # 信噪比

img_array = np.array(img)
img_SPnoise = sp_noise(img, snr)  # 为图像添加椒盐噪声
#img_Gnoise = gasuss_noise(img_array, 1, 0.01)



dst_m = adaptive_median_filter(img_SPnoise, minsize, maxsize)  # 自适应中值滤波
cv2.namedWindow('image',cv2.WINDOW_NORMAL)
cv2.resizeWindow("image", 1183, 830)
cv2.imshow("image", dst_m)
cv2.waitKey(0)
cv2.destroyAllWindows()

file_name = path + "\\3.png"
img_Gnoise = cv2.imread(file_name, 0)  # 读取被高斯噪声污染的图像
blur = cv2.blur(img_Gnoise, (7, 7))  # 7*7算术均值滤波
dst_a = adaptive_average_filter(img_Gnoise, 200, 7)  # 7*7自适应均值滤波
res = np.hstack(( blur, dst_a))
cv2.namedWindow('image',cv2.WINDOW_NORMAL)
cv2.resizeWindow("image", 2366, 830)
cv2.imshow("image", res)
cv2.waitKey(0)
cv2.destroyAllWindows()
