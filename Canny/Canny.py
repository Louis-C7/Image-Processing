# Author: Louis Chen
# -*- coding: utf-8 -*-
import cv2
import numpy as np
import os
from matplotlib import pyplot as plt
import math
def img_show(name, img):
    cv2.imshow(name, img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

def gaussian_filter(img, sigma, size):
    '''
    高斯滤波器
    :param img: 输入图像
    :param sigma: 高斯函数中的sigma参数
    :param size: 滤波器大小size = 2k + 1（k为滤波半径）
    :return:
    '''
    gaussian = np.zeros([size, size])
    kernel_r = size // 2  # 滤波窗口半径
    sum = 0
    for i in range(size):
        for j in range(size):
            gaussian[i, j] = (1 / 2*math.pi*sigma*sigma) \
                             * math.exp(-(np.square(i - kernel_r - 1) - np.square(j - kernel_r - 1)) / 2*sigma*sigma)
            sum = sum + gaussian[i, j]  # 计算窗口内的和
    # 归一化
    gaussian = gaussian / sum
    # 扩展图像的边界
    src = cv2.copyMakeBorder(img, kernel_r, kernel_r, kernel_r, kernel_r, cv2.BORDER_REFLECT)
    # 高斯滤波
    H, W = src.shape
    new_src = np.zeros([H, W])
    for i in range(kernel_r , H - kernel_r):
        for j in range(kernel_r, W - kernel_r):
            new_src[i,j] = np.sum(src[i - kernel_r : i + kernel_r + 1, j - kernel_r : j + kernel_r + 1] * gaussian)
    # 去除扩展的边界
    new_img = new_src[kernel_r : kernel_r + img.shape[0], kernel_r : kernel_r + img.shape[1]]
    return new_img

def gradients(img):
    '''
    计算梯度幅值和方向
    :param img: 输入图像
    :return: x方向sobel算子处理后的图像，y方向sobel算子处理后的图像，梯度幅值矩阵，梯度方向矩阵
    '''
    H, W = img.shape
    sobelx = cv2.Sobel(img, -1, 1, 0, ksize=3)  # 求导方向为x的sobel算子
    #sobelx = cv2.convertScaleABs(sobelx)
    sobely = cv2.Sobel(img, -1, 0, 1, ksize=3)  # 求导方向为y的sobel算子
    #sobely = cv2.convertScaleABs(sobely)
    #sobelx = np.zeros([H-1 , W-1 ])
    #sobely = np.zeros([H-1 , W-1 ])
    M = np.zeros([H-1 , W-1 ])
    theta = np.zeros([H-1 , W-1 ])
    for i in range(H-1):
        for j in range(W-1):
            #sobelx[i, j] = img[i + 1, j] - img[i, j]
            #sobely[i, j] = img[i, j + 1] - img[i, j]
            # 计算梯度幅值
            M[i, j] = np.sqrt(np.square(sobelx[i, j]) + np.square(sobely[i, j]))
            # 计算梯度方向
            theta[i,j] = math.atan(sobelx[i, j] / sobely[i, j]) * 180 / math.pi
    return sobelx, sobely, M, theta

def NMS(M, sobelx, sobely):
    '''
    非极大值抑制
    :param M: 梯度幅值矩阵
    :param sobelx: x方向sobel算子处理后的图像
    :param sobely: y方向sobel算子处理后的图像
    :return: 处理后的梯度幅值矩阵
    '''
    H, W = M.shape
    nms = np.copy(M)
    nms[0,:] = nms[:, 0] = nms[H - 1, :] = nms[:, W - 1] = 0
    for i in range(1, H - 1):
        for j in range(1, W - 1):
            if M[i, j ] == 0:
                nms[i, j] = 0
            else:
                gradx = sobelx[i, j]  # 当前点x方向的导数
                grady = sobely[i, j]  # 当前点y方向的导数
                gradTemp = M[i, j]  # 当前点的梯度值

                # 如果y方向梯度值比较大，说明导数方向趋向于y分量
                if np.abs(grady) > np.abs(gradx):
                    weight = np.abs(gradx) / np.abs(grady)  # 权重
                    grad2 = M[i - 1, j]
                    grad4 = M[i + 1, j]
                    # y轴竖直向下，x轴水平向右
                    # 如果x，y方向的梯度方向一致
                    if gradx * grady >0:
                        grad1 = M[i - 1, j - 1]
                        grad3 = M[i + 1, j + 1]
                        # 像素点位置关系
                        # grad1   grad2    xxx
                        # xxx     gradT    xxx
                        # xxx     grad4    grad3
                    else:
                        grad1 = M[i - 1, j + 1]
                        grad3 = M[i + 1, j + 1]
                        # 像素点位置关系
                        # xxx     grad2    grad1
                        # xxx     gradT    xxx
                        # grad3   grad4    xxx
                # 如果x方向梯度值比较大，说明导数方向趋向于x分量
                else:
                    weight = np.abs(grady) / np.abs(gradx)  # 权重
                    grad2 = M[i , j - 1]
                    grad4 = M[i , j + 1]
                    # y轴竖直向下，x轴水平向右
                    # 如果x，y方向的梯度方向一致
                    if gradx * grady > 0:
                        grad1 = M[i + 1, j - 1]
                        grad3 = M[i - 1, j + 1]
                        # 像素点位置关系
                        # xxx       xxx      grad3
                        # grad2     gradT    grad4
                        # grad1     xxx      xxx
                    else:
                        grad1 = M[i - 1, j - 1]
                        grad3 = M[i + 1, j + 1]
                        # 像素点位置关系
                        # grad1     xxx      xxx
                        # grad2     gradT    grad4
                        # xxx       xxx      grad3
                # 计算亚像素的梯度值（插值）
                gradTemp1 = weight * grad1 + (1 - weight) * grad2
                gradTemp2 = weight * grad3 + (1 - weight) * grad4
                # 判断当前点是否为局部最大值，如果是则可能是边缘点
                if gradTemp >= gradTemp1 and gradTemp >= gradTemp2:
                    nms[i, j] = gradTemp
                else:
                    # 当前点不是边缘的点
                    nms[i, j] = 0
    return nms

def double_threshold(nms):
    '''
    双边阈值选取
    :param nms: 非极大值抑制处理后的梯度幅值矩阵
    :return: canny算子处理后的结果
    '''
    H, W = nms.shape
    DTC = np.zeros([H, W])
    # 选取高阈值TH 和低阈值TL ，比率约为为3:1
    TL = 0.02 * np.max(nms)
    TH = 0.08 * np.max(nms)
    for i in range(1, H - 1):
        for j in range(1, W -1):
            if nms[i, j] < TL:
                DTC[i, j] = 0
            elif nms[i, j] > TH:
                DTC[i, j] = 1
            elif ((nms[i-1, j - 1:j + 1] > TH).any() or (nms[i, j - 1:j + 1] > TH).any()
                  or (nms[i + 1, j - 1:j + 1] > TH).any()):
                DTC[i, j] = 1
    return DTC

def self_made_canny(img):  # Canny算子
    img_gauss = gaussian_filter(img, 1.4, 5)
    sobelx, sobely, M, theta = gradients(img_gauss)
    nms = NMS(M, sobelx, sobely)
    DTC = double_threshold(nms)
    return img_gauss, DTC

def main():
    # 获取当前路径
    path = os.getcwd()
    # 图像路径
    file_name = path + "\\6.jpg"
    # 彩色图
    img_rgb = cv2.imread(file_name)
    # 灰度图
    img_gray = cv2.cvtColor(img_rgb, cv2.COLOR_BGR2GRAY)
    # opencv中的canny算子
    edges_cv = cv2.Canny(img_gray, 50, 250)
    # 自建的canny算子
    img_gauss, edges_self = self_made_canny(img_gray)
    # 显示比较结果
    plt.subplot(141), plt.imshow(img_gray, cmap='gray')
    plt.title('The original image'), plt.xticks([]), plt.yticks([])
    plt.subplot(142), plt.imshow(img_gauss, cmap='gray')
    plt.title('Gaussian filter'), plt.xticks([]), plt.yticks([])
    plt.subplot(143), plt.imshow(edges_self, cmap='gray')
    plt.title('My Canny algorithm'), plt.xticks([]), plt.yticks([])
    plt.subplot(144), plt.imshow(edges_cv, cmap='gray')
    plt.title('Canny function in OpenCV'), plt.xticks([]), plt.yticks([])
    plt.show()

main()