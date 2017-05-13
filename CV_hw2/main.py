import cv2
import numpy as np

puzzle = "test/logo/puzzle1.bmp"
target = "test/logo/target.bmp"

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

def L1Norm(data):
	return np.sum(abs(data), axis = 1)

def FindKNN(puzzleDes, targetDes, KNN = 4):

	puzzleShape, _ = puzzleDes.shape
	targetShape, _ = targetDes.shape
	matches = []

	for d in range(targetShape):
		
		newPuzzleDes = np.repeat(puzzleDes[d].reshape((1,128)), targetShape, axis = 0)		
		Diff = L1Norm(newPuzzleDes - targetDes).argsort()
		DiffIndex = Diff[:KNN]

		for i in DiffIndex:
			matches.append({"puzzleIndex" : d, "targetIndex" : i, "targetDescripter" : newPuzzleDes[i]})
		
	return matches

KNNmatches = FindKNN(puzzleDes, targetDes)

print(KNNmatches[0]["puzzleIndex"])

dummy = np.zeros((1,1))

index = 30
puzzleKp = [puzzleKp[index]]
targetKp = [targetKp[int(KNNmatches[n]["targetIndex"])] for n in range(len(KNNmatches)) if int(KNNmatches[n]["puzzleIndex"]) == index]

res = cv2.drawKeypoints(puzzleMatGray, puzzleKp, dummy, flags=cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

cv2.imwrite('sift_keypoints1.jpg',res)

res2 = cv2.drawKeypoints(targetMatGray, targetKp, dummy, flags=cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)

cv2.imwrite('sift_keypoints2.jpg',res2)
