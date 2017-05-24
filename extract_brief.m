function outputs = extract_brief_old(inputs, params)
n_feat=size(params{1},4);
kernel=size(params{1}(:,:,:,1));
batch=size(inputs{1},4);
pad=(kernel-1)./2;
size_m=size(inputs{1},1)-2*pad(1);
size_n=size(inputs{1},2)-2*pad(2);

[x_train1,y_train1]=meshgrid(1:size_n,1:size_m);
n_train=numel(x_train1);
%find offset locations in random BRIEF layout for first layer
indPlus=find(params{1}(:)==+1);
[yP,xP,zP,~]=ind2sub([kernel,n_feat],indPlus);
indFeat=sub2ind(size(inputs{1}(:,:,:,1)),repmat(y_train1(:),1,n_feat)+repmat(yP',n_train,1)-1,repmat(x_train1(:),1,n_feat)+repmat(xP',n_train,1)-1,1+repmat(zP',n_train,1)-1);
indMinus=find(params{1}(:)==-1);
[yP,xP,zP,~]=ind2sub([kernel,n_feat],indMinus);
indFeatM=sub2ind(size(inputs{1}(:,:,:,1)),repmat(y_train1(:),1,n_feat)+repmat(yP',n_train,1)-1,repmat(x_train1(:),1,n_feat)+repmat(xP',n_train,1)-1,1+repmat(zP',n_train,1)-1);

%for very sparse kernels this is much faster than normal convolution
outputs{1}=zeros(size_m,size_n,n_feat,batch,'single');
slice=numel(inputs{1}(:,:,:,1));
for i=1:batch
    outputs{1}(:,:,:,i)=reshape(tanh((inputs{1}(indFeat+(i-1)*slice)-inputs{1}(indFeatM+(i-1)*slice))),size_m,size_n,n_feat);
end
    
