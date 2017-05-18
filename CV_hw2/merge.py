import cv2
import numpy as np
import sys

if len(sys.argv) != 3:
	print ('python3 main.py srcPathPrefix limit(excluded)')
	print ('Usage: merge {srcPathPrefix}{0-limit-1}-0.jpg to merged.bmp')
	sys.exit()

imgList = sys.argv

target = sys.argv[1] + "1-0.jpg"
print(target)
targetMat = cv2.imread(target)
targetMat = np.copy(targetMat)
targetMat.fill(0)

for i in range(1, int(sys.argv[2])):

	img = sys.argv[1] + str(i) + "-0.jpg"
	print ('merging ' + img)
	mat = cv2.imread(img)

	x, y, _ = mat.shape
	print(mat.shape)

	for i in range(x):
		for j in range(y):
			if np.average(mat[i][j]) > 0:
				targetMat[i][j] = mat[i][j]

cv2.imwrite('merged.bmp', targetMat)