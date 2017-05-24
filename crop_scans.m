function crop_scans(bbox,in_folder,out_folder)
% load ground truth data for thirty scans to crop and resample
% abdominal segmentation challenge at MICCAI 2015
% the data can be downloaded at https://www.synapse.org/#!Synapse:syn3193805/wiki/89480
% see "Evaluation of Six Registration Methods for the Human Abdomen on
% Clinically Acquired CT" by Z. Xu et al. TBME 2016 for more information

nuTrain=[1:10,21:40];

tic;
for i=1:30;
    %load and resample scans and segmentations
    scanTrain=load_untouch_nii([in_folder,'/img/img',num2str(nuTrain(i),'%04d'),'.nii']);
    voxTrain=scanTrain.hdr.dime.pixdim(2:4); scanTrain=single(scanTrain.img);
        
    segmentTrain=load_untouch_nii([in_folder,'/label/label',num2str(nuTrain(i),'%04d'),'.nii']);
    segmentTrain=int16(segmentTrain.img==11); %pancreas is label 11
        
    %bounding box and padding
    bboxnew=bbox.boundingbox_all(i,:);
    scanTrain=scanTrain(bboxnew(2):bboxnew(2)+bboxnew(5),bboxnew(1):bboxnew(1)+bboxnew(4),bboxnew(3):bboxnew(3)+bboxnew(6));
    segmentTrain=segmentTrain(bboxnew(2):bboxnew(2)+bboxnew(5),bboxnew(1):bboxnew(1)+bboxnew(4),bboxnew(3):bboxnew(3)+bboxnew(6));
    
    % mask and save pancreas region
    [m,n,o]=size(scanTrain);
    %resample to fixed dimensions [124,84,94]
    [xi,yi,zi]=meshgrid(linspace(1,n,84),linspace(1,m,124),linspace(1,o,94));
    scanTrain=interp3(single(scanTrain),xi,yi,zi,'*linear');
    scanTrain(isnan(scanTrain(:)))=0;
    voxelsize_approx=[1.5,1.5,1.5];
    save_nii(make_nii(scanTrain,voxelsize_approx),[out_folder,'/img',num2str(i),'_res.nii.gz']);
    segmentTrain=interp3(segmentTrain,xi,yi,zi,'*nearest');
    save_nii(make_nii(segmentTrain,voxelsize_approx),[out_folder,'/seg',num2str(i),'_res.nii.gz']);
    
    toc;
end