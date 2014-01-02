
Requirements:
Opencv 2.43 or above.
liblinear-1.94 


Method(s):
void sceneDetection() - (at bottom of sceneloading2.cpp) is when there's no input from the camera system. If this method is invoke, then it will take picture of the robot's current view. Which then invoke sceneDetection2(cv::Mat) method.
However, I have no idea which camera we will be using hence I am leaving it blank right now. I know cv_bridge might help covering the sensor image into cv::Mat. 

sceneDetection2(cv::Mat img):
This method use PACT method to extract features from img. Then it will output a file as data1.txt. 

commandline for testing: ./place


Program(s):
predict2.c
This program required a data file (obtain via sceneDetection2), a model file (should be there by default). Then it would output it's predicted class in the command line.
Command Line: ./predict2 dataFile ModelFile

Overalll accuracy:
Indoor/Outdoor: ~96% average
Room classification: ~80% accuracy

Re-training Datas: 

1. Obtaining datas
under Place/main.cpp uncomment //Load_UIUC_Scenes();
for training outdoor / indoor svm:
	in line 26 under UIUCscene.cpp set   bool INDOOR_OUTDOOR = true;
otherwise set it to false.

then make and execute place

2. move the output .txt files into liblinear-1.94 folder then run batchjob

Either ./batchjob or -sh batchjob

This will train both data and output their own training model file


Adding new pictures for training:
1. All pictures for training must have a name under this format: image_XXXX.jpg where X is a number.

2a. Modify these lines in UIUCscene.cpp if a new class was added
line 41 nClass = ; 
line 42 imagesizes[nClass] // how many pictures for each class 
line 43 imagenames[nClass] // name of the class (name as directory)
line 48 classLabels[nClass] // adding a new number at end of the array, for room specification
line 49 labelCorrespondence[nClass] // for classifying indoor/outdoor
line 50 totalNum ;

2b. adding pictures to existing class:
line 42 imagesizes[nClass] // how many pictures for each class 
line 50 totalNum ;


!!!!!!!!!!CAUTION!!!!!!!!!!!

Never delete train_Normal1.txt.model(For Classify Rooms and Places) & train_OutIn1.txt.model (For Classify In and Outdoors);

!!!!!!!!!!END OF CAUTION!!!!!!!!!!



Credits:

This Program using PACT (Principal Component Analysis of Census Transoform histograms) from Jianxin Wu and James M. Rehg and Liblinear-1.94 from the creators of LibSvm.

Training data(s):

Download link: http://www.mediafire.com/download/vjy7srhc7sfdhdd/Data.tar.gz




