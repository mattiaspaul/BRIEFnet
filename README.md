# BRIEFnet
Code for MICCAI 2017 paper
please see http://mpheinrich.de for PDF and more details

## Prerequisites to run example

### 1) Create free synapse.org account and download pancreas dataset
"Beyond the Cranial Vault" MICCAI workshop (https://www.synapse.org/#!Synapse:syn3193805/wiki/89480)

Log in, click on 'Files' and select 'Abdomen' (or go to https://www.synapse.org/#!Synapse:syn3376386)
you will only need to download RawData.zip (which has a size of 1.53 GBytes and contains 30 scans) and subsequently extract the files. Please note, that the files are not consecutively named (11-20 are missing) - which will be fixed in step 2. 

### 2) Download and install the c3d of ITK-Snap
http://www.itksnap.org/pmwiki/pmwiki.php?n=Downloads.C3D

Open a terminal window and navigate into the BRIEFnet source folder
run the following script to crop the scans (edit path if needed)

./crop_scans.sh ~/Downloads/RawData/

This will generate 30 scans and corresponding (binary) segmentations of sizes 124x84x94.

### 3) Open a MATLAB instance and install matconvnet
https://github.com/vlfeat/matconvnet

Follow the guide to compile the toolbox at http://www.vlfeat.org/matconvnet/install/.
When applying the trained models the CPU is sufficient, otherwise a GPU is recommended.
Make sure MatConvNet is working and its paths are set within MATLAB.

In case you have not used nifti files in MATLAB before install the toolbox of Jimmy Shen: http://de.mathworks.com/matlabcentral/fileexchange/8797-tools-for-nifti-and-analyze-image

### 4) Load a trained BRIEFnet model and a apply it to a scan (\#7)
We have split the cross-validation in 6 folds of 25 training images. 
Fold 1: \#6-\#30, Fold 2: \#1-\#5 and \#11-\#30 etc. For testing \#7, we are using the following commands 

load models/briefnet_fold2.mat
