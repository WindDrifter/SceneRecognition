%for lf_UIUC
load pcatrain.txt
ii=[repmat(1,1,25) repmat(2,1,5) 3]
ii=repmat(ii,1,1500); % change 1500 to your number of images
data=pcatrain(ii==1,:);
cov=data'*data;
[v d]=eig(cov);
d=diag(d);
a=flipud(d)
b=cumsum(a)
c=b/max(b)
c(40)
c(10)
a=v(:,end-39:end)';
save lf_temp.txt a -ASCII % change lf_temp.txt to your filename
%for lf_leaf & lf_leafContour
load pcatrain.txt
ii=[repmat(1,1,25) repmat(2,1,5) 3]
ii=repmat(ii,1,375);
data=pcatrain(ii==1,:);
cov=data'*data;
[v d]=eig(cov);
d=diag(d);
a=flipud(d)
b=cumsum(a)
c=b/max(b)
c(40)
c(10)
a=v(:,end-39:end)';
save lf_temp.txt a -ASCII
