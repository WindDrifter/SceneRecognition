0. Copyright

Use of this software is allowed for research purposes. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This software uses the LIBSVM package, included in the libsvm-2.82/ directory.
Please read the COPYRIGHT file for LIBSVM copyright information.

2. Prerequisite

OpenCV and LIBSVM are required for running this software.

OpenCV can be acquired from http://opencvlibrary.sourceforge.net/. Please refer
to their install instructions, and setup your PKG_CONFIG_PATH correctly.

LIBSVM is included in the subdirectory libsvm-2.82/. Note that files in this
directory is revised for usage in this software. Please enter this directory
and run 'make' to compile it.

3. Data required for this software
(These instructions are for the Linux environment, for Windows environment, read below.)

Two datasets are included as example in this software: the Sweden Leaf dataset
(contour only), and the 15 class scene dataset.

The contour of the leaves are included in Data/SwedenLeaf.

The scene images can be acquired from http://www-cvr.ai.uiuc.edu/ponce_grp/data/index.html#scenes.
Please download the http://www-cvr.ai.uiuc.edu/ponce_grp/data/scene_categories/scene_categories.zip
file, put the file to Data/scene/, and run 'unzip scene_categories.zip'.

4. Running the Sweden Leaf example
   4.1 Enter the directory Place/
   4.2 change main.cpp -- uncomment Load_UIUC_Scenes(), and comment out SwedenLeafContour()
   4.3 run 'make'
   4.4 run 'place'; 10 files will be generated, including 'train?.txt' and 'test?.txt'; where ? is in
       1, 2, 3, 4, or 5. These files are the randomly generated training/testing set from the leaf dataset.
   4.5 run 'mv -f t*.txt ../libsvm-2.82/'
   4.6 run 'cd ../libsvm-2.82/tools'
   4.7 change SVM parameters in easy_fix_cg.py. In this example, change to c=128, g=0.0078125, t=2, b=1 in
       lines 57-60.
   4.8 run './batchjob' (depending on your shell, you may need to run 'source batchjog' etc) 
   4.9 results are displayed onscreen.    
  
5. Running the scene recognition example
   5.1 Enter the directory Place/
   5.2 change main.cpp -- comment out Load_UIUC_Scenes(), and uncomment SwedenLeafContour()
   5.3 run 'make'
   5.4 run 'place'; 10 files will be generated, including 'train?.txt' and 'test?.txt'; where ? is in
       1, 2, 3, 4, or 5. These files are the randomly generated training/testing set from the scene dataset.
   5.5 run 'mv -f t*.txt ../libsvm-2.82/'
   5.6 run 'cd ../libsvm-2.82/tools'
   5.7 change SVM parameters in easy_fix_cg.py. In this example, change to c=8, g=0.0078125, t=2, b=1 in
       lines 57-60.
   5.8 run './batchjob' (depending on your shell, you may need to run 'source batchjog' etc) 
   5.9 results are displayed onscreen.
version 1.0 COMMENTS: Please execute "chmod a+x batchjob" in the directory
                      libsvm-2.82/tools if that file is not executable after unzip.
                      Please execute "chmod a+x svm-predict-2" and "chmod a+x svm-train" in the directory
		      libsvm-2.82/
version 2.0 COMMENTS: For this dataset, an option is added to use both
                      the original image and the Sobel images to
		      extract PACT
   5.10 in the function Load_UIUC_Scenes() in UIUCscene.cpp, set
        useSobel = false, and useBoth = true;
   5.11 run steps 5.3 to 5.6
   5.12 chagne SVM parameters in easy_fix_cg.py. set c=8,
        g=0.001953125, t=2, b=1
   5.13 run './batchjob', the average accuracy in this dataset is then 85.07%.
  
6. Windows instructions
   This software is tested in Linux, but not fully tested in Windows. It can be compiled successfully using
Microsoft Visual Studio 2005 (project file 'Place.sln'). Depending on your Windows compiler version, or the
gcc version, the resulting recognition performance may have slight differences.

   The LIBSVM package also has its Windows version, included in libsvm-2.82/windows/. But the LIBSVM Windows
executables are not compiled with the changes made for this software. It is possible to compile the source
code in Windows. 
   
7. Using the PACT representation in your application (using C++)
   7.1 Include the files "global.cpp", "util.cpp" in your project, and #include necessary .h files
   7.2 Declare an image variable: IntImage<double>  im;
   7.3 Load the image: im.Load(filename) -- using OpenCV, so supporting all OpenCV supported format;
   7.4 Allocate buffer (say, 'feature') for storing the generated feature vector;
   7.5 Load the eigenvectors ( Array2d<double> lf; lf.Load(filename,nrow,ncol) ) 
   7.5 Call GenerateFeatureForSingleChannelImage(im,feature,NULL)
  
8 Generating your own eigenvectors
   8.1 Call GenerateFeatureForSingleChannelImage(im,feature,of) will write the feature vector to 'of',
       which is std::ofstream (I use std::ofstream of("pcatrain.txt") )
   8.2 Call the above function for all your training images
   8.3 Refer to loadfactor.m for a Matlab script to generate eigenvectors.
