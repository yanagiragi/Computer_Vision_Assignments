import cv2
import numpy as np
import random

import time
from itertools import product as iterProduct
start_time = time.time()

knnThreshold = 600.0

puzzle = "test/table/puzzle4.bmp"
target = "test/table/sample.bmp"
#puzzle = "0456624_hw2/TestData/object_11.bmp"
#target = "0456624_hw2/TestData/target.bmp"

puzzleMat = cv2.imread(puzzle)
puzzleMatGray = cv2.cvtColor(puzzleMat,cv2.COLOR_BGR2GRAY)
targetMat = cv2.imread(target)
targetMatGray = cv2.cvtColor(targetMat,cv2.COLOR_BGR2GRAY)

def SIFT(gray):

	sift = cv2.xfeatures2d.SIFT_create()
	
	kp, des = sift.detectAndCompute(gray,None)
	
	return kp, des, sift

puzzleKp, puzzleDes, puzzleSIFT = SIFT(puzzleMatGray)
targetKp, targetDes, targetSIFT = SIFT(targetMatGray)

def L1Norm(data, axis = None):
	return np.sum(abs(data), axis = axis)

def FindKNN(puzzleDes, targetDes, KNN = 4):

	puzzleShape, _ = puzzleDes.shape
	targetShape, _ = targetDes.shape
	matches = []

	for d in range(puzzleShape):
		# print (puzzleDes[d].shape)
		# print (targetShape)
		newPuzzleDes = np.repeat(puzzleDes[d].reshape((1,128)), targetShape, axis = 0)		
		Diff = L1Norm(newPuzzleDes - targetDes, axis = 1)
		DiffArg = Diff.argsort()
		DiffIndex = DiffArg[:KNN]

		for i in DiffIndex:
			# print(Diff)
			if Diff[i] < knnThreshold : # without threshold search space would be too large
				matches.append({"puzzleIndex" : d, "targetIndex" : i, "targetDescripter" : newPuzzleDes[i]})
		
	return matches

KNNmatches = FindKNN(puzzleDes, targetDes)
# KNNmatches = [{'dis': 560.0, 'targetIndex': 177, 'puzzleIndex': 29}, {'dis': 504.0, 'targetIndex': 159, 'puzzleIndex': 38}, {'dis': 477.0, 'targetIndex': 172, 'puzzleIndex': 41}, {'dis': 300.0, 'targetIndex': 171, 'puzzleIndex': 42}, {'dis': 451.0, 'targetIndex': 164, 'puzzleIndex': 45}, {'dis': 368.0, 'targetIndex': 130, 'puzzleIndex': 49}, {'dis': 235.0, 'targetIndex': 296, 'puzzleIndex': 60}, {'dis': 276.0, 'targetIndex': 297, 'puzzleIndex': 62}, {'dis': 490.0, 'targetIndex': 281, 'puzzleIndex': 63}, {'dis': 351.0, 'targetIndex': 280, 'puzzleIndex': 64}, {'dis': 352.0, 'targetIndex': 271, 'puzzleIndex': 65}, {'dis': 133.0, 'targetIndex': 285, 'puzzleIndex': 66}, {'dis': 280.0, 'targetIndex': 266, 'puzzleIndex': 67}, {'dis': 278.0, 'targetIndex': 265, 'puzzleIndex': 68}, {'dis': 202.0, 'targetIndex': 438, 'puzzleIndex': 78}, {'dis': 180.0, 'targetIndex': 435, 'puzzleIndex': 80}, {'dis': 187.0, 'targetIndex': 434, 'puzzleIndex': 81}, {'dis': 444.0, 'targetIndex': 437, 'puzzleIndex': 84}, {'dis': 253.0, 'targetIndex': 427, 'puzzleIndex': 85}, {'dis': 285.0, 'targetIndex': 417, 'puzzleIndex': 87}, {'dis': 290.0, 'targetIndex': 421, 'puzzleIndex': 89}, {'dis': 237.0, 'targetIndex': 433, 'puzzleIndex': 91}, {'dis': 560.0, 'targetIndex': 416, 'puzzleIndex': 93}, {'dis': 339.0, 'targetIndex': 428, 'puzzleIndex': 94}, {'dis': 267.0, 'targetIndex': 517, 'puzzleIndex': 97}, {'dis': 251.0, 'targetIndex': 511, 'puzzleIndex': 98}, {'dis': 275.0, 'targetIndex': 583, 'puzzleIndex': 104}, {'dis': 252.0, 'targetIndex': 582, 'puzzleIndex': 105}, {'dis': 153.0, 'targetIndex': 619, 'puzzleIndex': 108}, {'dis': 590.0, 'targetIndex': 653, 'puzzleIndex': 110}, {'dis': 470.0, 'targetIndex': 613, 'puzzleIndex': 111}]

print("KNN matches : ", len(KNNmatches))

def generateTransformMatrix(sample):
	puzzleSamplekp = puzzleKp[int(sample["puzzleIndex"])].pt
	targetSamplekp = targetKp[int(sample["targetIndex"])].pt
	
	puzzleY, puzzleX = puzzleSamplekp
	targetY, targetX = targetSamplekp
	
	return np.array((
			(puzzleX, puzzleY, 1,  0,  0, 0, - targetX * puzzleX, -targetX * puzzleY, - targetX),
	        ( 0,  0, 0, puzzleX, puzzleY, 1, - targetY * puzzleX, - targetY * puzzleY, - targetY)
       	)).astype(np.float64)

def getAccuracy(best, puzzleMatGray, targetMatGray, iters):
	
	canvas = np.copy(targetMatGray)
	xShape, yShape = puzzleMatGray.shape
	xTargetShape, yTargetShape = targetMatGray.shape
	# print (xShape, yShape)

	ssd = 0.0

	for i in range(xShape):
		for j in range(yShape):
			if puzzleMatGray[i][j] != 0: # omit black in this case
				
				# warping!
				newPos = best * np.matrix((i,j,1)).T;

				# normalize z to 1 to find x , y
				x, y = (int)(newPos[0,0] / newPos[2,0]), (int)(newPos[1,0] / newPos[2,0])

				if x < xTargetShape and x > 0 and y < yTargetShape and y > 0:
					canvas[x][y] = puzzleMatGray[i][j]
					
					# cast uchar to int to prevent overflow
					ssd += abs((int)(canvas[x][y]) - (int)(targetMatGray[x][y]))
	
	cv2.imwrite("result/ransac" + str(iters) + ".bmp", canvas)
	return ssd

def ransac(puzzleDes, puzzleMatGray, targetDes, targetMatGray, matches, K = 4):

	iters = 100

	Nowargmin = -1;
	NowminSSD = 9999999

	for i in range(iters):
		print (str(i) + " th ransac")
		sample = random.sample(matches, K)
		
		transformMatrix = None
		for s in sample:
			if transformMatrix is None:
				transformMatrix = np.asmatrix(generateTransformMatrix(s))
			else:
				transformMatrix = np.concatenate((transformMatrix, generateTransformMatrix(s)), axis = 0)
		
		# print(transformMatrix.shape)
		transformMatrix = np.asmatrix(transformMatrix)
		retval, eigenValue, eigenVectors = cv2.eigen(transformMatrix.T * transformMatrix)
		
		eigenValue = [abs(e) for e in eigenValue]
		best = eigenVectors[np.argmin(eigenValue)]
		best = best.reshape((3,3))
		# print(best)

		ssd = getAccuracy(best, puzzleMatGray, targetMatGray, i);
		print("Accuracy rate = " , ssd)
		
		if NowminSSD > ssd :
			NowminSSD = ssd
			Nowargmin = i
		# return
			
	print (Nowargmin, NowminSSD)


result = ransac(puzzleDes, puzzleMatGray, targetDes, targetMatGray, KNNmatches)


def KpKNNTestDebug():

	# Output specific kps and it's KNN

	print(KNNmatches[0]["puzzleIndex"])

	dummy = np.zeros((1,1))

	index = 30
	puzzleKp = [puzzleKp[index]]
	targetKp = [targetKp[int(KNNmatches[n]["targetIndex"])] for n in range(len(KNNmatches)) if int(KNNmatches[n]["puzzleIndex"]) == index]

	res = cv2.drawKeypoints(puzzleMatGray, puzzleKp, dummy, flags=cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

	cv2.imwrite('sift_keypoints1.jpg',res)

	res2 = cv2.drawKeypoints(targetMatGray, targetKp, dummy, flags=cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

	cv2.imwrite('sift_keypoints2.jpg',res2)