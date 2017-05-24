function d=dice1(mask1,mask2)
d=2*sum(mask1(:)&mask2(:))/(sum(mask1(:))+sum(mask2(:)));