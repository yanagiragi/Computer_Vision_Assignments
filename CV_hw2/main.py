import cv2
import numpy as np
import random
import sys
from utils import *

if len(sys.argv) < 4:
	print ('python3 main.py srcImagePath targetImagePath outputPrefix [-forward]')
	print ('P.S. Store as {outputPrefix}{iter}.jpg')
	sys.exit()

puzzle = sys.argv[1]
target = sys.argv[2]

#puzzle = "test/logo/puzzle3.bmp"
#target = "test/logo/sample.bmp"

puzzleMat = cv2.imread(puzzle)
puzzleMatGray = cv2.cvtColor(puzzleMat,cv2.COLOR_BGR2GRAY)
targetMat = cv2.imread(target)
targetMatGray = cv2.cvtColor(targetMat,cv2.COLOR_BGR2GRAY)

puzzleKp, puzzleDes, puzzleSIFT = SIFT(puzzleMatGray)
targetKp, targetDes, targetSIFT = SIFT(targetMatGray)


dummy = np.zeros((1,1))

res = cv2.drawKeypoints(puzzleMatGray, puzzleKp, dummy, flags=cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

cv2.imwrite('sift_keypoints1.jpg',res)

res2 = cv2.drawKeypoints(targetMatGray, targetKp, dummy, flags=cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

cv2.imwrite('sift_keypoints2.jpg',res2)


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

if len(sys.argv) == 5:
	# foward wrapping

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
					#if int(puzzleMatGray[x][y]) > 0:
						canvas[newX][newY] = puzzleMat[x][y]

		cv2.imwrite(sys.argv[3] + str(iters) +'.jpg', canvas)
		iters += 1
else:
	# Back wrapping

	for i in bestRansac:

		# pick best 10
		if iters > 10 :
			break

		# fill tp black background
		canvas = np.copy(targetMat)
		canvas.fill(0)
		
		for x in range(targetX):
			for y in range(targetY):

				newPos = np.matrix(i['TransformMatrix']).I * np.matrix((x, y, 1.0)).T

				# avoid devide by zero
				if newPos[2,0] == 0.0:
					continue

				newX, newY = (int)(newPos[0,0] / newPos[2,0]), (int)(newPos[1,0]/newPos[2,0])

				if newX >= 0 and newX < puzzleX and newY >= 0 and newY < puzzleY:
					if int(puzzleMat[newX][newY][0]) + (int)(puzzleMat[newX][newY][1]) + (int)(puzzleMat[newX][newY][2]) > 0 : # omit black, cast to integer to ignore uchar overflow warnings
						if np.average(canvas[x][y]) > 0:
							canvas[x][y] = puzzleMat[newX][newY]

		cv2.imwrite(sys.argv[3] + str(iters) +'.jpg', canvas)
		iters += 1