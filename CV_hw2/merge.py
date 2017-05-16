import cv2
import numpy as np

imgList = ['merge/0.jpg','merge/1.jpg','merge/2.jpg','merge/3.jpg','merge/4.jpg','merge/5.jpg','merge/6.jpg','merge/7.jpg']

target = 'merge/sample.bmp'
targetMat = cv2.imread(target)
targetMat = np.copy(targetMat)
targetMat.fill(0)

for img in imgList:

	mat = cv2.imread(img)

	x, y, _ = mat.shape
	print(mat.shape)

	for i in range(x):
		for j in range(y):
			if np.average(mat[i][j]) > 0:
				targetMat[i][j] = mat[i][j]

cv2.imwrite('merged.bmp', targetMat)