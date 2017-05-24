function fused=overlayparula(heatmap,greyim)

greyim=normalise(single(greyim));
heatmap=normalise(single(heatmap));

map=colormap(parula(256)).*256;%choose prefered colormap

heatmapRGB=map(uint8(heatmap.*256)+1,1); %map values to RGB
heatmapRGB(:,2)=map(uint8(heatmap.*256)+1,2); 
heatmapRGB(:,3)=map(uint8(heatmap.*256)+1,3);
heatmapRGB=reshape(heatmapRGB,[size(heatmap),3])./256;

alpha=min(heatmap./0.10,1).*.5;%adjust alpha for transparent pixels

fused=repmat(greyim.*(1-alpha),[1,1,3])+heatmapRGB.*repmat(alpha,[1,1,3]);

function data=normalise(data)

data=(data-min(data(:)))./(max(data(:))-min(data(:)));