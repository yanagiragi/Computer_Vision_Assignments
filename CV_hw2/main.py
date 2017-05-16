import cv2
import numpy as np
import random
from utils import *

puzzle = "test/table/target.bmp"
target = "test/table/sample.bmp"

#puzzle = "test/logo/puzzle3.bmp"
#target = "test/logo/sample.bmp"

puzzleMat = cv2.imread(puzzle)
puzzleMatGray = cv2.cvtColor(puzzleMat,cv2.COLOR_BGR2GRAY)
targetMat = cv2.imread(target)
targetMatGray = cv2.cvtColor(targetMat,cv2.COLOR_BGR2GRAY)

puzzleKp, puzzleDes, puzzleSIFT = SIFT(puzzleMatGray)
targetKp, targetDes, targetSIFT = SIFT(targetMatGray)


KNNmatches = FindKNN(puzzleDes, targetDes)

print("KNN matches : ", len(KNNmatches))

# result = ransac(puzzleKp, puzzleDes, puzzleMatGray, targetKp, targetDes, targetMat, targetMatGray, KNNmatches)

bestRansac = []
for i in range(100):
    print(i, "th ransac")

    tmpres = ransac(puzzleKp, puzzleDes, puzzleMatGray, targetKp, targetDes, targetMat, targetMatGray, KNNmatches)

    # cause tmpres = [{}]
    for j in tmpres:
    	bestRansac.append(j)

# sort ransac result to get best 10
bestRansac = sorted(bestRansac, key=lambda k : k['Rate'], reverse = True)

# Output Results
iters = 0

# channel should be same, that is 3
puzzleX, puzzleY, puzzleChannel = puzzleMat.shape
targetX, targetY, targetChannel = targetMat.shape

for i in bestRansac:

	# pick best 10
	if iters > 10 :
		break

	# fill tp black background
	canvas = np.copy(targetMat)
	canvas.fill(0)
	
	for x in range(puzzleX):
		for y in range(puzzleY):

			newPos = i['TransformMatrix'] * np.matrix((x, y, 1.0)).T

			# avoid devide by zero
			if newPos[2,0] == 0.0:
				continue

			newX, newY = (int)(newPos[0,0] / newPos[2,0]), (int)(newPos[1,0]/newPos[2,0])

			if newX >= 0 and newX < targetX and newY >= 0 and newY < targetY:
				if int(puzzleMat[x][y][0]) + (int)(puzzleMat[x][y][1]) + (int)(puzzleMat[x][y][2]) > 0 : # omit black, cast to integer to ignore uchar overflow warnings
					canvas[newX][newY] = puzzleMat[x][y]

	cv2.imwrite('result/' + str(iters) +'.jpg', canvas)
	iters += 1