function [imCoarse,imLocal,img1]=prepare_data_individual_scan(filename)

hs=fspecial('gaussian',[9,1],1.50);
% original image is used for final edge-preserving smoothing
img1=load_untouch_nii(filename); img1=img1.img;

imgFilt=single(imfilter(imfilter(imfilter(double(img1),hs),hs'),reshape(hs,1,1,[])));

% extract strided sampling coordinates for contextual path
[m,n,o]=size(imgFilt);

strideTrain=3;
[x_train,y_train,z_train]=meshgrid(2:strideTrain:n,2:strideTrain:m,2:strideTrain:o);
ind_train=sub2ind([m,n,o],y_train,x_train,z_train);

zslice=2:strideTrain:o;
% imCoarse has stride of 3
imCoarse=zeros(41+16*2,28+16*2,33,length(zslice),'single');

%pad filtered image with zeros and divide by 100 (see paper for details)
volPad=single(padarray(imgFilt(ind_train)+1000,[16,16,16],0))/100;

% multiple 3D stacks (with neighbouring slices as channels)
for i=1:length(zslice)
    imCoarse(:,:,:,i)=volPad(:,:,i:1:33+i-1);
end

% local CNN: prepare image data (has 9 slices)
imLocal=zeros(m,n,9,length(zslice),'single');

scanPad=padarray(img1+1000,[0,0,4]);

% multiple 3D stacks (with neighbouring slices as channels)
for i=1:length(zslice);
    imLocal(:,:,:,i)=scanPad(:,:,zslice(i)+1:zslice(i)+9);
end

% scale intensities for pancreas in CT
imLocal=min(max(imLocal+160-1000,0),400)/200-1;
