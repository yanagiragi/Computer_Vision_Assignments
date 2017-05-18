import cv2
import numpy as np
import random
from itertools import product

knnThreshold = 600.0

def normalized(a, axis=-1, order=2):
    l2 = np.atleast_1d(np.linalg.norm(a, order, axis))
    l2[l2==0] = 1
    return a / np.expand_dims(l2, axis)

def SIFT(gray):

	sift = cv2.xfeatures2d.SIFT_create()	
	kp, des = sift.detectAndCompute(gray,None)	
	return kp, des, sift

def L1Norm(data, axis = None):
	return np.sum(abs(data), axis = axis)

def FindKNN(puzzleDes, targetDes, KNN = 5):

	puzzleShape, _ = puzzleDes.shape
	targetShape, _ = targetDes.shape
	matches = []

	for d in range(puzzleShape):
		newPuzzleDes = np.repeat(puzzleDes[d].reshape((1,128)), targetShape, axis = 0)		
		Diff = L1Norm(newPuzzleDes - targetDes, axis = 1)
		DiffArg = Diff.argsort()
		DiffIndex = DiffArg[:KNN]

		for i in DiffIndex:
			if Diff[i] < knnThreshold : # without threshold search space would be too large
				matches.append({"puzzleIndex" : d, "targetIndex" : i})
			# matches.append({"puzzleIndex" : d, "targetIndex" : i})
		
	return matches

def generateTransformMatrix(puzzleSamplekp, targetSamplekp):
	
	puzzleY, puzzleX = puzzleSamplekp
	targetY, targetX = targetSamplekp
	
	return np.array((
			(puzzleX, puzzleY, 1,  0,  0, 0, - targetX * puzzleX, -targetX * puzzleY, - targetX),
	        ( 0,  0, 0, puzzleX, puzzleY, 1, - targetY * puzzleX, - targetY * puzzleY, - targetY)
       	)).astype(np.float64)

def getAccuracy(puzzleKp ,targetKp, matches, Tx):

	threshold = 20
	matchgoods = 0.0
	
	for m in matches:
		pKp = np.array(puzzleKp[m['puzzleIndex']].pt)
		transformedPKp = Tx * np.matrix((pKp[1], pKp[0], 1.0)).T
		# divide z 
		if transformedPKp[2,0] == 0.0:
			continue
		
		transformedPKpPos = np.array((transformedPKp[1,0] / transformedPKp[2,0], transformedPKp[0,0] / transformedPKp[2,0]))
		tKp = np.array(targetKp[m['targetIndex']].pt) # => (x, y)
		
		if L1Norm(transformedPKpPos - tKp) < threshold:
			matchgoods += 1

	return matchgoods

def ransac(puzzleKp, puzzleDes, puzzleMatGray, targetKp, targetDes, targetMat, targetMatGray, matches, K = 4):

	container = []

	retargs = []
	RAN_PICK = 4
	
	# sample 4 for ransac
	sample = random.sample([k["puzzleIndex"] for k in matches], 4)
	sampleMatches = [[m for m in matches if m['puzzleIndex'] == s] for s in sample]
	
	rangeOfSampleMatches = (range(len(sm)) for sm in sampleMatches)

	# generate iterator for product of smapling rangeOfSampleMatches per element in sampleMatches
	# e.g. for 4-NN, sample 4, it generates
	# k = (0,0,0,0) (0,0,0,1) (0,0,0,2) (0,0,0,3) (0,0,1,0) ~ (1,1,1,1)
	for k in product(*tuple(rangeOfSampleMatches)):
		
		# get picked matches
		picked = [ sampleMatches[i][k[i]] for i in range(4) ]
		
		TransformMatrix = None
		for p in picked:
			
			spt, tpt = puzzleKp[p['puzzleIndex']].pt, targetKp[p['targetIndex']].pt

			# change to quadrilateral form
			if TransformMatrix == None:
				TransformMatrix = generateTransformMatrix(spt, tpt)
			else:
				TransformMatrix = np.concatenate((TransformMatrix, generateTransformMatrix(spt, tpt)))

		TransformMatrix = np.asmatrix(TransformMatrix)

		# use eigen value to solve to equation
		retval, eigenValue, eigenVector = cv2.eigen(TransformMatrix.T * TransformMatrix)
		
		best = eigenVector[len(eigenVector) - 1]
		best = best.reshape((3, 3))
		
		rate = getAccuracy(puzzleKp, targetKp, matches, best)
		
		retargs.append({'Rate' : rate, 'TransformMatrix' : best})

	return retargs

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