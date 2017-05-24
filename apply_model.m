function [probabilities,segmentation]=apply_model(netDAG,imCoarse,imLocal,img1)

netDAG.mode='test'; %important put DAG into test mode

[m,n,o]=size(img1);
m1=floor(m/3)*3;
% set up coordinate grid 
strideTrain=3; zslice=2:strideTrain:o;
[x_uu,y_uu,z_uu]=meshgrid(1:1:n,1:1:m1,zslice);
[x,y,z]=meshgrid(1:n,1:m,1:o);

% get deep CNN prediction 
idxPred=getVarIndex(netDAG,'prediction');

%for k=1:31; %when using GPU in 'apply' multiple batches are advisable
netDAG.eval({'input_orig',(imCoarse(:,:,:,:)),'input_2',(imLocal(1:m1,:,:,:))});
res1=permute(gather(netDAG.vars(idxPred).value),[3,1,2,4]);
resL=softmax(res1);
% interpolate slices
denseLocal=interp3(x_uu,y_uu,z_uu,reshape(single(resL(2,:,:,:)),size(x_uu)),x,y,z,'*linear',0);

% post-process (edge-preserving smoothing)
maskTest=true(m,n,o);
probVol=[1-denseLocal(:)';denseLocal(:)'];
compiled=dir('postProcessRegularise.mex*');
if(isempty(compiled))
    mex postProcessRegularise.cpp
end
[~,~,probreg]=postProcessRegularise(probVol,single(img1),uint8(maskTest),50,10);

probabilities=reshape((probreg(2,:)./sum(probreg,1)),m,n,o);
segmentation=probabilities>0.54; %cross-validated threshold