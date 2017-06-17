import cv2
import numpy as np
import random
import sys
from utils import *

if len(sys.argv) < 4:
	print ('python3 main.py srcImagePath targetImagePath outputPrefix [-forward] [-warpTarget]')
	print ('P.S. Store as {outputPrefix}{iter}.jpg')
	sys.exit()

puzzle = sys.argv[1]
target = sys.argv[2]

puzzleMat = cv2.imread(puzzle)
targetMat = cv2.imread(target)

puzzleX, puzzleY, puzzleChannel = puzzleMat.shape
targetX, targetY, targetChannel = targetMat.shape

res = [[  2.93417846e-03 , -1.68418067e-03 ,  8.80998691e-01], [  1.21780562e-03   ,2.34109681e-03,   4.73091906e-01], [ -2.27729977e-07,  -1.00632309e-06  , 2.63542667e-03]]

canvas = np.copy(targetMat)
canvas.fill(0)

iters = 1

for x in range(puzzleX):
	for y in range(puzzleY):

		newPos = res * np.matrix((x, y, 1.0)).T

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
"""
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
"""